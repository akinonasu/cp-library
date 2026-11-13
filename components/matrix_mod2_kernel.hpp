#ifndef MATRIX_MOD2_KERNEL_HPP
#define MATRIX_MOD2_KERNEL_HPP

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

#endif
