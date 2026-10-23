#ifndef FLOOR_QUOTIENTS_HPP
#define FLOOR_QUOTIENTS_HPP

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace floor_quotient_detail {
inline std::uint64_t divide(std::uint64_t n, std::uint64_t divisor) {
	if constexpr (std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits >= 53) {
		if (n <= (std::uint64_t(1) << 53)) {
			std::uint64_t value = double(n) / double(divisor);
			return value - (value * divisor > n);
		}
	}
	return n / divisor;
}
}

class FloorQuotients {
	std::uint64_t n_, root_ = 0;
	std::size_t large_ = 0;
	std::vector<std::uint64_t> values_;

  public:
	explicit FloorQuotients(std::uint64_t n = 0) : n_(n) {
		if (!n) return;
		root_ = std::sqrt(static_cast<long double>(n));
		while (__uint128_t(root_) * root_ > n) --root_;
		while (__uint128_t(root_ + 1) * (root_ + 1) <= n) ++root_;
		large_ = n / (root_ + 1);
		values_.reserve(large_ + root_);
		for (std::size_t i = 1; i <= large_; ++i) values_.push_back(floor_quotient_detail::divide(n, i));
		for (auto x = root_; x; --x) values_.push_back(x);
	}
	FloorQuotients(const FloorQuotients&) = default;
	FloorQuotients& operator=(const FloorQuotients&) = default;
	FloorQuotients(FloorQuotients&& other) noexcept : FloorQuotients() { swap(other); }
	FloorQuotients& operator=(FloorQuotients&& other) noexcept {
		if (this != &other) { FloorQuotients empty; swap(empty); swap(other); }
		return *this;
	}
	void swap(FloorQuotients& other) noexcept {
		std::swap(n_, other.n_), std::swap(root_, other.root_), std::swap(large_, other.large_);
		values_.swap(other.values_);
	}
	std::uint64_t max_n() const { return n_; }
	std::uint64_t root() const { return root_; }
	std::size_t size() const { return values_.size(); }
	std::size_t large_size() const { return large_; }
	const std::vector<std::uint64_t>& values() const { return values_; }
	bool contains(std::uint64_t x) const { return x && x <= n_ && (x <= root_ || n_ / (n_ / x) == x); }
	std::size_t index(std::uint64_t x) const {
		assert(x && x <= n_);
		return x <= root_ ? size() - x : n_ / x - 1;
	}
	std::size_t divided_index(std::size_t i, std::uint64_t divisor) const {
		assert(i < size() && divisor && divisor <= values_[i]);
		if (i >= large_) return size() - values_[i] / divisor;
		const auto d = (i + 1) * divisor;
		return d <= large_ ? d - 1 : size() - n_ / d;
	}
};

#endif
