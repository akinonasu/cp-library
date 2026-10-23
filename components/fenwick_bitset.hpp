#ifndef FENWICK_BITSET_HPP
#define FENWICK_BITSET_HPP

#include "fenwick_sum.hpp"
#include <bit>
#include <cassert>
#include <cstdint>
#include <vector>

class FenwickBitset {
	std::vector<std::uint64_t> words;
	FenwickSum<int> counts;
	int n = 0;
  public:
	explicit FenwickBitset(int size = 0) : counts(size > 0 ? (std::size_t(size) + 63) / 64 : 0), n(size) {
		assert(size >= 0);
		words.resize((std::size_t(size) + 63) / 64 + 1);
	}
	int size() const { return n; }
	bool contains(int p) const {
		assert(0 <= p && p < n);
		return (words[p >> 6] >> (p & 63)) & 1;
	}
	void set(int p, bool value) {
		assert(0 <= p && p < n);
		std::uint64_t mask = std::uint64_t(1) << (p & 63);
		if (bool(words[p >> 6] & mask) == value) return;
		words[p >> 6] ^= mask;
		counts.add(p >> 6, value ? 1 : -1);
	}
	int count(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		return counts.prod(l >> 6, r >> 6)
			+ std::popcount(words[r >> 6] & ((std::uint64_t(1) << (r & 63)) - 1))
			- std::popcount(words[l >> 6] & ((std::uint64_t(1) << (l & 63)) - 1));
	}
};

#endif
