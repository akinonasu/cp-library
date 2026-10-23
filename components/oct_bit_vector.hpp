#ifndef OCT_BIT_VECTOR_HPP
#define OCT_BIT_VECTOR_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

struct OctBitVector {
	struct alignas(64) Block {
		std::array<std::uint32_t, 8> masks{}, prefix{};
	};
	struct Route {
		int digit, a, b, k;
	};
	int n;
	std::vector<Block> blocks;
	static std::uint32_t mask(int r) { return (std::uint32_t(1) << (r & 31)) - 1; }
	static bool has_avx2() {
#if defined(__x86_64__) || defined(__i386__)
		return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("popcnt");
#else
		return false;
#endif
	}
	static bool has_avx512() {
#if defined(__x86_64__) || defined(__i386__)
		return has_avx2() && __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vl") && __builtin_cpu_supports("avx512vpopcntdq");
#else
		return false;
#endif
	}
	static std::array<std::uint32_t, 8> cumulative(std::uint32_t low, std::uint32_t mid, std::uint32_t high, std::uint32_t valid) {
		return {~(high | mid | low) & valid, ~(high | mid) & valid, ~high & ~(mid & low) & valid,
				~high & valid, (~high | ~(mid | low)) & valid, ~(high & mid) & valid, ~(high & mid & low) & valid, valid};
	}
	explicit OctBitVector(int n = 0) : n(n), blocks((n >> 5) + 1) {}
	void build_scalar(const std::vector<std::uint32_t> &a, int shift) {
		std::array<std::uint32_t, 8> total{};
		for (int w = 0; w < static_cast<int>(blocks.size()); ++w) {
			std::uint32_t low = 0, mid = 0, high = 0;
			int start = 32 * w, length = std::min(32, n - start);
			for (int j = 0; j < length; ++j) {
				auto x = a[start + j] >> shift;
				low |= (x & 1) << j;
				mid |= ((x >> 1) & 1) << j;
				high |= ((x >> 2) & 1) << j;
			}
			auto &b = blocks[w];
			b.masks = cumulative(low, mid, high, length == 32 ? ~std::uint32_t(0) : mask(length));
			b.prefix = total;
			for (int i = 0; i < 8; ++i) total[i] += std::popcount(b.masks[i]);
		}
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"), always_inline)) static __m256i popcounts(__m256i x) {
		const auto table = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
		const auto nibble = _mm256_set1_epi8(15);
		auto low = _mm256_shuffle_epi8(table, _mm256_and_si256(x, nibble));
		auto high = _mm256_shuffle_epi8(table, _mm256_and_si256(_mm256_srli_epi16(x, 4), nibble));
		auto pairs = _mm256_maddubs_epi16(_mm256_add_epi8(low, high), _mm256_set1_epi8(1));
		return _mm256_madd_epi16(pairs, _mm256_set1_epi16(1));
	}
	__attribute__((target("avx2,popcnt"), always_inline)) static __m256i prefix_counts(const Block &b, int r) {
		auto masked = _mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i *>(b.masks.data())), _mm256_set1_epi32(mask(r)));
		return _mm256_add_epi32(popcounts(masked), _mm256_load_si256(reinterpret_cast<const __m256i *>(b.prefix.data())));
	}
	__attribute__((target("avx2,popcnt"))) void build_avx2(const std::vector<std::uint32_t> &a, int shift) {
		std::array<std::uint32_t, 8> total{};
		auto low_shift = _mm_cvtsi32_si128(31 - shift), mid_shift = _mm_cvtsi32_si128(30 - shift), high_shift = _mm_cvtsi32_si128(29 - shift);
		for (int w = 0; w < static_cast<int>(blocks.size()); ++w) {
			std::uint32_t low = 0, mid = 0, high = 0;
			int start = 32 * w, length = std::min(32, n - start), j = 0;
			for (; j + 8 <= length; j += 8) {
				auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a.data() + start + j));
				low |= std::uint32_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, low_shift)))) << j;
				mid |= std::uint32_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, mid_shift)))) << j;
				high |= std::uint32_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, high_shift)))) << j;
			}
			for (; j < length; ++j) {
				auto x = a[start + j] >> shift;
				low |= (x & 1) << j;
				mid |= ((x >> 1) & 1) << j;
				high |= ((x >> 2) & 1) << j;
			}
			auto &b = blocks[w];
			b.masks = cumulative(low, mid, high, length == 32 ? ~std::uint32_t(0) : mask(length));
			b.prefix = total;
			auto counts = popcounts(_mm256_load_si256(reinterpret_cast<const __m256i *>(b.masks.data())));
			auto updated = _mm256_add_epi32(counts, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(total.data())));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(total.data()), updated);
		}
	}
	__attribute__((target("avx2,popcnt"), always_inline)) Route route_avx2(int l, int r, int k) const {
		auto left = prefix_counts(blocks[l >> 5], l), right = prefix_counts(blocks[r >> 5], r);
		auto passed = _mm256_cmpgt_epi32(_mm256_set1_epi32(k + 1), _mm256_sub_epi32(right, left));
		int digit = std::popcount(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(passed))));
		std::array<int, 9> a{}, b{};
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a.data() + 1), left);
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(b.data() + 1), right);
		return {digit, a[digit + 1] - a[digit], b[digit + 1] - b[digit], k - b[digit] + a[digit]};
	}
	__attribute__((target("avx2,popcnt"), always_inline)) int select_avx2(int l, int r, int k) const {
		__m256i counts;
		if ((l >> 5) == (r >> 5)) {
			auto m = _mm256_set1_epi32(mask(l) ^ mask(r));
			counts = popcounts(_mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i *>(blocks[l >> 5].masks.data())), m));
		} else counts = _mm256_sub_epi32(prefix_counts(blocks[r >> 5], r), prefix_counts(blocks[l >> 5], l));
		auto passed = _mm256_cmpgt_epi32(_mm256_set1_epi32(k + 1), counts);
		return std::popcount(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(passed))));
	}
	__attribute__((target("avx2,popcnt,avx512f,avx512vl,avx512vpopcntdq"))) void build_avx512(const std::vector<std::uint32_t> &a, int shift) {
		auto total = _mm256_setzero_si256();
		std::uint32_t bit = std::uint32_t(1) << shift;
		auto low_bit = _mm512_set1_epi32(bit), mid_bit = _mm512_set1_epi32(bit << 1), high_bit = _mm512_set1_epi32(bit << 2);
		for (int w = 0; w < static_cast<int>(blocks.size()); ++w) {
			std::uint32_t low = 0, mid = 0, high = 0;
			int start = 32 * w, length = std::min(32, n - start), j = 0;
			for (; j + 16 <= length; j += 16) {
				auto x = _mm512_loadu_si512(a.data() + start + j);
				low |= std::uint32_t(_mm512_test_epi32_mask(x, low_bit)) << j;
				mid |= std::uint32_t(_mm512_test_epi32_mask(x, mid_bit)) << j;
				high |= std::uint32_t(_mm512_test_epi32_mask(x, high_bit)) << j;
			}
			for (; j < length; ++j) {
				auto x = a[start + j] >> shift;
				low |= (x & 1) << j;
				mid |= ((x >> 1) & 1) << j;
				high |= ((x >> 2) & 1) << j;
			}
			auto &b = blocks[w];
			b.masks = cumulative(low, mid, high, length == 32 ? ~std::uint32_t(0) : mask(length));
			_mm256_store_si256(reinterpret_cast<__m256i *>(b.prefix.data()), total);
			total = _mm256_add_epi32(total, _mm256_popcnt_epi32(_mm256_load_si256(reinterpret_cast<const __m256i *>(b.masks.data()))));
		}
	}
	__attribute__((target("avx2,popcnt,avx512f,avx512vl,avx512vpopcntdq"), always_inline)) static __m256i prefix_counts_avx512(const Block &b, int r) {
		auto masked = _mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i *>(b.masks.data())), _mm256_set1_epi32(mask(r)));
		return _mm256_add_epi32(_mm256_popcnt_epi32(masked), _mm256_load_si256(reinterpret_cast<const __m256i *>(b.prefix.data())));
	}
	__attribute__((target("avx2,popcnt,avx512f,avx512vl,avx512vpopcntdq"), always_inline)) Route route_avx512(int l, int r, int k) const {
		auto left = prefix_counts_avx512(blocks[l >> 5], l), right = prefix_counts_avx512(blocks[r >> 5], r);
		auto passed = _mm256_cmpgt_epi32(_mm256_set1_epi32(k + 1), _mm256_sub_epi32(right, left));
		int digit = std::popcount(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(passed))));
		auto index = _mm256_setr_epi32(digit, digit - 1, digit, digit - 1, digit, digit - 1, digit, digit - 1);
		auto a = _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(left, index));
		auto b = _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(right, index));
		int before_l = _mm_extract_epi32(a, 1) & -int(digit != 0), before_r = _mm_extract_epi32(b, 1) & -int(digit != 0);
		return {digit, _mm_cvtsi128_si32(a) - before_l, _mm_cvtsi128_si32(b) - before_r, k - before_r + before_l};
	}
	__attribute__((target("avx2,popcnt,avx512f,avx512vl,avx512vpopcntdq"), always_inline)) int select_avx512(int l, int r, int k) const {
		__m256i counts;
		if ((l >> 5) == (r >> 5)) {
			auto m = _mm256_set1_epi32(mask(l) ^ mask(r));
			counts = _mm256_popcnt_epi32(_mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i *>(blocks[l >> 5].masks.data())), m));
		} else counts = _mm256_sub_epi32(prefix_counts_avx512(blocks[r >> 5], r), prefix_counts_avx512(blocks[l >> 5], l));
		auto passed = _mm256_cmpgt_epi32(_mm256_set1_epi32(k + 1), counts);
		return std::popcount(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(passed))));
	}

#endif
	void build(const std::vector<std::uint32_t> &a, int shift) {
		assert(a.size() == std::size_t(n) && 0 <= shift && shift <= 30);
#if defined(__x86_64__) || defined(__i386__)
		if (has_avx512()) { build_avx512(a, shift); return; }
		if (has_avx2()) { build_avx2(a, shift); return; }
#endif
		build_scalar(a, shift);
	}
	int below(int r, int digit) const {
		if (!digit) return 0;
		const auto &b = blocks[r >> 5];
		return b.prefix[digit - 1] + std::popcount(b.masks[digit - 1] & mask(r));
	}
	int rank(int r, int digit) const {
		const auto &b = blocks[r >> 5];
		return b.prefix[digit] - (digit ? b.prefix[digit - 1] : 0) + std::popcount((b.masks[digit] ^ (digit ? b.masks[digit - 1] : 0)) & mask(r));
	}
	std::array<int, 8> ranks(int r) const {
		std::array<int, 8> counts;
		int previous = 0;
		for (int i = 0; i < 8; ++i) {
			int current = below(r, i + 1);
			counts[i] = current - previous;
			previous = current;
		}
		return counts;
	}
	Route route_scalar(int l, int r, int k) const {
		int low = 0, high = 8, before_l = 0, before_r = 0, after_l = l, after_r = r;
		while (high - low > 1) {
			int mid = (low + high) / 2, a = below(l, mid), b = below(r, mid);
			if (b - a <= k) low = mid, before_l = a, before_r = b;
			else high = mid, after_l = a, after_r = b;
		}
		return {low, after_l - before_l, after_r - before_r, k - before_r + before_l};
	}
};

#endif
