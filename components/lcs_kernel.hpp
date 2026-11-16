#ifndef LCS_KERNEL_HPP
#define LCS_KERNEL_HPP

#include "simd_capabilities.hpp"
#include "sequence_bit_masks.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>
#if defined(__x86_64__)
#include <immintrin.h>
#endif

namespace lcs_detail {
using Word = std::uint64_t;
using Symbol = sequence_bit_masks_detail::Symbol;
using MaskWorkspace = sequence_bit_masks_detail::MaskWorkspace;

inline bool near_candidate(std::size_t n, std::size_t m) {
	return (n > m ? n - m : m - n) <= 8 && std::uint64_t(n) * ((m + 63) / 64) >= 65536;
}

template <class Sequence, class Output = std::nullptr_t>
int near(Sequence s, Sequence t, Output *out = nullptr) {
	constexpr int cap = 8, base = cap + 1, width = 2 * cap + 3;
	constexpr bool restore = !std::is_same_v<Output, std::nullptr_t>;
	const int n = int(s.size()), m = int(t.size());
	if (n - m > cap || m - n > cap) return -1;
	std::array<int, width> row;
	std::array<std::array<int, width>, restore ? cap + 1 : 0> trace;
	row.fill(-1);
	int initial = 0;
	while (initial < std::min(n, m) && s[initial] == t[initial]) ++initial;
	row[base] = initial;
	if constexpr (restore) trace[0] = row;
	int distance = initial == n && initial == m ? 0 : -1;
	for (int d = 1; distance < 0 && d <= cap; ++d) {
		for (int k = -d; k <= d; k += 2) {
			int x = -1, a = row[base + k - 1], b = row[base + k + 1];
			if (a >= 0 && a < n) x = a + 1;
			if (b >= 0 && b - (k + 1) < m) x = std::max(x, b);
			if (x >= 0) {
				int y = x - k;
				while (x < n && y < m && s[x] == t[y]) ++x, ++y;
				if (x == n && y == m) {
					distance = d;
					break;
				}
			}
			row[base + k] = x;
		}
		if constexpr (restore) trace[d] = row;
	}
	if (distance < 0) return -1;
	const int length = int((std::size_t(n) + std::size_t(m) - distance) / 2);
	if constexpr (restore) {
		const std::size_t start = out->size();
		out->resize(start + length);
		std::size_t position = out->size();
		int x = n, y = m;
		for (int d = distance; d; --d) {
			int k = x - y;
			const auto &previous = trace[d - 1];
			int a = previous[base + k - 1], b = previous[base + k + 1];
			bool deletion = a >= 0 && a < n && (b < 0 || b - (k + 1) == m || a + 1 >= b);
			int previous_k = k + (deletion ? -1 : 1);
			int previous_x = previous[base + previous_k];
			int begin = previous_x + int(deletion);
			while (x > begin) (*out)[--position] = s[--x], --y;
			x = previous_x;
			y = x - previous_k;
		}
		while (x) (*out)[--position] = s[--x];
		assert(position == start);
	}
	return length;
}

#if defined(__x86_64__)
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
template <bool reverse, class Sequence, class Lookup>
__attribute__((target("avx2"))) inline void update_avx2(Sequence s, Lookup &masks, Word *__restrict row, int words) {
	const __m256i ones = _mm256_set1_epi64x(-1), one = _mm256_set1_epi64x(1), positions = _mm256_setr_epi64x(0, 1, 2, 3);
	for (int i = 0, n = int(s.size()); i < n; ++i) {
		const Word *__restrict mask = masks(s[reverse ? n - 1 - i : i]);
		if (!mask) continue;
		unsigned carry = 0;
		for (int j = 0; j < words; j += 4) {
			__m256i v = _mm256_load_si256(reinterpret_cast<const __m256i *>(row + j));
			__m256i u = _mm256_and_si256(v, _mm256_load_si256(reinterpret_cast<const __m256i *>(mask + j))), sum = _mm256_add_epi64(v, u);
			unsigned g = unsigned(_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_or_si256(u, _mm256_andnot_si256(sum, v)))));
			unsigned propagation = unsigned(_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(sum, ones))));
			unsigned incoming = (propagation + (g << 1) + carry) ^ propagation;
			carry = incoming >> 4;
			__m256i increments = _mm256_and_si256(_mm256_srlv_epi64(_mm256_set1_epi64x(incoming), positions), one);
			sum = _mm256_add_epi64(sum, increments);
			_mm256_store_si256(reinterpret_cast<__m256i *>(row + j), _mm256_or_si256(sum, _mm256_xor_si256(v, u)));
		}
	}
}
#if CP_LIBRARY_AVX512
template <bool reverse, class Sequence, class Lookup>
__attribute__((target("avx512f"))) inline void update_avx512(Sequence s, Lookup &masks, Word *__restrict row, int words) {
	const __m512i ones = _mm512_set1_epi64(-1), one = _mm512_set1_epi64(1);
	for (int i = 0, n = int(s.size()); i < n; ++i) {
		const Word *__restrict mask = masks(s[reverse ? n - 1 - i : i]);
		if (!mask) continue;
		unsigned carry = 0;
		for (int j = 0; j < words; j += 8) {
			__m512i v = _mm512_load_si512(row + j), u = _mm512_and_si512(v, _mm512_load_si512(mask + j)), sum = _mm512_add_epi64(v, u);
			unsigned g = unsigned(_mm512_cmp_epu64_mask(sum, u, _MM_CMPINT_LT)), propagation = unsigned(_mm512_cmpeq_epi64_mask(sum, ones));
			unsigned incoming = (propagation + (g << 1) + carry) ^ propagation;
			carry = incoming >> 8;
			sum = _mm512_mask_add_epi64(sum, __mmask8(incoming), sum, one);
			_mm512_store_si512(row + j, _mm512_ternarylogic_epi64(sum, v, u, 0xf6));
		}
	}
}
#endif
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif
#endif

#if defined(__x86_64__)
template <bool reverse>
__attribute__((target("avx2"))) inline void avx2(std::string_view s, const std::array<const Word *, 256> &masks, Word *__restrict row, int words) {
	auto lookup = [&](char c) { return masks[static_cast<unsigned char>(c)]; };
	update_avx2<reverse>(s, lookup, row, words);
}
#if CP_LIBRARY_AVX512
template <bool reverse>
__attribute__((target("avx512f"))) inline void avx512(std::string_view s, const std::array<const Word *, 256> &masks, Word *__restrict row, int words) {
	auto lookup = [&](char c) { return masks[static_cast<unsigned char>(c)]; };
	update_avx512<reverse>(s, lookup, row, words);
}
#endif
#endif

struct MaskPositions {
	MaskWorkspace &workspace;
	std::vector<Symbol> used;
	std::vector<int> positions;
	MaskPositions(std::span<const Symbol> t, MaskWorkspace &scratch, bool reverse) : workspace(scratch) {
		for (Symbol c : t) {
			if (workspace.head[c] == -1) {
				workspace.head[c] = 0;
				used.push_back(c);
			}
			++workspace.head[c];
		}
		int size = 0;
		for (Symbol c : used) {
			int frequency = workspace.head[c];
			workspace.head[c] = workspace.tail[c] = size;
			size += frequency;
		}
		positions.resize(size);
		for (int j = 0, m = int(t.size()); j < m; ++j)
			positions[workspace.head[t[reverse ? m - 1 - j : j]]++] = j;
	}
	MaskPositions(const MaskPositions &) = delete;
	MaskPositions &operator=(const MaskPositions &) = delete;
	~MaskPositions() {
		for (Symbol c : used) workspace.head[c] = workspace.tail[c] = -1;
	}
};

struct DenseMasks {
	MaskPositions &positions;
	std::vector<Word> storage;
	Word *data, *row, *scratch;
	int stride, before_start = 0, before_end = 0;
	DenseMasks(MaskPositions &group, int words) : positions(group) {
		stride = (words + 7) & ~7;
		auto &workspace = positions.workspace;
		int count = 0;
		for (Symbol c : positions.used)
			count += workspace.head[c] - workspace.tail[c] > words;
		storage.resize(std::size_t(count + 2) * stride + 7);
		data = storage.data() + ((-reinterpret_cast<std::uintptr_t>(storage.data()) & 63) / sizeof(Word));
		row = data + std::size_t(count) * stride;
		scratch = row + stride;
		std::fill_n(row, stride, ~Word(0));
		int id = 0;
		for (Symbol c : positions.used) {
			int start = workspace.tail[c], end = workspace.head[c];
			if (end - start <= words) continue;
			Word *mask = data + std::size_t(id) * stride;
			for (int i = start; i < end; ++i) {
				int j = positions.positions[i];
				mask[j / 64] |= Word(1) << (j % 64);
			}
			workspace.head[c] = -2 - id++;
		}
	}
	const Word *operator()(Symbol c) {
		int end = positions.workspace.head[c];
		if (end == -1) return nullptr;
		if (end < -1) return data + std::size_t(-2 - end) * stride;
		int start = positions.workspace.tail[c];
		if (before_start == start && before_end == end) return scratch;
		for (int i = before_start; i < before_end; ++i) scratch[positions.positions[i] / 64] = 0;
		before_start = start, before_end = end;
		for (int i = start; i < end; ++i) {
			int j = positions.positions[i];
			scratch[j / 64] |= Word(1) << (j % 64);
		}
		return scratch;
	}
};

template <bool reverse = false, int backend = 0>
std::vector<Word> profile_dense(std::span<const Symbol> s, MaskPositions &positions, int words) {
	DenseMasks masks(positions, words);
#if defined(__x86_64__)
	int kernel = backend;
	if (backend == 0 && words >= 8) {
#if CP_LIBRARY_AVX512
		if (__builtin_cpu_supports("avx512f")) kernel = 3;
		else
#endif
		if (__builtin_cpu_supports("avx2")) kernel = 2;
	}
	if (kernel >= 2) {
#if CP_LIBRARY_AVX512
		if (kernel == 3) update_avx512<reverse>(s, masks, masks.row, masks.stride);
		else
#endif
		update_avx2<reverse>(s, masks, masks.row, masks.stride);
		return std::vector<Word>(masks.row, masks.row + words);
	}
#endif
	for (int i = 0, n = int(s.size()); i < n; ++i) {
		const Word *mask = masks(s[reverse ? n - 1 - i : i]);
		if (!mask) continue;
		Word carry = 0;
		for (int j = 0; j < words; ++j) {
			Word v = masks.row[j], u = v & mask[j];
			__uint128_t sum = __uint128_t(v) + u + carry;
			masks.row[j] = Word(sum) | (v ^ u);
			carry = Word(sum >> 64);
		}
	}
	return std::vector<Word>(masks.row, masks.row + words);
}
template <bool reverse = false>
std::vector<Word> profile_sparse(std::span<const Symbol> s, MaskPositions &positions, int words) {
	auto &workspace = positions.workspace;
	std::vector<Word> masks;
	std::vector<int> indices;
	masks.reserve(positions.positions.size());
	indices.reserve(positions.positions.size());
	for (Symbol c : positions.used) {
		int i = workspace.tail[c], end = workspace.head[c];
		workspace.tail[c] = int(masks.size());
		while (i < end) {
			int word = positions.positions[i] / 64;
			Word mask = 0;
			do { mask |= Word(1) << (positions.positions[i++] % 64); }
			while (i < end && positions.positions[i] / 64 == word);
			masks.push_back(mask);
			indices.push_back(word);
		}
		workspace.head[c] = int(masks.size());
	}
	std::vector<Word> row(words, ~Word(0)), summary((words + 63) / 64);
	auto set = [&](int j, Word value) {
		row[j] = value;
		Word bit = Word(1) << (j % 64);
		if (value != ~Word(0)) summary[j / 64] |= bit;
		else summary[j / 64] &= ~bit;
	};
	auto propagate = [&](int start, int end) {
		if (start >= end) return Word(1);
		int block = start / 64;
		Word bits = summary[block] & (~Word(0) << (start % 64));
		while (!bits) {
			if (++block > (end - 1) / 64) return Word(1);
			bits = summary[block];
		}
		int j = block * 64 + std::countr_zero(bits);
		if (j >= end) return Word(1);
		set(j, row[j] | (row[j] + 1));
		return Word(0);
	};
	for (int i = 0, n = int(s.size()); i < n; ++i) {
		Symbol c = s[reverse ? n - 1 - i : i];
		Word carry = 0;
		int next = 0;
		for (int index = workspace.tail[c], end = workspace.head[c]; index < end; ++index) {
			int j = indices[index];
			if (carry) carry = propagate(next, j);
			Word v = row[j], u = v & masks[index];
			__uint128_t sum = __uint128_t(v) + u + carry;
			set(j, Word(sum) | (v ^ u));
			carry = Word(sum >> 64);
			next = j + 1;
		}
		if (carry) propagate(next, words);
	}
	return row;
}

}

#endif
