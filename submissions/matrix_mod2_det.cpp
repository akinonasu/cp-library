#line 1 "test/matrix_mod2_det.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/matrix_det_mod_2"
#line 1 "test/fixtures/matrix_mod2_io.hpp"



#line 1 "math/matrix_mod2.hpp"



#line 1 "components/matrix_mod2_kernel.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace matrix_mod2_detail {
using Word = std::uint64_t;

inline std::size_t words(int n) { return (std::size_t(n) + 63) / 64; }
inline Word mask(int n) { return n == 64 ? ~Word(0) : (Word(1) << n) - 1; }
inline bool bit(const Word* row, int column) { return (row[unsigned(column) / 64] >> (column & 63)) & 1; }
inline void xor_row(Word* __restrict__ to, const Word* __restrict__ from, int first, int stride) {
	for (int j = first; j < stride; ++j) to[j] ^= from[j];
}

inline std::vector<int> eliminate(std::vector<Word>& data, int rows, int stride, int columns, bool reduced) {
	assert(rows >= 0 && stride >= 0 && columns >= 0 && std::int64_t(columns) <= std::int64_t(stride) * 64);
	assert(data.size() == std::size_t(rows) * stride);
	std::vector<int> pivots;
	if (!rows || !columns) return pivots;
	std::vector<Word*> row(rows);
	for (int i = 0; i < rows; ++i) row[i] = data.data() + std::size_t(i) * stride;
	pivots.reserve(std::min(rows, columns));
	auto next_column = [&](int start, int column) {
		int next = columns;
		for (int i = start; i < rows; ++i) {
			int j = column / 64;
			Word value = row[i][j] & (~Word(0) << (column & 63));
			while (!value && std::int64_t(j + 1) * 64 < next) value = row[i][++j];
			if (value) next = std::min<std::int64_t>(next, std::int64_t(j) * 64 + std::countr_zero(value));
		}
		return next;
	};
	int rank = 0, column = 0;
	if (rows < 128 || stride == 1) {
		while (rank < rows && column < columns) {
			int found = rank;
			while (found < rows && !bit(row[found], column)) ++found;
			if (found == rows) { column = next_column(rank, column); continue; }
			std::swap(row[rank], row[found]);
			for (int i = reduced ? 0 : rank + 1; i < rows; ++i)
				if (i != rank && bit(row[i], column)) xor_row(row[i], row[rank], column / 64, stride);
			pivots.push_back(column++);
			++rank;
		}
	} else {
		std::vector<unsigned char> applied(rows);
		std::vector<Word> table;
		while (rank < rows && column < columns) {
			int start = rank, first = column / 64, count = 0;
			std::array<int, 8> panel{};
			std::fill(applied.begin() + rank, applied.end(), 0);
			while (count < 8 && rank < rows && column < columns) {
				int found = rank;
				while (found < rows) {
					for (int k = applied[found]; k < count; ++k)
						if (bit(row[found], panel[k])) xor_row(row[found], row[start + k], first, stride);
					applied[found] = static_cast<unsigned char>(count);
					if (bit(row[found], column)) break;
					++found;
				}
				if (found == rows) { column = next_column(rank, column); continue; }
				std::swap(row[rank], row[found]);
				std::swap(applied[rank], applied[found]);
				panel[count++] = column;
				pivots.push_back(column++);
				++rank;
			}
			if (!count) break;
			if (!reduced && rank == rows) break;
			for (int k = count; k--;) for (int j = 0; j < k; ++j)
				if (bit(row[start + j], panel[k])) xor_row(row[start + j], row[start + k], first, stride);
			bool contiguous = panel[count - 1] - panel[0] == count - 1;
			int pivot_word = panel[0] / 64, offset = panel[0] & 63;
			unsigned valid = (1U << count) - 1;
			std::uint64_t active = 0, additions = 0;
			for (int i = reduced ? 0 : rank; i < rows; ++i) {
				if (start <= i && i < rank) { applied[i] = 0; continue; }
				unsigned pattern = 0;
				if (contiguous) {
					Word value = row[i][pivot_word] >> offset;
					if (offset + count > 64) value |= row[i][pivot_word + 1] << (64 - offset);
					pattern = unsigned(value) & valid;
				} else for (int k = 0; k < count; ++k) pattern |= unsigned(bit(row[i], panel[k])) << k;
				applied[i] = static_cast<unsigned char>(pattern);
				active += pattern != 0;
				additions += std::popcount(pattern);
			}
			if (!active) continue;
			if (additions <= valid + active) {
				for (int i = reduced ? 0 : rank; i < rows; ++i) {
					if (start <= i && i < rank) continue;
					unsigned pattern = applied[i];
					while (pattern) {
						xor_row(row[i], row[start + std::countr_zero(pattern)], first, stride);
						pattern &= pattern - 1;
					}
				}
				continue;
			}
			if (table.empty()) table.resize((std::size_t(1) << std::min({8, rows, columns})) * stride);
			for (unsigned pattern = 1; pattern < (1U << count); ++pattern) {
				unsigned previous = pattern & (pattern - 1), k = std::countr_zero(pattern);
				Word* __restrict__ to = table.data() + std::size_t(pattern) * stride;
				const Word* __restrict__ from = table.data() + std::size_t(previous) * stride;
				const Word* __restrict__ pivot = row[start + k];
				for (int j = first; j < stride; ++j) to[j] = from[j] ^ pivot[j];
			}
			for (int i = reduced ? 0 : rank; i < rows; ++i) {
				if (start <= i && i < rank) continue;
				unsigned pattern = applied[i];
				if (pattern) xor_row(row[i], table.data() + std::size_t(pattern) * stride, first, stride);
			}
		}
	}
	std::vector<int> order(rows);
	for (int i = 0; i < rows; ++i) order[i] = int((row[i] - data.data()) / stride);
	std::vector<Word> temporary;
	for (int i = 0; i < rows; ++i) {
		if (order[i] < 0 || order[i] == i) continue;
		temporary.assign(data.begin() + std::size_t(i) * stride, data.begin() + std::size_t(i + 1) * stride);
		int current = i;
		while (order[current] != i) {
			int next = order[current];
			std::copy_n(data.data() + std::size_t(next) * stride, stride, data.data() + std::size_t(current) * stride);
			order[current] = -1;
			current = next;
		}
		std::copy(temporary.begin(), temporary.end(), data.begin() + std::size_t(current) * stride);
		order[current] = -1;
	}
	return pivots;
}

inline std::vector<Word> transpose(const std::vector<Word>& data, int n, int m) {
	std::size_t input = words(m), output = words(n);
	std::vector<Word> result(std::size_t(m) * output);
	if (!n || !m) return result;
	if (n < 8 || m < 8) {
		for (int i = 0; i < n; ++i) for (std::size_t j = 0; j < input; ++j) {
			Word value = data[std::size_t(i) * input + j];
			if (j + 1 == input && (m & 63)) value &= mask(m & 63);
			while (value) {
				std::size_t column = j * 64 + std::countr_zero(value);
				result[column * output + unsigned(i) / 64] |= Word(1) << (i & 63);
				value &= value - 1;
			}
		}
		return result;
	}
	for (int i = 0; i < n;) {
		int height = std::min(64, n - i);
		for (int j = 0; j < m;) {
			int width = std::min(64, m - j);
			std::array<Word, 64> block{};
			Word valid = mask(width);
			for (int k = 0; k < height; ++k) block[k] = data[std::size_t(i + k) * input + unsigned(j) / 64] & valid;
			Word select = 0xffffffffULL;
			for (unsigned shift = 32; shift; shift >>= 1, select ^= select << shift) {
				for (unsigned k = 0; k < 64; k = (k + shift + 1) & ~shift) {
					Word change = ((block[k] >> shift) ^ block[k + shift]) & select;
					block[k] ^= change << shift;
					block[k + shift] ^= change;
				}
			}
			for (int k = 0; k < width; ++k) result[std::size_t(j + k) * output + unsigned(i) / 64] = block[k];
			j += width;
		}
		i += height;
	}
	return result;
}

inline std::vector<Word> multiply(const std::vector<Word>& a, const std::vector<Word>& b, int n, int inner, int m) {
	std::size_t left = words(inner), out = words(m);
	std::vector<Word> result(std::size_t(n) * out);
	if (!n || !inner || !m) return result;
	std::uint64_t count = 0;
	bool inspect = m <= 16 || (n >= 128 && inner >= 16 && m >= 64);
	if (inspect) for (Word value : a) count += std::popcount(value);
	if (inspect && !count) return result;
	if (n >= 128 && inner >= 16 && m >= 64) {
		std::uint64_t existing = std::min(count, ((std::uint64_t(inner) + 7) / 8) * (std::uint64_t(n) + 256));
		std::uint64_t complement = std::uint64_t(n) * inner - count + inner + n;
		if (complement * 2 < existing) {
			std::vector<Word> sum(out);
			for (int k = 0; k < inner; ++k) xor_row(sum.data(), b.data() + std::size_t(k) * out, 0, int(out));
			for (int i = 0; i < n; ++i) {
				Word* target = result.data() + std::size_t(i) * out;
				std::copy(sum.begin(), sum.end(), target);
				for (std::size_t j = 0; j < left; ++j) {
					Word value = ~a[std::size_t(i) * left + j];
					if (j + 1 == left && (inner & 63)) value &= mask(inner & 63);
					while (value) {
						std::size_t k = j * 64 + std::countr_zero(value);
						xor_row(target, b.data() + k * out, 0, int(out));
						value &= value - 1;
					}
				}
				if (m & 63) target[out - 1] &= mask(m & 63);
			}
			return result;
		}
	}
	if (m <= 16 && count > std::uint64_t(n) * m * left + std::uint64_t(inner) * 4) {
		auto transposed = transpose(b, inner, m);
		for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
			Word value = 0;
			for (std::size_t k = 0; k < left; ++k) value ^= a[std::size_t(i) * left + k] & transposed[std::size_t(j) * left + k];
			if (std::popcount(value) & 1) result[std::size_t(i)] |= Word(1) << j;
		}
		return result;
	}
	bool table_mode = n >= 128 && inner >= 16 && m >= 64 && count > ((std::uint64_t(inner) + 7) / 8) * (std::uint64_t(n) + 256);
	if (table_mode) {
		bool fused = inner >= 64 && out <= 64;
		std::vector<Word> table(std::size_t(fused ? 2048 : 256) * out);
		int begin = 0;
		if (fused) for (; inner - begin >= 64; begin += 64) {
			for (unsigned group = 0; group < 8; ++group) {
				Word* base = table.data() + std::size_t(group) * 256 * out;
				for (unsigned pattern = 1; pattern < 256; ++pattern) {
					unsigned previous = pattern & (pattern - 1), k = std::countr_zero(pattern);
					Word* __restrict__ to = base + std::size_t(pattern) * out;
					const Word* __restrict__ from = base + std::size_t(previous) * out;
					const Word* __restrict__ row = b.data() + std::size_t(begin + int(group * 8 + k)) * out;
					for (std::size_t j = 0; j < out; ++j) to[j] = from[j] ^ row[j];
				}
			}
			for (int i = 0; i < n; ++i) {
				Word selectors = a[std::size_t(i) * left + unsigned(begin) / 64];
				const Word* source[8];
				for (unsigned group = 0; group < 8; ++group)
					source[group] = table.data() + (std::size_t(group) * 256 + ((selectors >> (group * 8)) & 255)) * out;
				Word* __restrict__ to = result.data() + std::size_t(i) * out;
				auto a0 = source[0], a1 = source[1], a2 = source[2], a3 = source[3];
				auto a4 = source[4], a5 = source[5], a6 = source[6], a7 = source[7];
				for (std::size_t j = 0; j < out; ++j) to[j] ^= a0[j] ^ a1[j] ^ a2[j] ^ a3[j] ^ a4[j] ^ a5[j] ^ a6[j] ^ a7[j];
			}
		}
		for (; begin < inner;) {
			int width = std::min(8, inner - begin);
			for (unsigned pattern = 1; pattern < (1U << width); ++pattern) {
				unsigned previous = pattern & (pattern - 1), k = std::countr_zero(pattern);
				Word* __restrict__ to = table.data() + std::size_t(pattern) * out;
				const Word* __restrict__ from = table.data() + std::size_t(previous) * out;
				const Word* __restrict__ row = b.data() + std::size_t(begin + k) * out;
				for (std::size_t j = 0; j < out; ++j) to[j] = from[j] ^ row[j];
			}
			for (int i = 0; i < n; ++i) {
				unsigned pattern = unsigned(a[std::size_t(i) * left + unsigned(begin) / 64] >> (begin & 63)) & ((1U << width) - 1);
				if (pattern) xor_row(result.data() + std::size_t(i) * out, table.data() + std::size_t(pattern) * out, 0, int(out));
			}
			begin += width;
		}
	} else {
		for (int i = 0; i < n; ++i) for (std::size_t j = 0; j < left; ++j) {
			Word value = a[std::size_t(i) * left + j];
			if (j + 1 == left && (inner & 63)) value &= mask(inner & 63);
			while (value) {
				std::size_t k = j * 64 + std::countr_zero(value);
				xor_row(result.data() + std::size_t(i) * out, b.data() + k * out, 0, int(out));
				value &= value - 1;
			}
		}
	}
	if (m & 63) for (int i = 0; i < n; ++i) result[std::size_t(i) * out + out - 1] &= mask(m & 63);
	return result;
}
}


#line 1 "data_structure/dynamic_bitset.hpp"



#line 1 "components/bitset_ops.hpp"



#line 1 "components/simd_capabilities.hpp"


#line 1 "components/compiler_features.hpp"



#if defined(__x86_64__) || defined(__i386__)
#define CP_X86 1
#else
#define CP_X86 0
#endif
#if CP_X86 && defined(__GNUC__) && !defined(__clang__)
#define CP_GCC_X86 1
#else
#define CP_GCC_X86 0
#endif
#if CP_X86 && (defined(__GNUC__) || defined(__clang__))
#define CP_GNU_X86 1
#else
#define CP_GNU_X86 0
#endif
#if CP_GCC_X86 && defined(__linux__) && defined(__x86_64__)
#define CP_GCC_LINUX64 1
#else
#define CP_GCC_LINUX64 0
#endif


#line 4 "components/simd_capabilities.hpp"

#if CP_GCC_X86
_Pragma("GCC push_options")
_Pragma("GCC target(\"no-avx512f,avx512f\")")
#ifdef __AVX512F__
#define CP_LIBRARY_AVX512 1
#else
#define CP_LIBRARY_AVX512 0
#endif
_Pragma("GCC pop_options")
#elif CP_X86 && defined(__clang__)
#define CP_LIBRARY_AVX512 1
#else
#define CP_LIBRARY_AVX512 0
#endif


#line 8 "components/bitset_ops.hpp"

#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace bitset_ops_detail {
enum class Operation { identity, bit_and, bit_or, bit_xor, and_not };

template <Operation Op> inline std::uint64_t combine(std::uint64_t a, std::uint64_t b) {
	if constexpr (Op == Operation::identity) return a;
	if constexpr (Op == Operation::bit_and) return a & b;
	if constexpr (Op == Operation::bit_or) return a | b;
	if constexpr (Op == Operation::bit_xor) return a ^ b;
	if constexpr (Op == Operation::and_not) return a & ~b;
}

template <Operation Op> inline std::size_t scalar(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	std::size_t result = 0;
	for (std::size_t i = 0; i < n; ++i) {
		if constexpr (Op == Operation::identity) result += std::popcount(a[i]);
		else result += std::popcount(combine<Op>(a[i], b[i]));
	}
	return result;
}

template <Operation Op>
#if defined(__GNUC__) && !defined(__clang__)
__attribute__((optimize("O3")))
#endif
inline std::size_t scalar_large(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	return scalar<Op>(a, b, n);
}

#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#if CP_LIBRARY_AVX512
template <Operation Op> __attribute__((target("avx512f,avx512vpopcntdq")))
inline std::size_t avx512(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	__m512i sum = _mm512_setzero_si512();
	std::size_t i = 0;
	for (; n - i >= 8; i += 8) {
		__m512i x = _mm512_loadu_si512(a + i);
		if constexpr (Op != Operation::identity) {
			__m512i y = _mm512_loadu_si512(b + i);
			if constexpr (Op == Operation::bit_and) x = _mm512_and_si512(x, y);
			if constexpr (Op == Operation::bit_or) x = _mm512_or_si512(x, y);
			if constexpr (Op == Operation::bit_xor) x = _mm512_xor_si512(x, y);
			if constexpr (Op == Operation::and_not) x = _mm512_andnot_si512(y, x);
		}
		sum = _mm512_add_epi64(sum, _mm512_popcnt_epi64(x));
	}
	return std::size_t(_mm512_reduce_add_epi64(sum)) + scalar<Op>(a + i, b ? b + i : nullptr, n - i);
}
#endif

template <Operation Op> __attribute__((target("avx2")))
inline std::size_t avx2(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	const __m256i lookup = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
	const __m256i mask = _mm256_set1_epi8(15), zero = _mm256_setzero_si256();
	__m256i sum = zero;
	std::size_t i = 0;
	for (; n - i >= 4; i += 4) {
		__m256i x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		if constexpr (Op != Operation::identity) {
			__m256i y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
			if constexpr (Op == Operation::bit_and) x = _mm256_and_si256(x, y);
			if constexpr (Op == Operation::bit_or) x = _mm256_or_si256(x, y);
			if constexpr (Op == Operation::bit_xor) x = _mm256_xor_si256(x, y);
			if constexpr (Op == Operation::and_not) x = _mm256_andnot_si256(y, x);
		}
		__m256i low = _mm256_shuffle_epi8(lookup, _mm256_and_si256(x, mask));
		__m256i high = _mm256_shuffle_epi8(lookup, _mm256_and_si256(_mm256_srli_epi16(x, 4), mask));
		sum = _mm256_add_epi64(sum, _mm256_sad_epu8(_mm256_add_epi8(low, high), zero));
	}
	alignas(32) std::uint64_t lanes[4];
	_mm256_store_si256(reinterpret_cast<__m256i *>(lanes), sum);
	return std::size_t(lanes[0] + lanes[1] + lanes[2] + lanes[3]) + scalar<Op>(a + i, b ? b + i : nullptr, n - i);
}

inline bool has_avx512() {
#if CP_LIBRARY_AVX512
	static const bool enabled = __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vpopcntdq");
	return enabled;
#else
	return false;
#endif
}
inline bool has_avx2() {
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
}
#endif

template <Operation Op>
#if defined(__GNUC__) && !defined(__clang__)
__attribute__((optimize("O3")))
#endif
inline void apply_large(std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	static_assert(Op != Operation::identity);
	for (std::size_t i = 0; i < n; ++i) a[i] = combine<Op>(a[i], b[i]);
}

template <Operation Op> inline void apply(std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	static_assert(Op != Operation::identity);
	if (n >= 32) return apply_large<Op>(a, b, n);
	for (std::size_t i = 0; i < n; ++i) a[i] = combine<Op>(a[i], b[i]);
}

template <Operation Op> inline std::size_t count(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if (n >= 64) {
#if CP_LIBRARY_AVX512
		if (has_avx512()) return avx512<Op>(a, b, n);
#endif
		if (has_avx2()) return avx2<Op>(a, b, n);
	}
#endif
	if (n >= 8) return scalar_large<Op>(a, b, n);
	return scalar<Op>(a, b, n);
}
}


#line 10 "data_structure/dynamic_bitset.hpp"
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#line 17 "data_structure/dynamic_bitset.hpp"

class [[nodiscard, gnu::warn_unused]] DynamicBitset {
	using Word = std::uint64_t;
	using Operation = bitset_ops_detail::Operation;
	std::vector<Word> data_;
	int n_ = 0;
	static std::size_t word_count(int n) { return (std::size_t(n) + 63) / 64; }
	static Word mask(int n) { return n == 64 ? ~Word(0) : (Word(1) << n) - 1; }
	void trim() { if (n_ & 63) data_.back() &= mask(n_ & 63); }
	void check([[maybe_unused]] int p) const { assert(0 <= p && p < n_); }
	void check_range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
	void check_size([[maybe_unused]] const DynamicBitset &other) const { assert(n_ == other.n_); }
	Word chosen(std::size_t i, bool value) const {
		Word x = value ? data_[i] : ~data_[i];
		if (!value && i + 1 == data_.size() && (n_ & 63)) x &= mask(n_ & 63);
		return x;
	}
	Word extract(int p, int length) const {
		std::size_t w = unsigned(p) / 64;
		int offset = p & 63;
		Word x = data_[w] >> offset;
		if (offset && length > 64 - offset) x |= data_[w + 1] << (64 - offset);
		return x & mask(length);
	}
	template <class F> void update_range(int l, int r, F apply) {
		check_range(l, r);
		if (l == r) return;
		std::size_t first = unsigned(l) / 64, last = unsigned(r - 1) / 64;
		Word left = ~Word(0) << (l & 63), right = mask(((r - 1) & 63) + 1);
		if (first == last) { apply(data_[first], left & right); return; }
		apply(data_[first], left);
		for (auto i = first + 1; i < last; ++i) apply(data_[i], ~Word(0));
		apply(data_[last], right);
	}
	template <Operation Op> int count_with(const DynamicBitset &other) const {
		if constexpr (Op == Operation::identity) return int(bitset_ops_detail::count<Op>(data_.data(), nullptr, data_.size()));
		if constexpr (Op == Operation::and_not) check_size(other);
		std::size_t common = std::min(data_.size(), other.data_.size());
		int result = 0;
		if (common >= 8) result = int(bitset_ops_detail::count<Op>(data_.data(), other.data_.data(), common));
		else for (std::size_t i = 0; i < common; ++i) result += std::popcount(bitset_ops_detail::combine<Op>(data_[i], other.data_[i]));
		if constexpr (Op == Operation::bit_or || Op == Operation::bit_xor) {
			const auto &tail = data_.size() > other.data_.size() ? data_ : other.data_;
			if (common < tail.size()) result += int(bitset_ops_detail::count<Operation::identity>(tail.data() + common, nullptr, tail.size() - common));
		}
		return result;
	}
	template <Operation Op> static DynamicBitset binary(const DynamicBitset &a, const DynamicBitset &b) {
		bool first = Op == Operation::bit_and ? a.n_ <= b.n_ : a.n_ >= b.n_;
		DynamicBitset result = first ? a : b;
		const auto &other = first ? b : a;
		bitset_ops_detail::apply<Op>(result.data_.data(), other.data_.data(), std::min(result.data_.size(), other.data_.size()));
		return result;
	}
	template <bool Left, bool Or> DynamicBitset &shift(std::size_t amount) {
		if (amount >= std::size_t(n_)) { if constexpr (!Or) reset_all(); return *this; }
		if (!amount) return *this;
		std::size_t skip = amount / 64, length = data_.size();
		unsigned bits = amount & 63;
		if constexpr (Left) {
			for (std::size_t i = length; i-- > skip;) {
				Word x = data_[i - skip] << bits;
				if (bits && i > skip) x |= data_[i - skip - 1] >> (64 - bits);
				if constexpr (Or) data_[i] |= x;
				else data_[i] = x;
			}
			if constexpr (!Or) std::fill(data_.begin(), data_.begin() + skip, 0);
		} else {
			for (std::size_t i = 0; i < length - skip; ++i) {
				Word x = data_[i + skip] >> bits;
				if (bits && i + skip + 1 < length) x |= data_[i + skip + 1] << (64 - bits);
				if constexpr (Or) data_[i] |= x;
				else data_[i] = x;
			}
			if constexpr (!Or) std::fill(data_.end() - skip, data_.end(), 0);
		}
		trim();
		return *this;
	}
	static Word reverse_word(Word x) {
		x = ((x >> 1) & 0x5555555555555555ULL) | ((x & 0x5555555555555555ULL) << 1);
		x = ((x >> 2) & 0x3333333333333333ULL) | ((x & 0x3333333333333333ULL) << 2);
		x = ((x >> 4) & 0x0f0f0f0f0f0f0f0fULL) | ((x & 0x0f0f0f0f0f0f0f0fULL) << 4);
		return std::byteswap(x);
	}

  public:
	class reference {
		Word *word_;
		Word mask_;
		friend class DynamicBitset;
		reference(Word &word, int bit) : word_(&word), mask_(Word(1) << bit) {}
	  public:
		reference(const reference &) = default;
		operator bool() const { return (*word_ & mask_) != 0; }
		reference &operator=(bool value) { if (value) *word_ |= mask_; else *word_ &= ~mask_; return *this; }
		reference &operator=(const reference &other) { return *this = bool(other); }
		reference &flip() { *word_ ^= mask_; return *this; }
	};
	DynamicBitset() = default;
	explicit DynamicBitset(int n, bool value = false) : n_(n) {
		assert(n >= 0);
		data_.assign(word_count(n), value ? ~Word(0) : 0); trim();
	}
	explicit DynamicBitset(std::string_view text) {
		assert(text.size() <= std::size_t(std::numeric_limits<int>::max()));
		n_ = int(text.size()); data_.resize(word_count(n_));
		for (int i = 0; i < n_; ++i) {
			char c = text[std::size_t(n_ - 1 - i)];
			if (c != '0' && c != '1') throw std::invalid_argument("DynamicBitset");
			if (c == '1') data_[unsigned(i) / 64] |= Word(1) << (i & 63);
		}
	}
	DynamicBitset(const DynamicBitset &) = default;
	DynamicBitset &operator=(const DynamicBitset &) = default;
	DynamicBitset(DynamicBitset &&other) noexcept { swap(other); }
	DynamicBitset &operator=(DynamicBitset &&other) noexcept {
		if (this != &other) { DynamicBitset moved(std::move(other)); swap(moved); } return *this;
	}
	static DynamicBitset from_words(int n, std::span<const Word> words) {
		assert(n >= 0 && words.size() == word_count(n));
		DynamicBitset result; result.data_.assign(words.begin(), words.end()); result.n_ = n; result.trim(); return result;
	}
	void swap(DynamicBitset &other) noexcept { data_.swap(other.data_); std::swap(n_, other.n_); }
	[[nodiscard]] int size() const { return n_; }
	[[nodiscard]] bool empty() const { return !n_; }
	[[nodiscard]] std::span<const Word> words() const { return data_; }
	[[nodiscard]] bool test(int p) const { check(p); return (data_[unsigned(p) / 64] >> (p & 63)) & 1; }
	[[nodiscard]] bool operator[](int p) const { return test(p); }
	[[nodiscard]] reference operator[](int p) { check(p); return reference(data_[unsigned(p) / 64], p & 63); }
	DynamicBitset &set(int p, bool value = true) { (*this)[p] = value; return *this; }
	DynamicBitset &reset(int p) { return set(p, false); }
	DynamicBitset &flip(int p) { (*this)[p].flip(); return *this; }
	DynamicBitset &set_all(bool value = true) { std::fill(data_.begin(), data_.end(), value ? ~Word(0) : 0); trim(); return *this; }
	DynamicBitset &reset_all() { return set_all(false); }
	DynamicBitset &flip_all() { for (auto &x : data_) x = ~x; trim(); return *this; }
	DynamicBitset &set_range(int l, int r, bool value = true) {
		update_range(l, r, [value](Word &x, Word m) { if (value) x |= m; else x &= ~m; }); return *this;
	}
	DynamicBitset &reset_range(int l, int r) { return set_range(l, r, false); }
	DynamicBitset &flip_range(int l, int r) { update_range(l, r, [](Word &x, Word m) { x ^= m; }); return *this; }
	void reserve(int n) { assert(n >= 0); data_.reserve(word_count(n)); }
	void resize(int n, bool value = false) {
		assert(n >= 0);
		int old = n_;
		data_.resize(word_count(n), 0); n_ = n;
		if (n > old && value) set_range(old, n); else trim();
	}
	void push_back(bool value) { assert(n_ < std::numeric_limits<int>::max()); int p = n_; resize(n_ + 1); set(p, value); }
	void pop_back() { assert(n_); resize(n_ - 1); }
	void clear() { data_.clear(); n_ = 0; }
	[[nodiscard]] int count() const { return count_with<Operation::identity>(*this); }
	[[nodiscard]] int count(int l, int r) const {
		check_range(l, r); if (l == r) return 0;
		std::size_t first = unsigned(l) / 64, last = unsigned(r - 1) / 64;
		Word left = ~Word(0) << (l & 63), right = mask(((r - 1) & 63) + 1);
		if (first == last) return std::popcount(data_[first] & left & right);
		int result = std::popcount(data_[first] & left) + std::popcount(data_[last] & right);
		result += int(bitset_ops_detail::count<Operation::identity>(data_.data() + first + 1, nullptr, last - first - 1));
		return result;
	}
	[[nodiscard]] int rank(int r) const { return count(0, r); }
	[[nodiscard]] bool any() const { for (auto x : data_) if (x) return true; return false; }
	[[nodiscard]] bool none() const { return !any(); }
	[[nodiscard]] bool all() const {
		std::size_t full = unsigned(n_) / 64;
		for (std::size_t i = 0; i < full; ++i) if (data_[i] != ~Word(0)) return false;
		return !(n_ & 63) || data_[full] == mask(n_ & 63);
	}
	[[nodiscard]] int count_and(const DynamicBitset &other) const { return count_with<Operation::bit_and>(other); }
	[[nodiscard]] int count_or(const DynamicBitset &other) const { return count_with<Operation::bit_or>(other); }
	[[nodiscard]] int count_xor(const DynamicBitset &other) const { return count_with<Operation::bit_xor>(other); }
	[[nodiscard]] int count_and_not(const DynamicBitset &other) const { return count_with<Operation::and_not>(other); }
	[[nodiscard]] bool intersection_parity(const DynamicBitset &other) const {
		check_size(other); Word x = 0;
		for (std::size_t i = 0; i < data_.size(); ++i) x ^= data_[i] & other.data_[i];
		return std::popcount(x) & 1;
	}
	[[nodiscard]] bool intersects(const DynamicBitset &other) const { check_size(other); for (std::size_t i = 0; i < data_.size(); ++i) if (data_[i] & other.data_[i]) return true; return false; }
	[[nodiscard]] bool is_subset_of(const DynamicBitset &other) const { check_size(other); for (std::size_t i = 0; i < data_.size(); ++i) if (data_[i] & ~other.data_[i]) return false; return true; }
	[[nodiscard]] bool is_proper_subset_of(const DynamicBitset &other) const { return is_subset_of(other) && *this != other; }
	[[nodiscard]] int compare(const DynamicBitset &other) const {
		check_size(other); for (auto i = data_.size(); i-- > 0;) if (data_[i] != other.data_[i]) return data_[i] < other.data_[i] ? -1 : 1; return 0;
	}
	[[nodiscard]] int find_next(int p, bool value = true) const {
		assert(-1 <= p && p < n_); ++p;
		if (p == n_) return n_;
		std::size_t i = unsigned(p) / 64;
		Word x = chosen(i, value) & (~Word(0) << (p & 63));
		for (;;) {
			if (x) return int(i * 64 + std::countr_zero(x));
			if (++i == data_.size()) return n_;
			x = chosen(i, value);
		}
	}
	[[nodiscard]] int find_prev(int p, bool value = true) const {
		assert(0 <= p && p <= n_); if (!p) return n_; --p;
		std::size_t i = unsigned(p) / 64;
		Word x = chosen(i, value) & mask((p & 63) + 1);
		for (;;) {
			if (x) return int(i * 64 + 63 - std::countl_zero(x));
			if (!i) return n_;
			x = chosen(--i, value);
		}
	}
	[[nodiscard]] int find_first(bool value = true) const { return n_ ? find_next(-1, value) : 0; }
	[[nodiscard]] int find_last(bool value = true) const { return find_prev(n_, value); }
	[[nodiscard]] int select(int k, bool value = true) const {
		assert(k >= 0);
		for (std::size_t i = 0; i < data_.size(); ++i) {
			Word x = chosen(i, value); int c = std::popcount(x);
			if (k < c) { while (k--) x &= x - 1; return int(i * 64 + std::countr_zero(x)); }
			k -= c;
		}
		return n_;
	}
	template <class F> void for_each(int l, int r, F &&emit) const {
		check_range(l, r); if (l == r) return;
		for (std::size_t i = unsigned(l) / 64, last = unsigned(r - 1) / 64; i <= last; ++i) {
			Word x = data_[i];
			if (i == unsigned(l) / 64) x &= ~Word(0) << (l & 63);
			if (i == last) x &= mask(((r - 1) & 63) + 1);
			while (x) { emit(int(i * 64 + std::countr_zero(x))); x &= x - 1; }
		}
	}
	template <class F> void for_each(F &&emit) const { for_each(0, n_, std::forward<F>(emit)); }
	DynamicBitset &operator&=(const DynamicBitset &other) {
		if (n_ > other.n_) resize(other.n_);
		bitset_ops_detail::apply<Operation::bit_and>(data_.data(), other.data_.data(), data_.size()); return *this;
	}
	DynamicBitset &operator|=(const DynamicBitset &other) {
		if (n_ < other.n_) resize(other.n_);
		bitset_ops_detail::apply<Operation::bit_or>(data_.data(), other.data_.data(), other.data_.size()); return *this;
	}
	DynamicBitset &operator^=(const DynamicBitset &other) {
		if (n_ < other.n_) resize(other.n_);
		bitset_ops_detail::apply<Operation::bit_xor>(data_.data(), other.data_.data(), other.data_.size()); return *this;
	}
	DynamicBitset &operator-=(const DynamicBitset &other) { check_size(other); bitset_ops_detail::apply<Operation::and_not>(data_.data(), other.data_.data(), data_.size()); return *this; }
	friend DynamicBitset operator&(const DynamicBitset &a, const DynamicBitset &b) { return binary<Operation::bit_and>(a, b); }
	friend DynamicBitset operator|(const DynamicBitset &a, const DynamicBitset &b) { return binary<Operation::bit_or>(a, b); }
	friend DynamicBitset operator^(const DynamicBitset &a, const DynamicBitset &b) { return binary<Operation::bit_xor>(a, b); }
	friend DynamicBitset operator-(DynamicBitset a, const DynamicBitset &b) { a -= b; return a; }
	DynamicBitset operator~() const { auto result = *this; result.flip_all(); return result; }
	[[nodiscard]] bool operator==(const DynamicBitset &) const = default;
	DynamicBitset &operator<<=(std::size_t k) { return shift<true, false>(k); }
	DynamicBitset &operator>>=(std::size_t k) { return shift<false, false>(k); }
	friend DynamicBitset operator<<(DynamicBitset a, std::size_t k) { a <<= k; return a; }
	friend DynamicBitset operator>>(DynamicBitset a, std::size_t k) { a >>= k; return a; }
	DynamicBitset &or_shift_left(std::size_t k) { return shift<true, true>(k); }
	DynamicBitset &or_shift_right(std::size_t k) { return shift<false, true>(k); }
	void copy_range(int dst, const DynamicBitset &source, int l, int r) {
		source.check_range(l, r); assert(0 <= dst && r - l <= n_ - dst);
		int length = r - l;
		auto copy = [&](int offset, int count) {
			Word x = source.extract(l + offset, count);
			int p = dst + offset; Word m = mask(count) << (p & 63);
			auto &target = data_[unsigned(p) / 64]; target = (target & ~m) | (x << (p & 63));
		};
		if (this == &source && dst > l) {
			while (length) { int count = std::min(length, ((dst + length - 1) & 63) + 1); length -= count; copy(length, count); }
		} else {
			for (int offset = 0; offset < length;) { int count = std::min(length - offset, 64 - ((dst + offset) & 63)); copy(offset, count); offset += count; }
		}
	}
	DynamicBitset slice(int l, int r) const { check_range(l, r); DynamicBitset result(r - l); result.copy_range(0, *this, l, r); return result; }
	void append(const DynamicBitset &other) {
		int length = other.n_, old = n_; assert(length <= std::numeric_limits<int>::max() - old);
		resize(old + length); copy_range(old, other, 0, length);
	}
	DynamicBitset &reverse() {
		std::reverse(data_.begin(), data_.end()); for (auto &x : data_) x = reverse_word(x);
		if (n_ & 63) {
			unsigned bits = 64 - (n_ & 63);
			for (std::size_t i = 0; i < data_.size(); ++i)
				data_[i] = (data_[i] >> bits) | (i + 1 < data_.size() ? data_[i + 1] << (64 - bits) : 0);
		}
		return *this;
	}
	DynamicBitset &rotate_left(std::size_t amount) {
		if (!n_ || !(amount %= std::size_t(n_))) return *this;
		auto wrap = *this; wrap >>= std::size_t(n_) - amount; *this <<= amount; return *this |= wrap;
	}
	DynamicBitset &rotate_right(std::size_t amount) { if (n_ && (amount %= std::size_t(n_))) rotate_left(std::size_t(n_) - amount); return *this; }
	[[nodiscard]] std::string to_string() const {
		std::string result(n_, '0'); for_each([&](int p) { result[std::size_t(n_ - 1 - p)] = '1'; }); return result;
	}
};


#line 6 "math/matrix_mod2.hpp"
#include <cstring>
#include <initializer_list>
#line 9 "math/matrix_mod2.hpp"

struct LinearSolutionMod2 {
	DynamicBitset particular;
	std::vector<DynamicBitset> basis;
};

class MatrixMod2 {
	using Word = std::uint64_t;
	int h_ = 0, w_ = 0, stride_ = 0;
	std::vector<Word> data_;
	static int word_count(int n) { return int((std::size_t(n) + 63) / 64); }
	static std::size_t allocation(int h, int stride) {
		if (stride && std::size_t(h) > std::vector<Word>().max_size() / std::size_t(stride))
			throw std::length_error("MatrixMod2");
		return std::size_t(h) * stride;
	}
	void check_row([[maybe_unused]] int i) const { assert(0 <= i && i < h_); }
	void check([[maybe_unused]] int i, [[maybe_unused]] int j) const { check_row(i); assert(0 <= j && j < w_); }
	Word* raw_row(int i) { return stride_ ? data_.data() + std::size_t(i) * stride_ : data_.data(); }
	const Word* raw_row(int i) const { return stride_ ? data_.data() + std::size_t(i) * stride_ : data_.data(); }
	void trim() {
		if (w_ & 63) for (int i = 0; i < h_; ++i) raw_row(i)[stride_ - 1] &= (Word(1) << (w_ & 63)) - 1;
	}
	std::vector<int> reduce(int columns, bool reduced) {
		return matrix_mod2_detail::eliminate(data_, h_, stride_, columns, reduced);
	}
	static std::vector<DynamicBitset> nullspace(const MatrixMod2& r, const std::vector<int>& pivots, int vars) {
		std::vector<unsigned char> is_pivot(vars);
		for (int p : pivots) is_pivot[p] = 1;
		std::vector<DynamicBitset> result;
		result.reserve(std::size_t(vars) - pivots.size());
		for (int free = 0; free < vars; ++free) if (!is_pivot[free]) {
			DynamicBitset v(vars);
			v.set(free);
			for (int i = 0; i < int(pivots.size()); ++i) if (r(i, free)) v.set(pivots[i]);
			result.push_back(std::move(v));
		}
		return result;
	}
	std::pair<MatrixMod2, std::vector<int>> echelon(bool reduced) const {
		MatrixMod2 result = *this;
		auto pivots = result.reduce(w_, reduced);
		return {std::move(result), std::move(pivots)};
	}

  public:
	using value_type = bool;
	using word_type = Word;
	using Solution = LinearSolutionMod2;
	class reference {
		Word* word_;
		Word mask_;
		friend class MatrixMod2;
		reference(Word& word, int bit) : word_(&word), mask_(Word(1) << bit) {}
	  public:
		reference(const reference&) = default;
		operator bool() const { return (*word_ & mask_) != 0; }
		reference& operator=(bool value) { if (value) *word_ |= mask_; else *word_ &= ~mask_; return *this; }
		reference& operator=(const reference& other) { return *this = bool(other); }
		reference& flip() { *word_ ^= mask_; return *this; }
		reference& operator^=(bool value) { if (value) flip(); return *this; }
	};
	class Row {
		MatrixMod2* matrix_;
		int row_;
		friend class MatrixMod2;
		Row(MatrixMod2& matrix, int row) : matrix_(&matrix), row_(row) {}
	  public:
		reference operator[](int j) const { return (*matrix_)(row_, j); }
		int size() const { return matrix_->cols(); }
	};
	class ConstRow {
		const MatrixMod2* matrix_;
		int row_;
		friend class MatrixMod2;
		ConstRow(const MatrixMod2& matrix, int row) : matrix_(&matrix), row_(row) {}
	  public:
		bool operator[](int j) const { return (*matrix_)(row_, j); }
		int size() const { return matrix_->cols(); }
	};
	MatrixMod2() = default;
	explicit MatrixMod2(int n) : MatrixMod2(n, n) {}
	MatrixMod2(int rows, int columns, bool value = false) : h_(rows), w_(columns) {
		assert(rows >= 0 && columns >= 0);
		stride_ = word_count(columns);
		data_.assign(allocation(rows, stride_), value ? ~Word(0) : Word(0));
		trim();
	}
	MatrixMod2(std::initializer_list<std::initializer_list<int>> values)
		: MatrixMod2(int(values.size()), values.size() ? int(values.begin()->size()) : 0) {
		int i = 0;
		for (const auto& row : values) {
			assert(int(row.size()) == w_);
			int j = 0;
			for (int value : row) { assert(value == 0 || value == 1); (*this)(i, j++) = bool(value); }
			++i;
		}
	}
	explicit MatrixMod2(const std::vector<std::vector<int>>& values)
		: MatrixMod2(int(values.size()), values.empty() ? 0 : int(values[0].size())) {
		for (int i = 0; i < h_; ++i) {
			assert(int(values[i].size()) == w_);
			for (int j = 0; j < w_; ++j) { assert(values[i][j] == 0 || values[i][j] == 1); (*this)(i, j) = bool(values[i][j]); }
		}
	}
	explicit MatrixMod2(const std::vector<std::string>& values)
		: MatrixMod2(int(values.size()), values.empty() ? 0 : int(values[0].size())) {
		for (int i = 0; i < h_; ++i) set_row(i, values[i]);
	}
	explicit MatrixMod2(const std::vector<DynamicBitset>& values)
		: MatrixMod2(int(values.size()), values.empty() ? 0 : values[0].size()) {
		for (int i = 0; i < h_; ++i) set_row(i, values[i]);
	}
	MatrixMod2(const MatrixMod2&) = default;
	MatrixMod2& operator=(const MatrixMod2&) = default;
	MatrixMod2(MatrixMod2&& other) noexcept { swap(other); }
	MatrixMod2& operator=(MatrixMod2&& other) noexcept {
		if (this != &other) { MatrixMod2 next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(MatrixMod2& other) noexcept {
		std::swap(h_, other.h_), std::swap(w_, other.w_), std::swap(stride_, other.stride_);
		data_.swap(other.data_);
	}
	int rows() const { return h_; }
	int cols() const { return w_; }
	int words_per_row() const { return stride_; }
	bool empty() const { return !h_ || !w_; }
	bool operator()(int i, int j) const { check(i, j); return (raw_row(i)[j / 64] >> (j & 63)) & 1; }
	reference operator()(int i, int j) { check(i, j); return reference(raw_row(i)[j / 64], j & 63); }
	ConstRow operator[](int i) const { check_row(i); return ConstRow(*this, i); }
	Row operator[](int i) { check_row(i); return Row(*this, i); }
	void set(int i, int j, bool value = true) { (*this)(i, j) = value; }
	void flip(int i, int j) { (*this)(i, j).flip(); }
	void fill(bool value) { std::fill(data_.begin(), data_.end(), value ? ~Word(0) : Word(0)); trim(); }
	std::span<const Word> row_words(int i) const { check_row(i); return {raw_row(i), std::size_t(stride_)}; }
	static MatrixMod2 from_words(int rows, int columns, std::span<const Word> words) {
		MatrixMod2 result(rows, columns);
		assert(words.size() == result.data_.size());
		std::copy(words.begin(), words.end(), result.data_.begin());
		result.trim();
		return result;
	}
	DynamicBitset row(int i) const { return DynamicBitset::from_words(w_, row_words(i)); }
	void set_row(int i, const DynamicBitset& bits) {
		check_row(i);
		assert(bits.size() == w_);
		std::copy(bits.words().begin(), bits.words().end(), raw_row(i));
	}
	void set_row(int i, std::string_view text) {
		check_row(i);
		assert(text.size() == std::size_t(w_));
		if (stride_) std::fill_n(raw_row(i), stride_, Word(0));
		int j = 0;
		if constexpr ('0' == 0x30 && '1' == 0x31 && (std::endian::native == std::endian::little || std::endian::native == std::endian::big)) {
			for (; w_ - j >= 8; j += 8) {
				Word bytes;
				std::memcpy(&bytes, text.data() + j, 8);
				if constexpr (std::endian::native == std::endian::big) bytes = std::byteswap(bytes);
				if ((bytes & 0xfefefefefefefefeULL) != 0x3030303030303030ULL) throw std::invalid_argument("MatrixMod2");
				Word bits = ((bytes & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56;
				raw_row(i)[j / 64] |= bits << (j & 63);
			}
		}
		for (; j < w_; ++j) {
			if (text[j] != '0' && text[j] != '1') throw std::invalid_argument("MatrixMod2");
			if (text[j] == '1') raw_row(i)[j / 64] |= Word(1) << (j & 63);
		}
	}
	std::string row_string(int i) const {
		check_row(i);
		std::string result(w_, '0');
		int j = 0;
		if constexpr ('0' == 0x30 && '1' == 0x31 && (std::endian::native == std::endian::little || std::endian::native == std::endian::big)) {
			for (; w_ - j >= 8; j += 8) {
				Word bits = (raw_row(i)[j / 64] >> (j & 63)) & 255;
				bits = (bits | (bits << 28)) & 0x0000000f0000000fULL;
				bits = (bits | (bits << 14)) & 0x0003000300030003ULL;
				bits = (bits | (bits << 7)) & 0x0101010101010101ULL;
				bits += 0x3030303030303030ULL;
				if constexpr (std::endian::native == std::endian::big) bits = std::byteswap(bits);
				std::memcpy(result.data() + j, &bits, 8);
			}
		}
		for (; j < w_; ++j) if ((*this)(i, j)) result[j] = '1';
		return result;
	}
	bool operator==(const MatrixMod2&) const = default;
	static MatrixMod2 identity(int n) {
		MatrixMod2 result(n);
		for (int i = 0; i < n; ++i) result(i, i) = true;
		return result;
	}
	MatrixMod2 transpose() const {
		allocation(w_, word_count(h_));
		MatrixMod2 result;
		result.h_ = w_, result.w_ = h_, result.stride_ = word_count(h_);
		result.data_ = matrix_mod2_detail::transpose(data_, h_, w_);
		return result;
	}
	MatrixMod2 submatrix(int row_l, int row_r, int col_l, int col_r) const {
		assert(0 <= row_l && row_l <= row_r && row_r <= h_ && 0 <= col_l && col_l <= col_r && col_r <= w_);
		MatrixMod2 result(row_r - row_l, col_r - col_l);
		int first = col_l / 64, shift = col_l & 63;
		for (int i = 0; i < result.h_; ++i) for (int j = 0; j < result.stride_; ++j) {
			Word value = raw_row(row_l + i)[first + j] >> shift;
			if (shift && first + j + 1 < stride_) value |= raw_row(row_l + i)[first + j + 1] << (64 - shift);
			result.raw_row(i)[j] = value;
		}
		result.trim();
		return result;
	}
	MatrixMod2 operator-() const { return *this; }
	MatrixMod2& operator+=(const MatrixMod2& other) {
		assert(h_ == other.h_ && w_ == other.w_);
		for (std::size_t i = 0; i < data_.size(); ++i) data_[i] ^= other.data_[i];
		return *this;
	}
	MatrixMod2& operator-=(const MatrixMod2& other) { return *this += other; }
	MatrixMod2& operator*=(const MatrixMod2& other) { return *this = *this * other; }
	MatrixMod2& operator*=(bool value) { if (!value) fill(false); return *this; }
	friend MatrixMod2 operator+(MatrixMod2 a, const MatrixMod2& b) { return a += b; }
	friend MatrixMod2 operator-(MatrixMod2 a, const MatrixMod2& b) { return a -= b; }
	friend MatrixMod2 operator*(const MatrixMod2& a, const MatrixMod2& b) {
		assert(a.w_ == b.h_);
		allocation(a.h_, b.stride_);
		MatrixMod2 result;
		result.h_ = a.h_, result.w_ = b.w_, result.stride_ = b.stride_;
		result.data_ = matrix_mod2_detail::multiply(a.data_, b.data_, a.h_, a.w_, b.w_);
		return result;
	}
	friend MatrixMod2 operator*(MatrixMod2 a, bool b) { return a *= b; }
	friend MatrixMod2 operator*(bool a, MatrixMod2 b) { return b *= a; }
	DynamicBitset operator*(const DynamicBitset& v) const {
		assert(v.size() == w_);
		DynamicBitset result(h_);
		for (int i = 0; i < h_; ++i) {
			Word value = 0;
			for (int j = 0; j < stride_; ++j) value ^= raw_row(i)[j] & v.words()[j];
			if (std::popcount(value) & 1) result.set(i);
		}
		return result;
	}
	MatrixMod2 pow(std::uint64_t exponent) const {
		assert(h_ == w_);
		MatrixMod2 result = identity(h_);
		if (!exponent) return result;
		MatrixMod2 base = *this;
		while (exponent) {
			if (exponent & 1) result *= base;
			exponent >>= 1;
			if (exponent) base *= base;
		}
		return result;
	}
	bool trace() const {
		assert(h_ == w_);
		bool result = false;
		for (int i = 0; i < h_; ++i) result ^= (*this)(i, i);
		return result;
	}
	int rank() const {
		if (!h_ || !w_) return 0;
		if (w_ <= 64) {
			Word basis[64]{};
			int result = 0;
			for (int i = 0; i < h_; ++i) {
				Word value = raw_row(i)[0];
				while (value) {
					int pivot = std::countr_zero(value);
					if (basis[pivot]) value ^= basis[pivot];
					else { basis[pivot] = value; ++result; break; }
				}
				if (result == w_) return result;
			}
			return result;
		}
		MatrixMod2 work = *this;
		return int(work.reduce(w_, false).size());
	}
	bool det() const { assert(h_ == w_); return rank() == h_; }
	std::pair<MatrixMod2, std::vector<int>> row_echelon() const { return echelon(false); }
	std::pair<MatrixMod2, std::vector<int>> rref() const { return echelon(true); }
	std::optional<MatrixMod2> inverse() const {
		assert(h_ == w_ && w_ <= std::numeric_limits<int>::max() / 2);
		MatrixMod2 work(h_, w_ * 2);
		for (int i = 0; i < h_; ++i) {
			if (stride_) std::copy_n(raw_row(i), stride_, work.raw_row(i));
			work(i, w_ + i) = true;
		}
		if (int(work.reduce(w_, true).size()) != h_) return std::nullopt;
		return work.submatrix(0, h_, w_, w_ * 2);
	}
	std::vector<DynamicBitset> kernel() const {
		auto [work, pivots] = rref();
		return nullspace(work, pivots, w_);
	}
	std::optional<Solution> solve(const DynamicBitset& b) const {
		assert(b.size() == h_ && w_ < std::numeric_limits<int>::max());
		MatrixMod2 work(h_, w_ + 1);
		for (int i = 0; i < h_; ++i) {
			if (stride_) std::copy_n(raw_row(i), stride_, work.raw_row(i));
			if (b[i]) work(i, w_) = true;
		}
		auto pivots = work.reduce(w_, true);
		for (int i = int(pivots.size()); i < h_; ++i) if (work(i, w_)) return std::nullopt;
		Solution result{DynamicBitset(w_), nullspace(work, pivots, w_)};
		for (int i = 0; i < int(pivots.size()); ++i) if (work(i, w_)) result.particular.set(pivots[i]);
		return result;
	}
};


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 13 "components/fast_io.hpp"
#include <memory>
#line 17 "components/fast_io.hpp"
#include <type_traits>
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


#line 6 "test/fixtures/matrix_mod2_io.hpp"

inline MatrixMod2 read_matrix_mod2(FastInput& input, int n, int m) {
	MatrixMod2 matrix(n, m);
	for (int i = 0; i < n; ++i) {
		std::string row;
		if (m) input.read(row);
		matrix.set_row(i, row);
	}
	return matrix;
}

inline void write_matrix_mod2(FastOutput& output, const MatrixMod2& matrix) {
	for (int i = 0; i < matrix.rows(); ++i) output.println(matrix.row_string(i));
}

inline DynamicBitset read_vector_mod2(FastInput& input, int n) {
	std::string text;
	if (n) input.read(text);
	assert(int(text.size()) == n);
	DynamicBitset result(n);
	for (int i = 0; i < n; ++i) result[i] = text[i] == '1';
	return result;
}

inline void write_vector_mod2(FastOutput& output, const DynamicBitset& vector) {
	std::string text(vector.size(), '0');
	for (int i = 0; i < vector.size(); ++i) text[i] += int(vector[i]);
	output.println(text);
}


#line 3 "test/matrix_mod2_det.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	output.println(int(read_matrix_mod2(input, n, n).det()));
}
