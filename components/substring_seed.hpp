#ifndef SUBSTRING_SEED_HPP
#define SUBSTRING_SEED_HPP

#include "sequence_compare.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace substring_seed_detail {
struct Match { int first, second, length; };

inline std::optional<Match> search(std::string_view first, std::string_view second, Match best) {
	bool swapped = first.size() > second.size();
	if (swapped) {
		std::swap(first, second);
		std::swap(best.first, best.second);
	}
	if (first.size() < 256 || first.size() > (std::size_t(1) << 21)) return std::nullopt;
	int n = static_cast<int>(first.size()), m = static_cast<int>(second.size());
	std::array<unsigned char, 256> present{}, codes{};
	for (unsigned char c : first) present[c] |= 1;
	for (unsigned char c : second) present[c] |= 2;
	unsigned alphabet = 0, first_symbols = 0, second_symbols = 0, uniform = 0;
	bool common = false;
	for (int c = 0; c < 256; ++c) {
		if (present[c]) codes[c] = static_cast<unsigned char>(alphabet++);
		first_symbols += bool(present[c] & 1), second_symbols += bool(present[c] & 2);
		if (present[c] == 3) common = true, uniform = c;
	}
	if (!common) return Match{0, 0, 0};
	if (first_symbols == 1 || second_symbols == 1) {
		bool uniform_first = first_symbols == 1;
		auto text = uniform_first ? second : first;
		int limit = uniform_first ? n : m, run = 0;
		best = {0, 0, 0};
		for (int i = 0; i < int(text.size()); ++i) {
			run = static_cast<unsigned char>(text[i]) == uniform ? run + 1 : 0;
			if (run > best.length) best = uniform_first ? Match{0, i + 1 - run, run} : Match{i + 1 - run, 0, run};
			if (run == limit) break;
		}
		if (swapped) std::swap(best.first, best.second);
		return best;
	}
	unsigned bits = std::bit_width(alphabet - 1), length = 1;
	std::uint64_t domain = alphabet;
	while (domain < 8 * std::uint64_t(n) && (length + 1) * bits <= 32) ++length, domain *= alphabet;
	unsigned key_bits = length * bits;
	std::uint32_t mask = key_bits == 32 ? ~std::uint32_t(0) : (std::uint32_t(1) << key_bits) - 1;
	auto key_at = [&](const char *p) {
		std::uint32_t key = 0;
		for (unsigned k = 0; k < length; ++k) key = (key << bits) | codes[static_cast<unsigned char>(p[k])];
		return key;
	};
	int positions = n - static_cast<int>(length) + 1;
	std::array<std::uint32_t, 128> sample;
	for (std::size_t i = 0; i < sample.size(); ++i)
		sample[i] = key_at(first.data() + i * (positions - 1) / (sample.size() - 1));
	std::sort(sample.begin(), sample.end());
	int duplicates = 0;
	for (std::size_t i = 1; i < sample.size(); ++i) duplicates += sample[i] == sample[i - 1];
	if (duplicates > 16) return std::nullopt;
	std::size_t capacity = std::bit_ceil(2 * std::size_t(positions)), table_mask = capacity - 1;
	unsigned shift = 32 - std::countr_zero(capacity);
	std::vector<std::uint64_t> table(capacity);
	std::vector<std::uint32_t> next(positions);
	std::uint64_t probes = 0, probe_budget = 8 * (std::uint64_t(n) + m);
	auto find = [&](std::uint32_t key) -> std::size_t {
		std::size_t slot = (key * std::uint32_t(0x9e3779b9)) >> shift;
		unsigned chain = 0;
		while (table[slot] && static_cast<std::uint32_t>(table[slot] >> 32) != key) {
			if (++probes > probe_budget || ++chain > 64) return capacity;
			slot = (slot + 1) & table_mask;
		}
		return slot;
	};
	std::uint32_t key = key_at(first.data());
	for (int i = 0; i < positions; ++i) {
		std::size_t slot = find(key);
		if (slot == capacity) return std::nullopt;
		next[i] = static_cast<std::uint32_t>(table[slot]);
		table[slot] = (std::uint64_t(key) << 32) | unsigned(i + 1);
		if (i + 1 < positions) key = ((key << bits) | codes[static_cast<unsigned char>(first[i + length])]) & mask;
	}
	std::uint64_t work = 0, budget = 4 * (std::uint64_t(n) + m);
	for (int j = 0; j + length <= unsigned(m);) {
		std::size_t slot = find(key_at(second.data() + j));
		if (slot == capacity) return std::nullopt;
		auto position = static_cast<std::uint32_t>(table[slot]);
		while (position) {
			int i = static_cast<int>(position) - 1;
			if (++work > budget) return std::nullopt;
			int left_limit = std::min(i, j), right_limit = std::min(n - i, m - j);
			if (left_limit + right_limit > best.length) {
				int right = length + sequence_compare_detail::prefix(first.data() + i + length, second.data() + j + length, right_limit - length);
				int left = sequence_compare_detail::suffix(first.data() + i, second.data() + j, left_limit);
				work += right + left;
				if (right + left > best.length) best = {i - left, j - left, right + left};
				if (best.length == n) {
					if (swapped) std::swap(best.first, best.second);
					return best;
				}
				if (work > budget) return std::nullopt;
			}
			position = next[i];
		}
		j += best.length >= int(length) ? best.length - length + 2 : 1;
	}
	if (best.length + 1 < int(length)) return std::nullopt;
	if (swapped) std::swap(best.first, best.second);
	return best;
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

#endif
