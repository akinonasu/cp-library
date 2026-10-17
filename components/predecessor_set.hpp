#ifndef PREDECESSOR_SET_HPP
#define PREDECESSOR_SET_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

class PredecessorSet {
	int n = 0, count = 0;
	std::vector<std::vector<std::uint64_t>> data;
public:
	explicit PredecessorSet(int n = 0, bool filled = false) { assign(n, filled); }
	PredecessorSet(const PredecessorSet &) = default;
	PredecessorSet &operator=(const PredecessorSet &) = default;
	PredecessorSet(PredecessorSet &&other) noexcept : n(std::exchange(other.n, 0)), count(std::exchange(other.count, 0)), data(std::move(other.data)) {}
	PredecessorSet &operator=(PredecessorSet &&other) noexcept {
		if (this != &other) { n = std::exchange(other.n, 0); count = std::exchange(other.count, 0); data = std::move(other.data); }
		return *this;
	}
	void assign(int length, bool filled = false) {
		assert(length >= 0);
		n = length; count = filled ? n : 0;
		data.clear();
		for (std::size_t bits = n; bits;) {
			std::size_t words = (bits + 63) / 64;
			data.emplace_back(words, filled ? ~std::uint64_t(0) : 0);
			if (filled && (bits & 63)) data.back().back() >>= 64 - (bits & 63);
			if (words == 1) break;
			bits = words;
		}
	}
	int universe_size() const { return n; }
	int size() const { return count; }
	bool empty() const { return count == 0; }
	bool contains(int p) const { assert(0 <= p && p < n); return data[0][p >> 6] >> (p & 63) & 1; }
	bool insert(int p) {
		assert(0 <= p && p < n);
		if (contains(p)) return false;
		++count;
		for (auto &level : data) {
			auto &word = level[p >> 6];
			auto before = word;
			word |= std::uint64_t(1) << (p & 63);
			if (before) break;
			p >>= 6;
		}
		return true;
	}
	bool erase(int p) {
		assert(0 <= p && p < n);
		if (!contains(p)) return false;
		--count;
		for (auto &level : data) {
			auto &word = level[p >> 6];
			word &= ~(std::uint64_t(1) << (p & 63));
			if (word) break;
			p >>= 6;
		}
		return true;
	}
	int next(int p) const {
		p = std::max(p, 0);
		if (p >= n) return n;
		for (int h = 0; h < int(data.size()); ++h) {
			if (std::size_t(p >> 6) >= data[h].size()) return n;
			auto word = data[h][p >> 6] & (~std::uint64_t(0) << (p & 63));
			if (word) {
				p = (p & ~63) + std::countr_zero(word);
				while (h--) p = p * 64 + std::countr_zero(data[h][p]);
				return p;
			}
			p = (p >> 6) + 1;
		}
		return n;
	}
	int prev(int p) const {
		p = std::min(p, n - 1);
		if (p < 0) return -1;
		for (int h = 0; h < int(data.size()); ++h) {
			auto word = data[h][p >> 6] & (~std::uint64_t(0) >> (63 - (p & 63)));
			if (word) {
				p = (p & ~63) + 63 - std::countl_zero(word);
				while (h--) p = p * 64 + 63 - std::countl_zero(data[h][p]);
				return p;
			}
			p = (p >> 6) - 1;
			if (p < 0) return -1;
		}
		return -1;
	}
	void clear() { for (auto &level : data) std::fill(level.begin(), level.end(), 0); count = 0; }
	std::size_t memory_usage() const {
		std::size_t bytes = data.capacity() * sizeof(data[0]);
		for (const auto &level : data) bytes += level.capacity() * sizeof(std::uint64_t);
		return bytes;
	}
};

#endif
