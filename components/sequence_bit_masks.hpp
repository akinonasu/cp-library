#ifndef SEQUENCE_BIT_MASKS_HPP
#define SEQUENCE_BIT_MASKS_HPP

#include "sequence_compare.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace sequence_bit_masks_detail {
using Symbol = std::uint32_t;
using Word = std::uint64_t;

template <class T>
concept IntegralSymbol = std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8;

template <class S, class T>
concept CompatibleSymbols = IntegralSymbol<std::remove_const_t<S>> && std::is_same_v<std::remove_const_t<S>, std::remove_const_t<T>> && !std::is_volatile_v<S> && !std::is_volatile_v<T>;

template <IntegralSymbol T>
struct CompressedPair {
	std::vector<Symbol> first, second;
	std::vector<T> symbols;

	CompressedPair(std::span<const T> s, std::span<const T> t) : first(s.size()), second(t.size()) {
		std::size_t size = s.size() + t.size();
		if (!size) return;
		std::vector<std::size_t> order(size), buffer(size);
		for (std::size_t i = 0; i < size; ++i) order[i] = i;
		auto value = [&](std::size_t i) { return i < s.size() ? s[i] : t[i - s.size()]; };
		for (unsigned shift = 0; shift < 8 * sizeof(T); shift += 8) {
			std::array<std::size_t, 256> offset{};
			auto digit = [&](std::size_t i) { return unsigned((static_cast<std::make_unsigned_t<T>>(value(i)) >> shift) & 255); };
			for (std::size_t i : order) ++offset[digit(i)];
			std::size_t sum = 0;
			for (auto &x : offset) {
				std::size_t count = x;
				x = sum;
				sum += count;
			}
			for (std::size_t i : order) buffer[offset[digit(i)]++] = i;
			order.swap(buffer);
		}
		for (std::size_t i : order) {
			T x = value(i);
			if (symbols.empty() || symbols.back() != x) symbols.push_back(x);
			Symbol rank = Symbol(symbols.size() - 1);
			if (i < s.size()) first[i] = rank;
			else second[i - s.size()] = rank;
		}
	}
};

struct MaskWorkspace {
	std::vector<int> head, tail;
	explicit MaskWorkspace(std::size_t alphabet) : head(alphabet, -1), tail(alphabet, -1) {}
};

struct SparseMasks {
	struct Block {
		Word bits;
		int word, next;
	};
	MaskWorkspace &workspace;
	std::vector<Block> blocks;
	std::vector<Symbol> used;

	SparseMasks(std::span<const Symbol> sequence, MaskWorkspace &scratch, bool reverse = false) : workspace(scratch) {
		blocks.reserve(sequence.size());
		for (int j = 0, n = int(sequence.size()); j < n; ++j) {
			Symbol c = sequence[reverse ? n - 1 - j : j];
			int &head = workspace.head[c], &tail = workspace.tail[c];
			if (tail != -1 && blocks[tail].word == j / 64) blocks[tail].bits |= Word(1) << (j % 64);
			else {
				int index = int(blocks.size());
				if (tail == -1) head = index, used.push_back(c);
				else blocks[tail].next = index;
				tail = index;
				blocks.push_back({Word(1) << (j % 64), j / 64, -1});
			}
		}
	}
	SparseMasks(const SparseMasks &) = delete;
	SparseMasks &operator=(const SparseMasks &) = delete;
	~SparseMasks() {
		for (Symbol c : used) workspace.head[c] = workspace.tail[c] = -1;
	}
	int first(Symbol c) const { return workspace.head[c]; }
	Word next(int &index, int word) const {
		if (index == -1 || blocks[index].word != word) return 0;
		Word result = blocks[index].bits;
		index = blocks[index].next;
		return result;
	}
};

template <class T>
inline bool subsequence(std::span<const T> s, std::span<const T> t) {
	if (s.empty()) return true;
	std::size_t i = 0;
	for (T c : t)
		if (s[i] == c && ++i == s.size()) return true;
	return false;
}

template <class T>
inline std::pair<std::span<const T>, std::span<const T>> trim(std::span<const T> &s, std::span<const T> &t) {
	int limit = int(std::min(s.size(), t.size()));
	std::size_t prefix = sequence_compare_detail::prefix(s.data(), t.data(), limit);
	auto head = s.first(prefix);
	s = s.subspan(prefix);
	t = t.subspan(prefix);
	limit -= int(prefix);
	std::size_t suffix = limit ? sequence_compare_detail::suffix(s.data() + s.size(), t.data() + t.size(), limit) : 0;
	auto tail = s.last(suffix);
	s = s.first(s.size() - suffix);
	t = t.first(t.size() - suffix);
	return {head, tail};
}
}

#endif
