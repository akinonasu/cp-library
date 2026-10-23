#ifndef INCREMENTAL_PREFIX_COUNT_HPP
#define INCREMENTAL_PREFIX_COUNT_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

class IncrementalPrefixCount {
	std::vector<std::vector<std::array<std::uint32_t, 16>>> levels;
	int n = 0, total = 0;
	[[maybe_unused]] bool vectorized = false;
	static void add_scalar(std::uint32_t *values, int first) {
		for (int i = first; i < 16; ++i) ++values[i];
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"), always_inline)) static void add_avx2(std::uint32_t *values, int first) {
		if (first == 16) return;
		int i = first & ~7;
		auto active = _mm256_cmpgt_epi32(_mm256_setr_epi32(i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7), _mm256_set1_epi32(first - 1));
		auto p = reinterpret_cast<__m256i *>(values + i);
		_mm256_storeu_si256(p, _mm256_sub_epi32(_mm256_loadu_si256(p), active));
		if (!i) {
			p = reinterpret_cast<__m256i *>(values + 8);
			_mm256_storeu_si256(p, _mm256_add_epi32(_mm256_loadu_si256(p), _mm256_set1_epi32(1)));
		}
	}
	__attribute__((target("avx2"))) void increment_avx2(std::size_t p) {
		add_avx2(levels[0][p >> 4].data(), p & 15);
		for (unsigned level = 1; level < levels.size(); ++level)
			add_avx2(levels[level][p >> (4 * (level + 1))].data(), ((p >> (4 * level)) & 15) + 1);
	}
#endif
  public:
	explicit IncrementalPrefixCount(int size = 0) : n(size) {
		assert(size >= 0);
		for (std::size_t width = std::size_t(size); width > 1 || levels.empty();) {
			width = (width + 15) >> 4;
			levels.emplace_back(width);
		}
#if defined(__x86_64__) || defined(__i386__)
		vectorized = __builtin_cpu_supports("avx2");
#endif
	}
	int size() const { return n; }
	void increment(int p) {
		assert(0 <= p && p < n && total < std::numeric_limits<int>::max());
		++total;
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) { increment_avx2(unsigned(p)); return; }
#endif
		std::size_t index = unsigned(p);
		add_scalar(levels[0][index >> 4].data(), index & 15);
		for (unsigned level = 1; level < levels.size(); ++level)
			add_scalar(levels[level][index >> (4 * (level + 1))].data(), ((index >> (4 * level)) & 15) + 1);
	}
	int prefix(int r) const {
		assert(0 <= r && r <= n);
		if (!r) return 0;
		std::size_t p = unsigned(r - 1);
		std::uint32_t result = levels[0][p >> 4][p & 15];
		for (unsigned level = 1; level < levels.size(); ++level) result += levels[level][p >> (4 * (level + 1))][(p >> (4 * level)) & 15];
		return int(result);
	}
};

#endif
