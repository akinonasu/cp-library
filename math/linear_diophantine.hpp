#ifndef LINEAR_DIOPHANTINE_HPP
#define LINEAR_DIOPHANTINE_HPP

#include "integer_division.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

class LinearDiophantineSolution {
	std::int64_t x_, y_;
	__int128_t dx_, dy_;
	__uint128_t count_, width_;
	constexpr LinearDiophantineSolution(std::int64_t x, std::int64_t y, __int128_t dx, __int128_t dy, __uint128_t count, __uint128_t width = 0)
		: x_(x), y_(y), dx_(dx), dy_(dy), count_(count), width_(width) {}
	friend constexpr std::optional<LinearDiophantineSolution> linear_diophantine(std::int64_t, std::int64_t, std::int64_t, std::int64_t, std::int64_t, std::int64_t, std::int64_t);

  public:
	constexpr __uint128_t count() const { return count_; }
	constexpr std::pair<std::int64_t, std::int64_t> kth(__uint128_t k) const {
		if (k >= count_) throw std::out_of_range("linear_diophantine index");
		if (width_) return {std::int64_t(__int128_t(x_) + __int128_t(k / width_)), std::int64_t(__int128_t(y_) + __int128_t(k % width_))};
		return {std::int64_t(x_ + __int128_t(k) * dx_), std::int64_t(y_ + __int128_t(k) * dy_)};
	}
	constexpr std::pair<std::int64_t, std::int64_t> first() const { return {x_, y_}; }
};

constexpr std::optional<LinearDiophantineSolution> linear_diophantine(std::int64_t a, std::int64_t b, std::int64_t c, std::int64_t xl, std::int64_t xr, std::int64_t yl, std::int64_t yr) {
	using I = __int128_t;
	if (xl > xr || yl > yr) throw std::invalid_argument("linear_diophantine interval");
	if (xl == xr || yl == yr) return std::nullopt;
	if (!a && !b) {
		if (c) return std::nullopt;
		__uint128_t width = I(yr) - yl;
		return LinearDiophantineSolution(xl, yl, 0, 0, __uint128_t(I(xr) - xl) * width, width);
	}
	std::uint64_t r = a < 0 ? -I(a) : I(a), s = b < 0 ? -I(b) : I(b);
	I x = 1, u = 0, y = 0, v = 1;
	if (s) for (;;) {
		std::uint64_t q = r / s;
		r -= q * s;
		if (!r) {
			r = s, x = u, y = v;
			break;
		}
		x -= q * u;
		y -= q * v;
		std::swap(r, s);
		std::swap(x, u);
		std::swap(y, v);
	}
	if (I(c) % r) return std::nullopt;
	x *= I(c) / r;
	y *= I(c) / r;
	if (a < 0) x = -x;
	if (b < 0) y = -y;
	I dx = I(b) / r, dy = -I(a) / r;
	if (dx < 0 || (!dx && dy < 0)) dx = -dx, dy = -dy;
	I left = std::numeric_limits<I>::min(), right = std::numeric_limits<I>::max();
	auto restrict = [&](I origin, I step, I lo, I hi) {
		if (!step) return lo <= origin && origin < hi;
		I l = step > 0 ? ceil_div(lo - origin, step) : ceil_div(hi - 1 - origin, step);
		I r = step > 0 ? floor_div(hi - 1 - origin, step) : floor_div(lo - origin, step);
		left = std::max(left, l);
		right = std::min(right, r);
		return left <= right;
	};
	if (!restrict(x, dx, xl, xr) || !restrict(y, dy, yl, yr)) return std::nullopt;
	return LinearDiophantineSolution(std::int64_t(x + left * dx), std::int64_t(y + left * dy), dx, dy, __uint128_t(right - left) + 1);
}

#endif
