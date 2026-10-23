#ifndef OFFLINE_RANGE_KTH_COMPONENT_HPP
#define OFFLINE_RANGE_KTH_COMPONENT_HPP

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace offline_range_kth_detail {
struct Frontier {
	std::vector<std::uint32_t> left, right, rank, answer;
	explicit Frontier(std::size_t n) : left(n), right(n), rank(n), answer(n) {}
};
inline void advance(Frontier &queries, const std::vector<std::uint32_t> &current, const std::vector<std::uint32_t> &prefix,
	unsigned zeros, bool finish, std::size_t start = 0) {
	for (std::size_t i = start; i < queries.left.size(); ++i) {
		auto left = queries.left[i], right = queries.right[i];
		auto a = prefix[left], b = prefix[right], count = right - left - b + a;
		bool one = queries.rank[i] >= count;
		if (finish) queries.answer[i] = (current[left] & ~1U) | unsigned(one);
		else {
			queries.left[i] = one ? zeros + a : left - a;
			queries.right[i] = one ? zeros + b : right - b;
			queries.rank[i] -= one ? count : 0;
		}
	}
}
inline void run_scalar(Frontier &queries, std::vector<std::uint32_t> current, unsigned bits) {
	auto n = current.size();
	std::vector<std::uint32_t> next(n), prefix(n + 1);
	while (bits--) {
		for (std::size_t i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + ((current[i] >> bits) & 1);
		unsigned zeros = unsigned(n) - prefix[n];
		if (bits) {
			std::array<unsigned, 2> cursor{0, zeros};
			for (auto value : current) next[cursor[(value >> bits) & 1]++] = value;
		}
		advance(queries, current, prefix, zeros, bits == 0);
		if (bits) current.swap(next);
	}
}
#if defined(__x86_64__) || defined(__i386__)
struct Tables {
	std::array<unsigned char, 8> prefix{};
	std::array<std::uint32_t, 2> packed{};
	std::array<int, 8> permutation{};
};
inline constexpr auto tables = [] {
	std::array<Tables, 256> result{};
	for (unsigned mask = 0; mask < 256; ++mask) {
		unsigned count = 0, selected = 0;
		for (unsigned i = 0; i < 8; ++i) {
			if (!(i & 3)) result[mask].packed[i >> 2] = count << 8;
			else result[mask].packed[i >> 2] |= (count - (result[mask].packed[i >> 2] >> 8)) << (2 * (i & 3));
			if ((mask >> i) & 1) result[mask].permutation[selected++] = int(i);
			count += (mask >> i) & 1;
			result[mask].prefix[i] = static_cast<unsigned char>(count);
		}
	}
	return result;
}();
inline bool packed_ranks(const Frontier &queries, std::size_t n) {
	if (n < 32768 || n > 0x00ffffff || queries.left.size() < 4096) return false;
	unsigned shift = std::bit_width(n) - 6;
	std::size_t stride = queries.left.size() / 32;
	std::uint64_t occupied = 0;
	for (std::size_t i = 0; i < 32; ++i) {
		occupied |= std::uint64_t(1) << (queries.left[i * stride] >> shift);
		occupied |= std::uint64_t(1) << (queries.right[i * stride] >> shift);
	}
	return std::popcount(occupied) >= 12;
}
__attribute__((target("avx2"), always_inline))
inline void store_selected(std::uint32_t *output, unsigned remaining, __m256i values, unsigned mask) {
	auto permutation = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(tables[mask].permutation.data()));
	auto selected = _mm256_permutevar8x32_epi32(values, permutation);
	if (remaining >= 8) _mm256_storeu_si256(reinterpret_cast<__m256i *>(output), selected);
	else {
		auto active = _mm256_cmpgt_epi32(_mm256_set1_epi32(std::popcount(mask)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		_mm256_maskstore_epi32(reinterpret_cast<int *>(output), active, selected);
	}
}
template <bool Packed, bool Finish>
__attribute__((target("avx2")))
void route(Frontier &queries, const std::vector<std::uint32_t> &current, const std::vector<std::uint32_t> &prefix, unsigned zeros) {
	std::size_t i = 0;
	auto middle = _mm256_set1_epi32(int(zeros));
	for (; i + 8 <= queries.left.size(); i += 8) {
		auto left = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(queries.left.data() + i));
		auto right = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(queries.right.data() + i));
		auto rank = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(queries.rank.data() + i));
		auto a = _mm256_i32gather_epi32(reinterpret_cast<const int *>(prefix.data()), Packed ? _mm256_srli_epi32(left, 2) : left, 4);
		auto b = _mm256_i32gather_epi32(reinterpret_cast<const int *>(prefix.data()), Packed ? _mm256_srli_epi32(right, 2) : right, 4);
		if constexpr (Packed) {
			auto mask = _mm256_set1_epi32(3);
			auto sa = _mm256_slli_epi32(_mm256_and_si256(left, mask), 1), sb = _mm256_slli_epi32(_mm256_and_si256(right, mask), 1);
			a = _mm256_add_epi32(_mm256_srli_epi32(a, 8), _mm256_and_si256(_mm256_srlv_epi32(a, sa), mask));
			b = _mm256_add_epi32(_mm256_srli_epi32(b, 8), _mm256_and_si256(_mm256_srlv_epi32(b, sb), mask));
		}
		auto lzero = _mm256_sub_epi32(left, a), rzero = _mm256_sub_epi32(right, b);
		auto count = _mm256_sub_epi32(rzero, lzero), take_zero = _mm256_cmpgt_epi32(count, rank);
		if constexpr (Finish) {
			auto value = _mm256_i32gather_epi32(reinterpret_cast<const int *>(current.data()), left, 4);
			value = _mm256_or_si256(_mm256_and_si256(value, _mm256_set1_epi32(-2)), _mm256_andnot_si256(take_zero, _mm256_set1_epi32(1)));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(queries.answer.data() + i), value);
		} else {
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(queries.left.data() + i), _mm256_blendv_epi8(_mm256_add_epi32(middle, a), lzero, take_zero));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(queries.right.data() + i), _mm256_blendv_epi8(_mm256_add_epi32(middle, b), rzero, take_zero));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(queries.rank.data() + i), _mm256_sub_epi32(rank, _mm256_andnot_si256(take_zero, count)));
		}
	}
	for (; i < queries.left.size(); ++i) {
		auto left = queries.left[i], right = queries.right[i];
		auto a = prefix[Packed ? left >> 2 : left], b = prefix[Packed ? right >> 2 : right];
		if constexpr (Packed) {
			a = (a >> 8) + ((a >> (2 * (left & 3))) & 3);
			b = (b >> 8) + ((b >> (2 * (right & 3))) & 3);
		}
		auto count = right - left - b + a;
		bool one = queries.rank[i] >= count;
		if constexpr (Finish) queries.answer[i] = (current[left] & ~1U) | unsigned(one);
		else {
			queries.left[i] = one ? zeros + a : left - a;
			queries.right[i] = one ? zeros + b : right - b;
			queries.rank[i] -= one ? count : 0;
		}
	}
}
template <bool Packed>
__attribute__((target("avx2")))
void run_avx2(Frontier &queries, std::vector<std::uint32_t> current, unsigned bits) {
	std::size_t n = current.size();
	current.resize(n + 8);
	std::vector<std::uint32_t> next(n + 8), prefix(Packed ? (n + 3) / 4 + 1 : n + 1);
	std::vector<unsigned char> masks((n + 7) / 8);
	while (bits--) {
		unsigned ones = 0;
		std::size_t i = 0;
		for (; i + 8 <= n; i += 8) {
			auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(current.data() + i));
			auto selected = _mm256_sll_epi32(value, _mm_cvtsi32_si128(31 - int(bits)));
			unsigned mask = unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(selected)));
			masks[i / 8] = static_cast<unsigned char>(mask);
			if constexpr (Packed) {
				prefix[i / 4] = (ones << 8) + tables[mask].packed[0];
				prefix[i / 4 + 1] = (ones << 8) + tables[mask].packed[1];
			} else {
				auto local = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(tables[mask].prefix.data())));
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(prefix.data() + i + 1), _mm256_add_epi32(local, _mm256_set1_epi32(int(ones))));
			}
			ones += unsigned(std::popcount(mask));
		}
		for (; i <= n; ++i) {
			if constexpr (Packed) {
				if (!(i & 3)) prefix[i / 4] = ones << 8;
				else prefix[i / 4] |= (ones - (prefix[i / 4] >> 8)) << (2 * (i & 3));
			} else prefix[i] = ones;
			if (i < n) ones += (current[i] >> bits) & 1;
		}
		unsigned zeros = unsigned(n) - ones;
		if (bits) {
			unsigned zero = 0, one = zeros;
			i = 0;
			for (; i + 8 <= n; i += 8) {
				auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(current.data() + i));
				unsigned mask = masks[i / 8], count = unsigned(std::popcount(mask));
				store_selected(next.data() + zero, zeros - zero, value, mask ^ 255);
				store_selected(next.data() + one, unsigned(n + 8) - one, value, mask);
				zero += 8 - count; one += count;
			}
			for (; i < n; ++i) {
				auto value = current[i];
				if ((value >> bits) & 1) next[one++] = value;
				else next[zero++] = value;
			}
			route<Packed, false>(queries, current, prefix, zeros);
			current.swap(next);
		} else route<Packed, true>(queries, current, prefix, zeros);
	}
}
#endif
inline void run(Frontier &queries, std::vector<std::uint32_t> current, unsigned bits) {
#if defined(__x86_64__) || defined(__i386__)
	if (__builtin_cpu_supports("avx2")) {
		if (packed_ranks(queries, current.size())) run_avx2<true>(queries, std::move(current), bits);
		else run_avx2<false>(queries, std::move(current), bits);
		return;
	}
#endif
	run_scalar(queries, std::move(current), bits);
}
}

#endif
