#line 1 "test/static_range_count_distinct.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/static_range_count_distinct"
#line 1 "data_structure/wavelet_matrix/wavelet_matrix.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#line 1 "components/oct_bit_vector.hpp"



#line 11 "components/oct_bit_vector.hpp"
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


#line 1 "components/quad_bit_vector.hpp"



#line 11 "components/quad_bit_vector.hpp"
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

struct QuadBitVector {
	struct Block {
		std::uint64_t low = 0, high = 0;
		std::array<int, 4> prefix{};
	};
	int n;
	std::vector<Block> blocks;

#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void pack_avx2(const std::uint32_t *a, int n, int shift, Block *blocks) {
		auto low_shift = _mm_cvtsi32_si128(31 - shift), high_shift = _mm_cvtsi32_si128(30 - shift);
		for (int w = 0; w < (n >> 6) + bool(n & 63); ++w) {
			int start = w * 64;
			std::uint64_t low = 0, high = 0;
			int length = std::min(64, n - start), j = 0;
			for (; j + 8 <= length; j += 8) {
				auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
				low |= std::uint64_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, low_shift)))) << j;
				high |= std::uint64_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, high_shift)))) << j;
			}
			for (; j < length; ++j) {
				low |= std::uint64_t((a[start + j] >> shift) & 1) << j;
				high |= std::uint64_t((a[start + j] >> (shift + 1)) & 1) << j;
			}
			blocks[w].low = low;
			blocks[w].high = high;
		}
		blocks[n >> 6].low = n & 63 ? blocks[n >> 6].low : 0;
		blocks[n >> 6].high = n & 63 ? blocks[n >> 6].high : 0;
	}
#endif
	explicit QuadBitVector(int n = 0) : n(n), blocks((n >> 6) + 1) {}
	void build(const std::vector<std::uint32_t> &a, int shift) {
		assert(a.size() == std::size_t(n) && 0 <= shift && shift <= 30);
		[[maybe_unused]] bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
		vectorized = n >= 64 && __builtin_cpu_supports("avx2");
		if (vectorized) pack_avx2(a.data(), n, shift, blocks.data());
#endif
		std::array<int, 4> counts{};
		for (int w = 0; w < static_cast<int>(blocks.size()); ++w) {
			std::uint64_t low = 0, high = 0;
			int start = w * 64, length = std::min(64, n - start);
			if (vectorized) low = blocks[w].low, high = blocks[w].high;
			else
				for (int j = 0; j < length; ++j) {
					low |= std::uint64_t((a[start + j] >> shift) & 1) << j;
					high |= std::uint64_t((a[start + j] >> (shift + 1)) & 1) << j;
				}
			blocks[w] = {low, high, counts};
			int c1 = std::popcount(low & ~high), c2 = std::popcount(~low & high), c3 = std::popcount(low & high);
			counts[0] += length - c1 - c2 - c3;
			counts[1] += c1;
			counts[2] += c2;
			counts[3] += c3;
		}
	}
	std::uint64_t mask(const Block &b, int digit) const {
		return ((digit & 1) ? b.low : ~b.low) & ((digit & 2) ? b.high : ~b.high);
	}

	int rank(int r, int digit) const {
		const auto &b = blocks[r >> 6];
		return b.prefix[digit] + std::popcount(mask(b, digit) & ((std::uint64_t(1) << (r & 63)) - 1));
	}
	int below(int r, int digit) const {
		if (!digit) return 0;
		if (digit == 1) return rank(r, 0);
		if (digit == 3) return r - rank(r, 3);
		const auto &b = blocks[r >> 6];
		return b.prefix[0] + b.prefix[1] + std::popcount(~b.high & ((std::uint64_t(1) << (r & 63)) - 1));
	}
	std::array<int, 4> ranks(int r) const {
		const auto &b = blocks[r >> 6];
		std::uint64_t m = (std::uint64_t(1) << (r & 63)) - 1;
		std::array<int, 4> a;
		a[1] = b.prefix[1] + std::popcount(b.low & ~b.high & m);
		a[2] = b.prefix[2] + std::popcount(~b.low & b.high & m);
		a[3] = b.prefix[3] + std::popcount(b.low & b.high & m);
		a[0] = r - a[1] - a[2] - a[3];
		return a;
	}
};


#line 1 "components/splitmix64.hpp"



#line 5 "components/splitmix64.hpp"

inline std::uint64_t splitmix64(std::uint64_t x) {
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	return x ^ (x >> 31);
}
class SplitMix64 {
	std::uint64_t state;

  public:
	explicit SplitMix64(std::uint64_t seed = 0) : state(seed) {}
	std::uint64_t operator()() { return splitmix64(state += 0x9e3779b97f4a7c15ULL); }
};


#line 1 "components/wavelet_compression.hpp"



#line 12 "components/wavelet_compression.hpp"

namespace wavelet_detail {
template<class T>
std::uint32_t lower_code(const std::vector<T> &values, bool dense, const T &x) {
	if (values.empty()) return 0;
	if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
		if (dense) {
			if (x <= values.front()) return 0;
			if (values.back() < x) return static_cast<int>(values.size());
			return std::uint64_t(x) - std::uint64_t(values.front());
		}
	int p = 0, length = static_cast<int>(values.size());
	while (length > 1) {
		int half = length / 2;
		p += (values[p + half] < x) * half;
		length -= half;
	}
	return p + (values[p] < x);
}

template<class T, bool WithPositions = true>
std::vector<std::uint32_t> compress(const std::vector<T> &a, std::vector<T> &values,
	int &ordered, std::vector<int> &positions, std::vector<int> &offsets) {
	assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(a.size());
	ordered = 0;
	if constexpr (WithPositions) offsets.clear();
	std::vector<std::uint32_t> current(n);
	bool ascending = std::is_sorted(a.begin(), a.end());
	if (ascending || std::is_sorted(a.rbegin(), a.rend())) {
		ordered = ascending ? 1 : -1;
		values.clear();
		values.reserve(n);
		for (int i = 0; i < n; ++i) {
			int j = ascending ? i : n - 1 - i;
			if (values.empty() || values.back() != a[j]) values.push_back(a[j]);
			current[j] = static_cast<int>(values.size()) - 1;
		}
		if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
		if constexpr (WithPositions) positions.clear();
		return current;
	}
	if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8) {
		if (n >= 1024) {
			using U = std::make_unsigned_t<T>;
			struct [[gnu::packed]] Item {
				U key;
				std::uint32_t index;
			};
			std::vector<Item> order(n);
			U lo = std::numeric_limits<U>::max(), hi = 0;
			constexpr U sign = std::is_signed_v<T> ? U(1) << (sizeof(T) * 8 - 1) : U(0);
			for (int i = 0; i < n; ++i) {
				U key = U(a[i]) ^ sign;
				order[i] = {key, std::uint32_t(i)};
				lo = std::min(lo, key);
				hi = std::max(hi, key);
			}
			if (hi - lo < 65536 && std::uint64_t(hi - lo) < unsigned(n)) {
				std::vector<int> codes(std::size_t(hi - lo) + 1, -1);
				for (const auto &v : order) codes[v.key - lo] = 0;
				values.clear();
				values.reserve(codes.size());
				for (std::size_t i = 0; i < codes.size(); ++i)
					if (codes[i] == 0) {
						codes[i] = static_cast<int>(values.size());
						values.push_back(T(U(lo + U(i)) ^ sign));
					}
				for (const auto &v : order) current[v.index] = codes[v.key - lo];
				if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
				if constexpr (WithPositions) positions.clear();
				return current;
			}
			std::vector<Item> next(n);
			int width = std::bit_width(U(lo ^ hi)), span_width = std::bit_width(U(hi - lo));
			if ((width + 10) / 11 > (span_width + 10) / 11) {
				for (auto &v : order) v.key -= lo;
				width = span_width;
			}
			for (int shift = 0; shift < width; shift += 11) {
				std::array<int, 2048> offsets{};
				for (const auto &v : order) ++offsets[(v.key >> shift) & 2047];
				int total = 0;
				for (int &c : offsets) {
					int t = c;
					c = total;
					total += t;
				}
				for (const auto &v : order) next[offsets[(v.key >> shift) & 2047]++] = v;
				order.swap(next);
			}
			values.clear();
			values.reserve(n);
			U previous = 0;
			if constexpr (WithPositions) {
				positions.resize(n);
				offsets.reserve(std::size_t(n) + 1);
			}
			for (int j = 0; j < n; ++j) {
				const auto &v = order[j];
				if (values.empty() || v.key != previous) {
					values.push_back(a[v.index]);
					if constexpr (WithPositions) offsets.push_back(j);
				}
				previous = v.key;
				current[v.index] = static_cast<int>(values.size()) - 1;
				if constexpr (WithPositions) positions[j] = v.index;
			}
			if constexpr (WithPositions) offsets.push_back(n);
			if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
			return current;
		}
	}
	values = a;
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
	for (int i = 0; i < n; ++i) current[i] = lower_code<T>(values, false, a[i]);
	if constexpr (WithPositions) positions.clear();
	return current;
}

template<class T> struct ValueIndex {
	int n = 0;
	std::vector<T> values;
	std::vector<std::uint32_t> original;
	void build(const std::vector<T> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = static_cast<int>(a.size());
		int ordered = 0;
		std::vector<int> positions, offsets;
		original = compress<T, false>(a, values, ordered, positions, offsets);
	}
	std::uint32_t code(const T &x) const { return lower_code(values, false, x); }
};
}


#line 20 "data_structure/wavelet_matrix/wavelet_matrix.hpp"

template <class T = std::int64_t, class Sum = std::conditional_t<std::is_integral_v<T>, std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>, T>>
class WaveletMatrix {
	using BitVector = OctBitVector;
	int n = 0, levels = 0, ordered = 0;
	std::vector<T> values;
	std::vector<std::uint32_t> original;
	std::vector<int> positions, offsets;
	bool dense = false;
	mutable std::vector<BitVector> bits;
	mutable std::vector<std::array<int, 8>> middle;
	bool vectorized = BitVector::has_avx2();
	bool wide_vectorized = BitVector::has_avx512();
	mutable bool quaternary = false;
	mutable int sum_levels = 0;
	mutable std::vector<QuadBitVector> sum_bits;
	mutable std::vector<std::array<int, 4>> sum_middle;
	static constexpr bool can_narrow = std::is_integral_v<T> && sizeof(T) <= 8 && std::is_same_v<Sum, __int128_t>;
	using CompactSum = std::conditional_t<can_narrow, std::int64_t, Sum>;
	mutable std::vector<std::vector<Sum>> sum_prefix;
	mutable std::vector<std::vector<CompactSum>> compact_prefix;
	mutable bool sum_ready = false, narrow_sums = false;
	mutable std::vector<std::uint32_t> count_index;
	mutable std::uint64_t count_seed = 0;
	std::uint32_t count_code(const T &x) const {
		std::size_t mask = count_index.size() - 1;
		std::size_t p = splitmix64((std::uint64_t(x) ^ count_seed) + 0x9e3779b97f4a7c15ULL) & mask;
		while (auto c = count_index[p]) {
			if (values[c - 1] == x) return c - 1;
			p = (p + 1) & mask;
		}
		return static_cast<std::uint32_t>(values.size());
	}
	static constexpr bool supports_sum = requires(const T &x, Sum s) { Sum(x); s + s; s - s; s * 1; };
	std::array<std::uint32_t, 8> small_codes(int l, int r) const {
		std::array<std::uint32_t, 8> a;
		a.fill(std::numeric_limits<std::uint32_t>::max());
		for (int i = 0; i < r - l; ++i) a[i] = original[l + i];
		auto swap = [&](int i, int j) {
			auto lo = std::min(a[i], a[j]), hi = std::max(a[i], a[j]);
			a[i] = lo;
			a[j] = hi;
		};
		swap(0, 1), swap(2, 3);
		if (r - l <= 4) {
			swap(0, 2), swap(1, 3), swap(1, 2);
		} else {
			swap(4, 5), swap(6, 7);
			swap(0, 2), swap(1, 3), swap(4, 6), swap(5, 7);
			swap(1, 2), swap(5, 6), swap(0, 4), swap(3, 7);
			swap(1, 5), swap(2, 6);
			swap(1, 4), swap(3, 6);
			swap(2, 4), swap(3, 5);
			swap(3, 4);
		}
		return a;
	}
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n); }
	std::uint32_t code(const T &x) const { return wavelet_detail::lower_code(values, dense, x); }
	std::pair<int, int> interval_scalar(int l, int r, std::uint32_t x) const {
		for (int d = 0; d < levels; ++d) {
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			l = middle[d][digit] + bits[d].rank(l, digit);
			r = middle[d][digit] + bits[d].rank(r, digit);
		}
		return {l, r};
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"))) std::pair<int, int> interval_avx2(int l, int r, std::uint32_t x) const {
		for (int d = 0; d < levels; ++d) {
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			l = middle[d][digit] + bits[d].rank(l, digit);
			r = middle[d][digit] + bits[d].rank(r, digit);
		}
		return {l, r};
	}
#endif
	std::pair<int, int> interval(int l, int r, std::uint32_t x) const {
		if (quaternary) return quad_interval(l, r, x);
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return interval_avx2(l, r, x);
#endif
		return interval_scalar(l, r, x);
	}
	int less_than_scalar(int l, int r, std::uint32_t x) const {
		if (!x || l == r) return 0;
		if (x >= values.size()) return r - l;
		if (ordered) return std::clamp(ordered > 0 ? offsets[x] - l : r - n + offsets[x], 0, r - l);
		int answer = 0;
		if (r - l <= 8) {
			for (int i = l; i < r; ++i) answer += original[i] < x;
			return answer;
		}
		for (int d = 0; d + 1 < levels; ++d) {
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (digit == 7) answer += r - l - b + a;
			else answer += bits[d].below(r, digit) - bits[d].below(l, digit);
			l = middle[d][digit] + a;
			r = middle[d][digit] + b;
		}
		int digit = x & 7;
		return answer + (bits.back().below(r, digit) - bits.back().below(l, digit));
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"))) int less_than_avx2(int l, int r, std::uint32_t x) const {
		if (!x || l == r) return 0;
		if (x >= values.size()) return r - l;
		if (ordered) return std::clamp(ordered > 0 ? offsets[x] - l : r - n + offsets[x], 0, r - l);
		int answer = 0;
		if (r - l <= 8) {
			for (int i = l; i < r; ++i) answer += original[i] < x;
			return answer;
		}
		for (int d = 0; d + 1 < levels; ++d) {
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (digit == 7) answer += r - l - b + a;
			else answer += bits[d].below(r, digit) - bits[d].below(l, digit);
			l = middle[d][digit] + a;
			r = middle[d][digit] + b;
		}
		int digit = x & 7;
		return answer + (bits.back().below(r, digit) - bits.back().below(l, digit));
	}
#endif
	int less_than(int l, int r, std::uint32_t x) const {
		if (quaternary) return quad_less_than(l, r, x);
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return less_than_avx2(l, r, x);
#endif
		return less_than_scalar(l, r, x);
	}
	[[gnu::always_inline]] int between_impl(int l, int r, std::uint32_t low, std::uint32_t high) const {
		int common = levels - 1 - (std::bit_width(low ^ high) - 1) / 3;
		int d = 0;
		for (; d < common; ++d) {
			int digit = (low >> (3 * (levels - 1 - d))) & 7;
			l = middle[d][digit] + bits[d].rank(l, digit);
			r = middle[d][digit] + bits[d].rank(r, digit);
		}
		int shift = 3 * (levels - 1 - d), a = (low >> shift) & 7, b = (high >> shift) & 7;
		int answer = bits[d].below(r, b) - bits[d].below(l, b) - bits[d].below(r, a) + bits[d].below(l, a);
		if (d + 1 == levels) return answer;
		int ll = middle[d][a] + bits[d].rank(l, a), lr = middle[d][a] + bits[d].rank(r, a);
		int hl = middle[d][b] + bits[d].rank(l, b), hr = middle[d][b] + bits[d].rank(r, b);
		for (++d; d + 1 < levels; ++d) {
			shift = 3 * (levels - 1 - d), a = (low >> shift) & 7, b = (high >> shift) & 7;
			answer += bits[d].below(hr, b) - bits[d].below(hl, b) - bits[d].below(lr, a) + bits[d].below(ll, a);
			ll = middle[d][a] + bits[d].rank(ll, a);
			lr = middle[d][a] + bits[d].rank(lr, a);
			hl = middle[d][b] + bits[d].rank(hl, b);
			hr = middle[d][b] + bits[d].rank(hr, b);
		}
		return answer + (bits.back().below(hr, high & 7) - bits.back().below(hl, high & 7))
			- (bits.back().below(lr, low & 7) - bits.back().below(ll, low & 7));
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"))) int between_avx2(int l, int r, std::uint32_t low, std::uint32_t high) const {
		return between_impl(l, r, low, high);
	}
#endif
	template <bool Previous>
	std::optional<T> neighbor_scalar(int l, int r, std::uint32_t x) const {
		if (r - l <= 8) {
			std::uint32_t best = Previous ? 0 : static_cast<std::uint32_t>(values.size());
			for (int i = l; i < r; ++i) {
				std::uint32_t c = original[i];
				if constexpr (Previous) {
					if (c <= x) best = std::max(best, c + 1);
				} else if (c >= x) best = std::min(best, c);
			}
			if constexpr (Previous) return best ? std::optional<T>(values[best - 1]) : std::nullopt;
			else return best < values.size() ? std::optional<T>(values[best]) : std::nullopt;
		}
		std::array<std::pair<int, int>, 11> path;
		int d = 0;
		for (; d < levels; ++d) {
			path[d] = {l, r};
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (a == b) break;
			l = middle[d][digit] + a;
			r = middle[d][digit] + b;
		}
		if (d == levels) return values[x];
		for (; d >= 0; --d) {
			l = path[d].first;
			r = path[d].second;
			int shift = 3 * (levels - 1 - d), digit = (x >> shift) & 7;
			for (digit += Previous ? -1 : 1; 0 <= digit && digit < 8; digit += Previous ? -1 : 1) {
				int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
				if (a == b) continue;
				x = ((x >> shift) & ~std::uint32_t(7)) | digit;
				l = middle[d][digit] + a;
				r = middle[d][digit] + b;
				for (++d; d < levels; ++d) {
					auto step = route_scalar(l, r, Previous ? r - l - 1 : 0, d);
					x = (x << 3) | step.digit;
					l = middle[d][step.digit] + step.a;
					r = middle[d][step.digit] + step.b;
				}
				return values[x];
			}
		}
		return std::nullopt;
	}
#if defined(__x86_64__) || defined(__i386__)
	template <bool Previous>
	__attribute__((target("avx2,popcnt"))) std::optional<T> neighbor_avx2(int l, int r, std::uint32_t x) const {
		if (r - l <= 8) {
			std::uint32_t best = Previous ? 0 : static_cast<std::uint32_t>(values.size());
			for (int i = l; i < r; ++i) {
				std::uint32_t c = original[i];
				if constexpr (Previous) {
					if (c <= x) best = std::max(best, c + 1);
				} else if (c >= x) best = std::min(best, c);
			}
			if constexpr (Previous) return best ? std::optional<T>(values[best - 1]) : std::nullopt;
			else return best < values.size() ? std::optional<T>(values[best]) : std::nullopt;
		}
		std::array<std::pair<int, int>, 11> path;
		int d = 0;
		for (; d < levels; ++d) {
			path[d] = {l, r};
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (a == b) break;
			l = middle[d][digit] + a;
			r = middle[d][digit] + b;
		}
		if (d == levels) return values[x];
		for (; d >= 0; --d) {
			l = path[d].first;
			r = path[d].second;
			int shift = 3 * (levels - 1 - d), digit = (x >> shift) & 7;
			for (digit += Previous ? -1 : 1; 0 <= digit && digit < 8; digit += Previous ? -1 : 1) {
				int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
				if (a == b) continue;
				x = ((x >> shift) & ~std::uint32_t(7)) | digit;
				l = middle[d][digit] + a;
				r = middle[d][digit] + b;
				for (++d; d < levels; ++d) {
					auto step = route_avx2(l, r, Previous ? r - l - 1 : 0, d);
					x = (x << 3) | step.digit;
					l = middle[d][step.digit] + step.a;
					r = middle[d][step.digit] + step.b;
				}
				return values[x];
			}
		}
		return std::nullopt;
	}
#endif
	template <bool Previous>
	std::optional<T> neighbor(int l, int r, std::uint32_t x) const {
		if (values.size() >= 1024 && values.size() >= std::size_t(n) / 8 && r - l >= n / 8) {
			int c = static_cast<int>(x);
			for (int repeat = 0; repeat < 4 && c >= 0 && c < static_cast<int>(values.size()); ++repeat, c += Previous ? -1 : 1) {
				int start = offsets[c], end = offsets[c + 1];
				if (end - start > 8) break;
				for (int i = start; i < end; ++i)
					if (unsigned(positions[i] - l) < unsigned(r - l)) return values[c];
			}
		}
		if (quaternary) return quad_neighbor<Previous>(l, r, x);
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return neighbor_avx2<Previous>(l, r, x);
#endif
		return neighbor_scalar<Previous>(l, r, x);
	}

	void partition(std::vector<std::uint32_t> &current, std::vector<std::uint32_t> &next, int d) const {
		auto offset = middle[d];
		int shift = 3 * (levels - 1 - d);
		for (int i = 0; i < n; ++i) next[offset[(current[i] >> shift) & 7]++] = current[i];
		current.swap(next);
	}
	template <class S>
	void prefix_sum(std::vector<S> &prefix, const std::vector<std::uint32_t> &current, int l, int r) const
		requires supports_sum
	{
		prefix.resize(std::size_t(r - l) + 1);
		for (int i = 0; i < r - l; ++i) prefix[i + 1] = prefix[i] + S(values[current[l + i]]);
	}
	void partition_for_sum(std::vector<std::uint32_t> &current, std::vector<std::uint32_t> &next, int d) const {
		auto offset = sum_middle[d];
		int shift = 2 * (sum_levels - 1 - d);
		for (int i = 0; i < n; ++i) next[offset[(current[i] >> shift) & 3]++] = current[i];
		current.swap(next);
	}
	template <class S>
	void partition_sum(std::vector<std::vector<S>> &prefix, std::vector<std::uint32_t> &current, std::vector<std::uint32_t> &next, int d) const
		requires supports_sum
	{
		int base = 3 * d + 1, shift = 2 * (sum_levels - 1 - d);
		if constexpr (std::is_integral_v<S> && sizeof(S) <= 8) {
			auto &row = prefix[base];
			row.resize(std::size_t(sum_middle[d][2]) + 1);
			auto offset = sum_middle[d];
			int zero = 0;
			S total = 0;
			for (std::uint32_t x : current) {
				int digit = (x >> shift) & 3;
				next[offset[digit]++] = x;
				total += S(values[x]) * (digit < 2);
				zero += digit < 2;
				row[zero] = total;
			}
			current.swap(next);
		} else {
			int zero = 0, one = sum_middle[d][2];
			for (std::uint32_t x : current) {
				bool high = (x >> (shift + 1)) & 1;
				int p = high ? one : zero;
				one += high;
				zero += !high;
				next[p] = x;
			}
			prefix_sum(prefix[base], next, 0, sum_middle[d][2]);
			current.swap(next);
			partition_for_sum(current, next, d);
		}
		prefix_sum(prefix[base + 1], current, 0, sum_middle[d][1]);
		prefix_sum(prefix[base + 2], current, sum_middle[d][2], sum_middle[d][3]);
	}
	struct SumRoute {
		int digit, a, b, k, high_l, high_r, zero_l, zero_r;
	};
	[[gnu::always_inline]] SumRoute sum_route(int l, int r, int k, int d) const {
		const auto &left = sum_bits[d].blocks[l >> 6], &right = sum_bits[d].blocks[r >> 6];
		std::uint64_t lm = (std::uint64_t(1) << (l & 63)) - 1, rm = (std::uint64_t(1) << (r & 63)) - 1;
		std::uint64_t lh = left.high & lm, rh = right.high & rm;
		int ha = left.prefix[2] + left.prefix[3] + std::popcount(lh);
		int hb = right.prefix[2] + right.prefix[3] + std::popcount(rh);
		int high_l = l - ha, high_r = r - hb;
		bool high = k >= high_r - high_l;
		k -= high ? high_r - high_l : 0;
		int a = left.prefix[2 * high] + std::popcount(~left.low & (high ? lh : lm ^ lh));
		int b = right.prefix[2 * high] + std::popcount(~right.low & (high ? rh : rm ^ rh));
		bool low = k >= b - a;
		k -= low ? b - a : 0;
		return {2 * high + low, low ? (high ? ha : high_l) - a : a,
				low ? (high ? hb : high_r) - b : b, k, high_l, high_r, a, b};
	}
	[[gnu::noinline]] std::pair<int, int> quad_interval(int l, int r, std::uint32_t x) const {
		for (int d = 0; d < sum_levels; ++d) {
			int digit = (x >> (2 * (sum_levels - 1 - d))) & 3;
			l = sum_middle[d][digit] + sum_bits[d].rank(l, digit);
			r = sum_middle[d][digit] + sum_bits[d].rank(r, digit);
		}
		return {l, r};
	}
	[[gnu::noinline]] int quad_less_than(int l, int r, std::uint32_t x) const {
		if (!x || l == r) return 0;
		if (x >= values.size()) return r - l;
		if (ordered) return std::clamp(ordered > 0 ? offsets[x] - l : r - n + offsets[x], 0, r - l);
		int answer = 0;
		if (r - l <= 8) {
			for (int i = l; i < r; ++i) answer += original[i] < x;
			return answer;
		}
		for (int d = 0; d + 1 < sum_levels; ++d) {
			int digit = (x >> (2 * (sum_levels - 1 - d))) & 3;
			int a = sum_bits[d].rank(l, digit), b = sum_bits[d].rank(r, digit);
			if (digit == 3) answer += r - l - b + a;
			else answer += sum_bits[d].below(r, digit) - sum_bits[d].below(l, digit);
			l = sum_middle[d][digit] + a;
			r = sum_middle[d][digit] + b;
		}
		int digit = x & 3;
		return answer + (sum_bits.back().below(r, digit) - sum_bits.back().below(l, digit));
	}
	template <bool Previous>
	[[gnu::noinline]] std::optional<T> quad_neighbor(int l, int r, std::uint32_t x) const {
		if (r - l <= 8) {
			std::uint32_t best = Previous ? 0 : static_cast<std::uint32_t>(values.size());
			for (int i = l; i < r; ++i) {
				std::uint32_t c = original[i];
				if constexpr (Previous) {
					if (c <= x) best = std::max(best, c + 1);
				} else if (c >= x) best = std::min(best, c);
			}
			if constexpr (Previous) return best ? std::optional<T>(values[best - 1]) : std::nullopt;
			else return best < values.size() ? std::optional<T>(values[best]) : std::nullopt;
		}
		std::array<std::pair<int, int>, 16> path;
		int d = 0;
		for (; d < sum_levels; ++d) {
			path[d] = {l, r};
			int digit = (x >> (2 * (sum_levels - 1 - d))) & 3;
			int a = sum_bits[d].rank(l, digit), b = sum_bits[d].rank(r, digit);
			if (a == b) break;
			l = sum_middle[d][digit] + a;
			r = sum_middle[d][digit] + b;
		}
		if (d == sum_levels) return values[x];
		for (; d >= 0; --d) {
			l = path[d].first;
			r = path[d].second;
			int shift = 2 * (sum_levels - 1 - d), digit = (x >> shift) & 3;
			for (digit += Previous ? -1 : 1; 0 <= digit && digit < 4; digit += Previous ? -1 : 1) {
				int a = sum_bits[d].rank(l, digit), b = sum_bits[d].rank(r, digit);
				if (a == b) continue;
				x = ((x >> shift) & ~std::uint32_t(3)) | digit;
				l = sum_middle[d][digit] + a;
				r = sum_middle[d][digit] + b;
				for (++d; d < sum_levels; ++d) {
					auto step = quad_route(l, r, Previous ? r - l - 1 : 0, d);
					x = (x << 2) | step.digit;
					l = sum_middle[d][step.digit] + step.a;
					r = sum_middle[d][step.digit] + step.b;
				}
				return values[x];
			}
		}
		return std::nullopt;
	}
	[[gnu::always_inline]] SumRoute quad_route(int l, int r, int k, int d) const {
		const auto &left = sum_bits[d].blocks[l >> 6], &right = sum_bits[d].blocks[r >> 6];
		std::uint64_t lm = (std::uint64_t(1) << (l & 63)) - 1, rm = (std::uint64_t(1) << (r & 63)) - 1;
		int la = left.prefix[0] + std::popcount(~(left.low | left.high) & lm);
		int ra = right.prefix[0] + std::popcount(~(right.low | right.high) & rm);
		int lb = left.prefix[0] + left.prefix[1] + std::popcount(~left.high & lm);
		int rb = right.prefix[0] + right.prefix[1] + std::popcount(~right.high & rm);
		int lc = l - left.prefix[3] - std::popcount(left.low & left.high & lm);
		int rc = r - right.prefix[3] - std::popcount(right.low & right.high & rm);
		int digit = (k >= ra - la) + (k >= rb - lb) + (k >= rc - lc);
		std::array<int, 5> a{0, la, lb, lc, l}, b{0, ra, rb, rc, r};
		int base = digit & 2;
		return {digit, a[digit + 1] - a[digit], b[digit + 1] - b[digit], k - b[digit] + a[digit], lb, rb, a[base + 1] - a[base], b[base + 1] - b[base]};
	}
	[[gnu::noinline]] T quad_kth(int l, int r, int k) const {
		std::uint32_t x = 0;
		for (int d = 0; d + 1 < sum_levels; ++d) {
			auto step = quad_route(l, r, k, d);
			x = (x << 2) | step.digit;
			k = step.k;
			l = sum_middle[d][step.digit] + step.a;
			r = sum_middle[d][step.digit] + step.b;
		}
		return values[(x << 2) | quad_route(l, r, k, sum_levels - 1).digit];
	}
	using Route = typename BitVector::Route;
	[[gnu::always_inline]] Route route_scalar(int l, int r, int k, int d) const {
		return bits[d].route_scalar(l, r, k);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"), always_inline)) Route route_avx2(int l, int r, int k, int d) const {
		return bits[d].route_avx2(l, r, k);
	}
#endif
	void init_sum() const
		requires supports_sum
	{
		sum_levels = (std::bit_width(unsigned(static_cast<int>(values.size()) - 1)) + 1) / 2;
		if constexpr (can_narrow) {
			Sum positive = 0, negative = 0;
			for (int i = 0; i < static_cast<int>(values.size()); ++i) {
				Sum v = Sum(values[i]) * (offsets[i + 1] - offsets[i]);
				if (v > 0) positive += v;
				else negative += v;
			}
			narrow_sums = positive <= std::numeric_limits<std::int64_t>::max() && negative >= std::numeric_limits<std::int64_t>::min();
		}
		if (narrow_sums) {
			compact_prefix.resize(ordered ? 1 : 3 * sum_levels + 1);
			prefix_sum(compact_prefix[0], original, 0, n);
		} else {
			sum_prefix.resize(ordered ? 1 : 3 * sum_levels + 1);
			prefix_sum(sum_prefix[0], original, 0, n);
		}
	}
	void construct(const std::vector<T> &a, bool with_sum) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		count_index.clear();
		count_seed = 0;
		sum_bits.clear();
		sum_middle.clear();
		sum_levels = 0;
		quaternary = false;
		sum_prefix.clear();
		compact_prefix.clear();
		sum_ready = narrow_sums = false;
		n = static_cast<int>(a.size());
		dense = false;
		ordered = 0;
		if (!n || std::all_of(a.begin() + 1, a.end(), [&](const T &x) { return x == a.front(); })) {
			values.clear();
			if (n) values.push_back(a.front());
			if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
			original.clear();
			positions.clear();
			offsets = {0, n};
			levels = 0;
			bits.clear();
			middle.clear();
			sum_ready = with_sum;
			return;
		}
		original = wavelet_detail::compress(a, values, ordered, positions, offsets);
		bool indexed = !offsets.empty();
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
			dense = std::uint64_t(values.back()) - std::uint64_t(values.front()) == values.size() - 1;
		if (!indexed) offsets.assign(values.size() + 1, 0);
		levels = (std::bit_width(unsigned(static_cast<int>(values.size()) - 1)) + 2) / 3;
		if (!indexed) {
			for (std::uint32_t x : original) ++offsets[x + 1];
			for (int i = 0; i < static_cast<int>(values.size()); ++i) offsets[i + 1] += offsets[i];
		}
		if (ordered) {
			bits.clear();
			middle.clear();
			if constexpr (supports_sum)
				if (with_sum) init_sum();
			sum_ready = with_sum;
			return;
		}
		if (!indexed) {
			positions.resize(n);
			auto cursor = offsets;
			for (int i = 0; i < n; ++i) positions[cursor[original[i]]++] = i;
		}
		bits.clear();
		middle.clear();
		if (with_sum) {
			quaternary = true;
			sum_levels = (std::bit_width(unsigned(static_cast<int>(values.size()) - 1)) + 1) / 2;
			levels = sum_levels;
			sum_bits.reserve(sum_levels);
			sum_middle.resize(sum_levels);
			if constexpr (supports_sum)
				if (with_sum) init_sum();
			std::vector<std::uint32_t> current = original, next(n);
			for (int d = 0; d < sum_levels; ++d) {
				sum_bits.emplace_back(n);
				sum_bits.back().build(current, 2 * (sum_levels - 1 - d));
				auto totals = sum_bits.back().ranks(n);
				sum_middle[d] = {0, totals[0], totals[0] + totals[1], totals[0] + totals[1] + totals[2]};
				if constexpr (supports_sum) {
					if (with_sum) {
						if (narrow_sums) partition_sum(compact_prefix, current, next, d);
						else partition_sum(sum_prefix, current, next, d);
					} else if (d + 1 < sum_levels) partition_for_sum(current, next, d);
				} else if (d + 1 < sum_levels) partition_for_sum(current, next, d);
			}
			sum_ready = with_sum;
			return;
		}
		auto current = original;
		bits.clear();
		bits.reserve(levels);
		middle.resize(levels);
		std::vector<std::uint32_t> next(n);
		for (int d = 0; d < levels; ++d) {
			bits.emplace_back(n);
			bits.back().build(current, 3 * (levels - 1 - d));
			auto totals = bits.back().ranks(n);
			middle[d][0] = 0;
			for (int i = 1; i < 8; ++i) middle[d][i] = middle[d][i - 1] + totals[i - 1];
			if (d + 1 < levels) partition(current, next, d);
		}
		if constexpr (supports_sum)
			if (with_sum) prepare_sum();
	}
	template <class S>
	S sum_smallest_impl_scalar(const std::vector<std::vector<S>> &prefix, int l, int r, int k) const
		requires supports_sum
	{
		S answer = 0;
		std::uint32_t x = 0;
		for (int d = 0; d < sum_levels; ++d) {
			auto step = sum_route(l, r, k, d);
			int base = 3 * d + 1;
			if (step.digit & 2) answer += prefix[base][step.high_r] - prefix[base][step.high_l];
			if (step.digit & 1) {
				const auto &row = prefix[base + 1 + (step.digit >> 1)];
				answer += row[step.zero_r] - row[step.zero_l];
			}
			k = step.k;
			if (!k) return answer;
			x = (x << 2) | step.digit;
			l = sum_middle[d][step.digit] + step.a;
			r = sum_middle[d][step.digit] + step.b;
		}
		return answer + S(values[x]) * k;
	}
#if defined(__x86_64__) || defined(__i386__)
	template <class S>
	__attribute__((target("avx2,popcnt"))) S sum_smallest_impl_avx2(const std::vector<std::vector<S>> &prefix, int l, int r, int k) const
		requires supports_sum
	{
		S answer = 0;
		std::uint32_t x = 0;
		for (int d = 0; d < sum_levels; ++d) {
			auto step = sum_route(l, r, k, d);
			int base = 3 * d + 1;
			if (step.digit & 2) answer += prefix[base][step.high_r] - prefix[base][step.high_l];
			if (step.digit & 1) {
				const auto &row = prefix[base + 1 + (step.digit >> 1)];
				answer += row[step.zero_r] - row[step.zero_l];
			}
			k = step.k;
			if (!k) return answer;
			x = (x << 2) | step.digit;
			l = sum_middle[d][step.digit] + step.a;
			r = sum_middle[d][step.digit] + step.b;
		}
		return answer + S(values[x]) * k;
	}
#endif
	template <class S>
	S sum_smallest_impl(const std::vector<std::vector<S>> &prefix, int l, int r, int k) const
		requires supports_sum
	{
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return sum_smallest_impl_avx2(prefix, l, r, k);
#endif
		return sum_smallest_impl_scalar(prefix, l, r, k);
	}

	template <bool Largest, bool Lower, class S>
	int sum_bound(const std::vector<std::vector<S>> &prefix, int l, int r, Sum target) const {
		Sum sum = Sum(prefix[0][r]) - Sum(prefix[0][l]);
		if constexpr (Lower) {
			if (sum < target) return -1;
		} else if (sum <= target) return r - l;
		using Work = std::conditional_t<can_narrow && std::is_same_v<S, std::int64_t>, std::int64_t, Sum>;
		Work total = Work(sum), remaining = Work(target);
		int answer = 0;
		std::uint32_t x = 0;
		auto choose = [&](Work zero, int count) {
			Work preferred = Largest ? total - zero : zero;
			int length = Largest ? r - l - count : count;
			bool skip = Lower ? preferred < remaining : preferred <= remaining;
			if (skip) remaining -= preferred, answer += length;
			bool one = Largest != skip;
			total = one ? total - zero : zero;
			return one;
		};
		for (int d = 0; d < sum_levels; ++d) {
			const auto &row = sum_bits[d];
			int a = row.below(l, 2), b = row.below(r, 2), base = 3 * d + 1;
			bool high = choose(Work(prefix[base][b]) - Work(prefix[base][a]), b - a);
			int high_l = high ? l - a : a, high_r = high ? r - b : b;
			int c = row.rank(l, 2 * high), e = row.rank(r, 2 * high);
			l = high_l, r = high_r;
			bool low = choose(Work(prefix[base + 1 + high][e]) - Work(prefix[base + 1 + high][c]), e - c);
			int digit = 2 * high + low;
			x = (x << 2) | digit;
			l = sum_middle[d][digit] + (low ? high_l - c : c);
			r = sum_middle[d][digit] + (low ? high_r - e : e);
		}
		Work value = Work(values[x]);
		return answer + int(remaining / value) + (Lower && remaining % value != 0);
	}
	template <bool Largest, bool Lower>
	int sum_bound(int l, int r, Sum target) const
		requires (std::is_integral_v<T> && std::numeric_limits<Sum>::is_integer)
	{
		check(l, r);
		if constexpr (Lower) {
			if (target <= 0) return 0;
		} else if (target < 0) return -1;
		if (l == r) return Lower ? -1 : 0;
		assert(values.front() >= 0);
		if (!levels) {
			Sum value = Sum(values[0]);
			if constexpr (Lower) return value == 0 || value * (r - l) < target ? -1 : int(target / value) + (target % value != 0);
			else return value == 0 || value * (r - l) <= target ? r - l : int(target / value);
		}
		prepare_sum();
		if (ordered) {
			Sum total = range_sum(l, r);
			if constexpr (Lower) {
				if (total < target) return -1;
			} else if (total <= target) return r - l;
			int lo = 0, hi = r - l;
			while (hi - lo > 1) {
				int k = lo + (hi - lo) / 2;
				Sum s = Largest ? sum_largest(l, r, k) : sum_smallest(l, r, k);
				if (Lower ? s < target : s <= target) lo = k;
				else hi = k;
			}
			return Lower ? hi : lo;
		}
		if (narrow_sums) return sum_bound<Largest, Lower>(compact_prefix, l, r, target);
		return sum_bound<Largest, Lower>(sum_prefix, l, r, target);
	}
	T kth_scalar(int l, int r, int k) const {
		std::uint32_t x = 0;
		for (int d = 0; d < levels; ++d) {
			auto step = route_scalar(l, r, k, d);
			x = (x << 3) | step.digit;
			k = step.k;
			l = middle[d][step.digit] + step.a;
			r = middle[d][step.digit] + step.b;
		}
		return values[x];
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"))) T kth_avx2(int l, int r, int k) const {
		std::uint32_t x = 0;
		for (int d = 0; d + 1 < levels; ++d) {
			auto step = route_avx2(l, r, k, d);
			x = (x << 3) | step.digit;
			k = step.k;
			l = middle[d][step.digit] + step.a;
			r = middle[d][step.digit] + step.b;
		}
		return values[(x << 3) | bits.back().select_avx2(l, r, k)];
	}
	__attribute__((target("avx2,popcnt,avx512f,avx512vl,avx512vpopcntdq"))) T kth_avx512(int l, int r, int k) const {
		std::uint32_t x = 0;
		for (int d = 0; d + 1 < levels; ++d) {
			auto step = bits[d].route_avx512(l, r, k);
			x = (x << 3) | step.digit;
			k = step.k;
			l = middle[d][step.digit] + step.a;
			r = middle[d][step.digit] + step.b;
		}
		return values[(x << 3) | bits.back().select_avx512(l, r, k)];
	}
#endif

  public:
	using sum_type = Sum;
	WaveletMatrix() = default;
	explicit WaveletMatrix(const std::vector<T> &a) { build(a); }
	WaveletMatrix(const std::vector<T> &a, bool with_sum)
		requires supports_sum
	{ construct(a, with_sum); }
	void build(const std::vector<T> &a) { construct(a, false); }
	void prepare_count() const {
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8) {
			if (values.size() <= 1024 || dense || !count_index.empty()) return;
			count_seed = std::uint64_t(std::chrono::steady_clock::now().time_since_epoch().count());
			count_index.assign(std::bit_ceil(2 * values.size()), 0);
			std::size_t mask = count_index.size() - 1;
			for (std::size_t c = 0; c < values.size(); ++c) {
				std::size_t p = splitmix64((std::uint64_t(values[c]) ^ count_seed) + 0x9e3779b97f4a7c15ULL) & mask;
				while (count_index[p]) p = (p + 1) & mask;
				count_index[p] = static_cast<std::uint32_t>(c + 1);
			}
		}
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	T access(int i) const {
		assert(0 <= i && i < n);
		return values[levels ? original[i] : 0];
	}
	T operator[](int i) const { return access(i); }
	int count(int l, int r, const T &x) const {
		check(l, r);
		if (l == r) return 0;
		std::uint32_t c;
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
			c = (values.size() <= 1024 || dense || count_index.empty()) ? code(x) : count_code(x);
		else c = code(x);
		if (c == values.size() || values[c] != x) return 0;
		if (!levels) return r - l;
		int start = offsets[c], end = offsets[c + 1];
		if (ordered) {
			int first = ordered > 0 ? start : n - end;
			return std::max(0, std::min(r, first + (end - start)) - std::max(l, first));
		}
		if (l == 0 && r == n) return end - start;
		if (end - start == 1) return int(l <= positions[start]) & int(positions[start] < r);
		if (end - start <= 16) {
			int answer = 0;
			for (int i = start; i < end; ++i) answer += unsigned(positions[i] - l) < unsigned(r - l);
			return answer;
		}
		if (r - l <= 8) {
			int answer = 0;
			for (int i = l; i < r; ++i) answer += original[i] == c;
			return answer;
		}
		if (values.size() >= 1024 && std::size_t(end - start) <= values.size()) {
			int a = 0, b = 0, length = end - start;
			while (length > 1) {
				int half = length / 2;
				a += (positions[start + a + half] < l) * half;
				b += (positions[start + b + half] < r) * half;
				length -= half;
			}
			return b - a + (positions[start + b] < r) - (positions[start + a] < l);
		}
		auto [a, b] = interval(l, r, c);
		return b - a;
	}
	int rank(const T &x, int r) const { return count(0, r, x); }
	int select(const T &x, int k) const {
		std::uint32_t c;
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
			c = (values.size() <= 1024 || dense || count_index.empty()) ? code(x) : count_code(x);
		else c = code(x);
		if (k < 0 || c == values.size() || values[c] != x || k >= offsets[c + 1] - offsets[c]) return -1;
		if (ordered) return (ordered > 0 ? offsets[c] : n - offsets[c + 1]) + k;
		return levels ? positions[offsets[c] + k] : k;
	}
	T kth_smallest(int l, int r, int k) const {
		check(l, r);
		assert(0 <= k && k < r - l);
		if (!levels) return values[0];
		if (ordered) return values[original[ordered > 0 ? l + k : r - 1 - k]];
		if (r - l <= 8) return values[small_codes(l, r)[k]];
		if (quaternary) return quad_kth(l, r, k);
#if defined(__x86_64__) || defined(__i386__)
		if (wide_vectorized) return kth_avx512(l, r, k);
		if (vectorized) return kth_avx2(l, r, k);
#endif
		return kth_scalar(l, r, k);
	}
	T kth_largest(int l, int r, int k) const {
		check(l, r);
		assert(0 <= k && k < r - l);
		return kth_smallest(l, r, r - l - 1 - k);
	}
	int range_freq(int l, int r, const T &upper) const {
		check(l, r);
		return less_than(l, r, code(upper));
	}
	int range_freq(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		if (!(lower < upper)) return 0;
		auto lo = code(lower), hi = code(upper);
		if (lo == hi || l == r) return 0;
		if (quaternary || ordered || r - l <= 8 || !lo || hi >= values.size()) return less_than(l, r, hi) - less_than(l, r, lo);
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return between_avx2(l, r, lo, hi);
#endif
		return between_impl(l, r, lo, hi);
	}
	void prepare_sum() const
		requires supports_sum
	{
		if (sum_ready) return;
		if (!levels) {
			sum_ready = true;
			return;
		}
		init_sum();
		if (ordered) {
			sum_ready = true;
			return;
		}
		std::vector<std::uint32_t> current = original, next(n);
		bool indexed = !sum_bits.empty();
		sum_bits.reserve(sum_levels);
		sum_middle.resize(sum_levels);
		for (int d = 0; d < sum_levels; ++d) {
			if (!indexed) {
				sum_bits.emplace_back(n);
				sum_bits.back().build(current, 2 * (sum_levels - 1 - d));
				auto totals = sum_bits.back().ranks(n);
				sum_middle[d] = {0, totals[0], totals[0] + totals[1], totals[0] + totals[1] + totals[2]};
			}
			if (narrow_sums) partition_sum(compact_prefix, current, next, d);
			else partition_sum(sum_prefix, current, next, d);
		}
		bits.clear();
		middle.clear();
		quaternary = true;
		sum_ready = true;
	}
	Sum range_sum(int l, int r) const
		requires supports_sum
	{
		check(l, r);
		if (l == r) return Sum(0);
		if (!levels) return Sum(values[0]) * (r - l);
		if (!sum_ready && r - l <= 8) {
			Sum answer = 0;
			for (int i = l; i < r; ++i) answer += Sum(values[original[i]]);
			return answer;
		}
		prepare_sum();
		if (narrow_sums) return compact_prefix[0][r] - compact_prefix[0][l];
		return sum_prefix[0][r] - sum_prefix[0][l];
	}
	Sum sum_smallest(int l, int r, int k) const
		requires supports_sum
	{
		check(l, r);
		assert(0 <= k && k <= r - l);
		if (!k) return Sum(0);
		if (k == r - l) return range_sum(l, r);
		if (!levels) return Sum(values[0]) * k;
		if (ordered) return ordered > 0 ? range_sum(l, l + k) : range_sum(r - k, r);
		if (k == 1) return Sum(kth_smallest(l, r, 0));
		if (r - l <= 8) {
			auto a = small_codes(l, r);
			Sum answer = 0;
			for (int i = 0; i < k; ++i) answer += Sum(values[a[i]]);
			return answer;
		}
		prepare_sum();
		if (narrow_sums) return sum_smallest_impl(compact_prefix, l, r, k);
		return sum_smallest_impl(sum_prefix, l, r, k);
	}
	Sum sum_largest(int l, int r, int k) const
		requires supports_sum
	{
		check(l, r);
		assert(0 <= k && k <= r - l);
		if (!k) return Sum(0);
		if (!levels) return Sum(values[0]) * k;
		if (ordered) return ordered > 0 ? range_sum(r - k, r) : range_sum(l, l + k);
		if (k == 1) return Sum(kth_largest(l, r, 0));
		if (k == r - l) return range_sum(l, r);
		if (r - l <= 8) {
			auto a = small_codes(l, r);
			Sum answer = 0;
			for (int i = r - l - k; i < r - l; ++i) answer += Sum(values[a[i]]);
			return answer;
		}
		return range_sum(l, r) - sum_smallest(l, r, r - l - k);
	}
	int min_count_smallest(int l, int r, Sum target) const
		requires (std::is_integral_v<T> && std::numeric_limits<Sum>::is_integer)
	{ return sum_bound<false, true>(l, r, target); }
	int min_count_largest(int l, int r, Sum target) const
		requires (std::is_integral_v<T> && std::numeric_limits<Sum>::is_integer)
	{ return sum_bound<true, true>(l, r, target); }
	int max_count_smallest(int l, int r, Sum budget) const
		requires (std::is_integral_v<T> && std::numeric_limits<Sum>::is_integer)
	{ return sum_bound<false, false>(l, r, budget); }
	int max_count_largest(int l, int r, Sum budget) const
		requires (std::is_integral_v<T> && std::numeric_limits<Sum>::is_integer)
	{ return sum_bound<true, false>(l, r, budget); }
	std::optional<T> prev_value(int l, int r, const T &upper) const {
		check(l, r);
		if (!levels) return l < r && values[0] < upper ? std::optional<T>(values[0]) : std::nullopt;
		if (!ordered) {
			auto x = code(upper);
			if (!x) return std::nullopt;
			return neighbor<true>(l, r, x - 1);
		}
		int k = range_freq(l, r, upper);
		if (!k) return std::nullopt;
		return kth_smallest(l, r, k - 1);
	}
	std::optional<T> next_value(int l, int r, const T &lower) const {
		check(l, r);
		if (!levels) return l < r && !(values[0] < lower) ? std::optional<T>(values[0]) : std::nullopt;
		if (!ordered) {
			auto x = code(lower);
			if (x == values.size()) return std::nullopt;
			return neighbor<false>(l, r, x);
		}
		int k = range_freq(l, r, lower);
		if (k == r - l) return std::nullopt;
		return kth_smallest(l, r, k);
	}
};

template <class T>
WaveletMatrix(const std::vector<T> &, bool) -> WaveletMatrix<T>;


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/static_range_count_distinct.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n), previous(n), last(n, -1);
	for (auto &x : a) input.read(x);
	auto values = a;
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	for (int i = 0; i < n; ++i) {
		const auto j = std::lower_bound(values.begin(), values.end(), a[i]) - values.begin();
		previous[i] = last[j];
		last[j] = i;
	}
	WaveletMatrix matrix(previous);
	while (q--) {
		int l = 0, r = 0;
		input.read(l), input.read(r);
		output.println(matrix.range_freq(l, r, l));
	}
}
