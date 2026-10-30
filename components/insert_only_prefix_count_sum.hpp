#ifndef INSERT_ONLY_PREFIX_COUNT_SUM_HPP
#define INSERT_ONLY_PREFIX_COUNT_SUM_HPP

#include "fenwick_sum.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace insert_only_prefix_detail {
inline constexpr int level_count = 4, block_bits = 4 * level_count, block_size = 1 << block_bits;
struct CountSum {
	__int128_t sum = 0;
	int count = 0;
	CountSum &operator+=(const CountSum &other) { sum += other.sum; count += other.count; return *this; }
	CountSum &operator-=(const CountSum &other) { sum -= other.sum; count -= other.count; return *this; }
};
class Packed {
	std::array<std::vector<std::array<std::uint64_t, 16>>, level_count> levels;
	FenwickSum<CountSum> coarse;
	__int128_t base;
	std::uint64_t sum_mask;
	int sum_bits;
	[[maybe_unused]] bool vectorized = false;
	static void add_scalar(std::uint64_t *values, int first, std::uint64_t delta) {
		for (int i = first; i < 16; ++i) values[i] += delta;
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"), always_inline)) static void add_avx2(std::uint64_t *values, int first, std::uint64_t delta) {
		if (first == 16) return;
		int i = first & ~3;
		auto change = _mm256_set1_epi64x(static_cast<long long>(delta));
		auto active = _mm256_cmpgt_epi64(_mm256_setr_epi64x(i, i + 1, i + 2, i + 3), _mm256_set1_epi64x(first - 1));
		auto p = reinterpret_cast<__m256i *>(values + i);
		_mm256_storeu_si256(p, _mm256_add_epi64(_mm256_loadu_si256(p), _mm256_and_si256(change, active)));
		for (i += 4; i < 16; i += 4) {
			p = reinterpret_cast<__m256i *>(values + i);
			_mm256_storeu_si256(p, _mm256_add_epi64(_mm256_loadu_si256(p), change));
		}
	}
#endif
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) void insert_avx2(unsigned p, std::uint64_t delta) {
		auto encoded = (std::uint64_t(1) << sum_bits) + delta;
		add_avx2(levels[0][p >> 4].data(), p & 15, encoded);
		for (int level = 1; level < level_count; ++level)
			add_avx2(levels[level][p >> (4 * (level + 1))].data(), ((p >> (4 * level)) & 15) + 1, encoded);
		coarse.add(p >> block_bits, {base + delta, 1});
	}
#endif

  public:
	Packed(int n, __int128_t minimum, int bits) : coarse((std::size_t(n) + block_size - 1) >> block_bits), base(minimum), sum_mask((std::uint64_t(1) << bits) - 1), sum_bits(bits) {
		assert(n >= 0 && 0 < bits && bits < 64);
		for (int level = 0; level < level_count; ++level) levels[level].resize((std::size_t(n) + (std::size_t(1) << (4 * (level + 1))) - 1) >> (4 * (level + 1)));
#if defined(__x86_64__) || defined(__i386__)
		vectorized = __builtin_cpu_supports("avx2");
#endif
	}
	void insert(int p, std::uint64_t delta) {
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) { insert_avx2(unsigned(p), delta); return; }
#endif
		auto encoded = (std::uint64_t(1) << sum_bits) + delta;
		add_scalar(levels[0][unsigned(p) >> 4].data(), p & 15, encoded);
		for (int level = 1; level < level_count; ++level)
			add_scalar(levels[level][unsigned(p) >> (4 * (level + 1))].data(), ((unsigned(p) >> (4 * level)) & 15) + 1, encoded);
		coarse.add(unsigned(p) >> block_bits, {base + delta, 1});
	}
	CountSum prefix(int r) const {
		if (!r) return {};
		unsigned p = unsigned(r - 1);
		std::uint64_t encoded = levels[0][p >> 4][p & 15];
		for (int level = 1; level < level_count; ++level) encoded += levels[level][p >> (4 * (level + 1))][(p >> (4 * level)) & 15];
		auto result = coarse.prefix(p >> block_bits);
		int count = int(encoded >> sum_bits);
		result.sum += __int128_t(encoded & sum_mask) + base * count;
		result.count += count;
		return result;
	}
	CountSum fold(int l, int r) const {
		auto result = prefix(r);
		return result -= prefix(l);
	}
};
}

#endif
