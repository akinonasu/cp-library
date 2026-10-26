#ifndef BINOMIAL_PREFIX_ONLINE_HPP
#define BINOMIAL_PREFIX_ONLINE_HPP

#include "mint_inner_product.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace binomial_prefix_detail {
template<class Mint, class Table> class Online {
	Table table_;
	int max_n_ = 0, block_ = 1;
	std::vector<Mint> scaled_factorial_, data_;
	void check([[maybe_unused]] int n) const { assert(0 <= n && n <= max_n_); }
	static std::size_t offset(int row) { return std::size_t(row) * (std::size_t(row) + 1) / 2; }
	template<bool Reverse> static Mint dot(const Mint* a, const Mint* b, int size) {
		if constexpr (std::is_same_v<Mint, mint>) {
			return mint_detail::inner_product<Reverse>(a, b, size);
		} else {
			Mint sum(0);
			for (int i = 0; i < size; ++i) sum += a[i] * b[Reverse ? -i : i];
			return sum;
		}
	}
	Mint range(int n, int l, int r) const {
		return table_.factorial[n] * dot<true>(table_.inverse_factorial.data() + l, table_.inverse_factorial.data() + n - l, r - l);
	}

  public:
	explicit Online(int max_n = 0, int block_size = 0) { build(max_n, block_size); }
	void build(int max_n, int block_size = 0) {
		assert(max_n >= 0 && block_size >= 0);
		table_.build(max_n);
		max_n_ = max_n;
		block_ = block_size ? block_size : std::max(1, int(std::sqrt(double(max_n) + 1)));
		if (max_n < 2) { scaled_factorial_.clear(); data_.clear(); return; }
		scaled_factorial_.resize(std::size_t(max_n) + 1);
		Mint inverse_power = table_.power2[max_n].inv() * table_.half;
		for (int i = max_n; i >= 0; --i) {
			scaled_factorial_[i] = table_.factorial[i] * inverse_power;
			inverse_power += inverse_power;
		}
		int rows = max_n / block_;
		data_.resize(offset(rows) + std::size_t(rows) + 1);
		for (int row = 0; row <= rows; ++row) {
			int n = row * block_;
			Mint factor = scaled_factorial_[n] + scaled_factorial_[n], sum(0);
			auto base = offset(row);
			for (int column = 0; column <= row / 2; ++column) {
				int k = column * block_;
				if (column) sum += dot<true>(table_.inverse_factorial.data() + k - block_, table_.inverse_factorial.data() + n - k + block_, block_);
				Mint value = sum * factor;
				data_[base + row - column] = Mint(1) - value;
				data_[base + column] = value + table_.inverse_factorial[k] * table_.inverse_factorial[n - k] * factor;
			}
		}
	}
	int max_n() const { return max_n_; }
	int block_size() const { return block_; }
	std::size_t table_size() const { return data_.size(); }
	Mint query(int n, std::int64_t k) const {
		check(n);
		if (k <= 0) return Mint(0);
		if (k >= std::int64_t(n) + 1) return table_.power2[n];
		bool reflect = k > (n + 1) / 2;
		int m = reflect ? n + 1 - int(k) : int(k);
		Mint value(1);
		if (m != 1) {
			if (m == (n + 1) / 2) value = table_.center(n);
			else {
				int row = std::min(int((std::int64_t(n) + block_ / 2) / block_), max_n_ / block_);
				int column = std::min(int((std::int64_t(m - 1) + block_ / 2) / block_), row);
				int start = row * block_, anchor = column * block_;
				int center = (n + 1) / 2;
				auto distance = std::int64_t(std::abs(n - start)) + std::abs(m - 1 - anchor);
				if (std::min(m - 1, center - m) <= distance) {
					value = m - 1 <= center - m ? Mint(1) + range(n, 1, m) : table_.center(n) - range(n, m, center);
				} else {
					value = data_[offset(row) + column];
					if (start < n) value -= table_.inverse_factorial[anchor] * dot<false>(scaled_factorial_.data() + start, table_.inverse_factorial.data() + start - anchor, n - start);
					else if (n < start) {
						int first = std::max(n, anchor);
						value += table_.inverse_factorial[anchor] * dot<false>(scaled_factorial_.data() + first, table_.inverse_factorial.data() + first - anchor, start - first);
					}
					value *= table_.power2[n];
					if (anchor < m - 1) value += range(n, anchor + 1, m);
					else if (m - 1 < anchor) value -= range(n, m, std::min(anchor, n) + 1);
				}
			}
		}
		return reflect ? table_.power2[n] - value : value;
	}
	Mint prod(int n, std::int64_t l, std::int64_t r) const { assert(l <= r); return query(n, r) - query(n, l); }
};
}

#endif
