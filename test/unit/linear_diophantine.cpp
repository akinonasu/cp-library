#include "math/util/linear_diophantine.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

using I = std::int64_t;
using W = boost::multiprecision::int256_t;

static_assert(linear_diophantine(2, 3, 7, 0, 10, 0, 10)->first() == std::pair<I, I>{2, 1});
static_assert(linear_diophantine(0, 0, 0, -2, 3, 1, 5)->count() == 20);
static_assert(linear_diophantine(0, 0, 0, -2, 3, 1, 5)->kth(7) == std::pair<I, I>{-1, 4});
static_assert(!linear_diophantine(2, 4, 1, -5, 5, -5, 5));

void brute(I a, I b, I c, I xl, I xr, I yl, I yr) {
	std::vector<std::pair<I, I>> expected;
	for (I x = xl; x < xr; ++x) for (I y = yl; y < yr; ++y) if (W(a) * x + W(b) * y == c) expected.emplace_back(x, y);
	auto answer = linear_diophantine(a, b, c, xl, xr, yl, yr);
	assert(bool(answer) == !expected.empty());
	if (!answer) return;
	assert(answer->count() == expected.size());
	assert(answer->first() == expected.front());
	for (std::size_t i = 0; i < expected.size(); ++i) assert(answer->kth(i) == expected[i]);
	bool caught = false;
	try { (void)answer->kth(answer->count()); } catch (const std::out_of_range &) { caught = true; }
	assert(caught);
}

void wide(I a, I b, I c, I xl, I xr, I yl, I yr) {
	W aa = a, bb = b;
	W g = aa, h = bb, x = 1, u = 0;
	while (h != 0) {
		W q = g / h;
		W next = g - q * h;
		g = h;
		h = next;
		next = x - q * u;
		x = u;
		u = next;
	}
	if (g < 0) g = -g, x = -x;
	auto answer = linear_diophantine(a, b, c, xl, xr, yl, yr);
	if (!g) {
		assert(bool(answer) == (!c && xl < xr && yl < yr));
		if (answer) assert(W(answer->count()) == (W(xr) - xl) * (W(yr) - yl));
		return;
	}
	if (W(c) % g != 0) { assert(!answer); return; }
	W first_x, first_y, dx, dy;
	if (b) {
		first_x = x * (W(c) / g);
		first_y = (W(c) - aa * first_x) / bb;
		dx = bb / g;
		dy = -aa / g;
		if (dx < 0) dx = -dx, dy = -dy;
	} else {
		first_x = W(c) / a;
		first_y = 0;
		dx = 0;
		dy = 1;
	}
	W lower = -(W(1) << 250), upper = (W(1) << 250);
	auto ceil = [](W n, W d) -> W {
		if (d < 0) n = -n, d = -d;
		W q = n / d;
		if (n % d > 0) ++q;
		return q;
	};
	auto constrain = [&](W origin, W step, W lo, W hi) {
		if (!step) return lo <= origin && origin < hi;
		W l, r;
		if (step > 0) l = ceil(lo - origin, step), r = ceil(hi - origin, step) - 1;
		else l = ceil(hi - 1 - origin, step), r = ceil(lo - 1 - origin, step) - 1;
		lower = std::max(lower, l);
		upper = std::min(upper, r);
		return lower <= upper;
	};
	bool exists = constrain(first_x, dx, xl, xr) && constrain(first_y, dy, yl, yr);
	assert(bool(answer) == exists);
	if (!answer) return;
	assert(W(answer->count()) == upper - lower + 1);
	assert(W(answer->first().first) == first_x + lower * dx);
	assert(W(answer->first().second) == first_y + lower * dy);
	for (__uint128_t k : {__uint128_t(0), answer->count() / 2, answer->count() - 1}) {
		auto [px, py] = answer->kth(k);
		assert(xl <= px && px < xr && yl <= py && py < yr);
		assert(W(a) * px + W(b) * py == c);
		assert(W(px) == first_x + (lower + W(k)) * dx);
		assert(W(py) == first_y + (lower + W(k)) * dy);
	}
}

int main() {
	for (I a = -5; a <= 5; ++a) for (I b = -5; b <= 5; ++b) for (I c = -15; c <= 15; ++c)
		for (I xl = -3; xl <= 3; ++xl) for (I yl = -3; yl <= 3; ++yl) brute(a, b, c, xl, 4, yl, 4);
	const I low = std::numeric_limits<I>::min(), high = std::numeric_limits<I>::max();
	std::array<I, 9> values{low, low + 1, -3, -1, 0, 1, 3, high - 1, high};
	for (I a : values) for (I b : values) for (I c : values) {
		brute(a, b, c, -3, 4, -3, 4);
		wide(a, b, c, low, high, low, high);
		wide(a, b, c, low, low + 2, high - 2, high);
	}
	auto rectangle = linear_diophantine(0, 0, 0, low, high, low, high);
	__uint128_t width = (__uint128_t(1) << 64) - 1;
	assert(rectangle->count() == width * width);
	assert((rectangle->kth(rectangle->count() - 1) == std::pair<I, I>(high - 1, high - 1)));
	assert((rectangle->kth(width) == std::pair<I, I>(low + 1, low)));
	std::mt19937_64 random(417998);
	for (int t = 0; t < 20000; ++t) {
		I a = std::bit_cast<I>(random()), b = std::bit_cast<I>(random()), c = std::bit_cast<I>(random());
		I xl = std::bit_cast<I>(random()), xr = std::bit_cast<I>(random()), yl = std::bit_cast<I>(random()), yr = std::bit_cast<I>(random());
		if (xl > xr) std::swap(xl, xr);
		if (yl > yr) std::swap(yl, yr);
		wide(a, b, c, xl, xr, yl, yr);
		if (t < 1000) brute(a, b, c, -3, 4, -3, 4);
	}
	assert(!linear_diophantine(1, 1, 1, 0, 0, 0, 4));
	assert(!linear_diophantine(0, 0, 0, 0, 4, 0, 0));
	for (bool axis : {false, true}) {
		bool caught = false;
		try { (void)linear_diophantine(1, 1, 1, axis ? 4 : 0, 3, axis ? 0 : 4, 3); } catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
}
