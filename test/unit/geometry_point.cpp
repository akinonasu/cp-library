#include "geometry/point.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <random>
#include <sstream>
#include <vector>

using namespace geometry;
constexpr std::int64_t large = 1000000000000000000LL;
static_assert(cross(IPoint{-large, -large}, IPoint{large, -large}, IPoint{large, large}) == __int128_t(4) * large * large);
static_assert(distance_squared(IPoint{-large, -large}, IPoint{large, large}) == __int128_t(8) * large * large);
static_assert(orientation(Point<int>{INT_MIN, INT_MIN}, Point<int>{INT_MAX, INT_MIN}, Point<int>{INT_MAX, INT_MAX}) == 1);
static_assert(ccw(IPoint{0, 0}, IPoint{2, 0}, IPoint{-1, 0}) == 2);
static_assert(ccw(IPoint{0, 0}, IPoint{2, 0}, IPoint{3, 0}) == -2);
static_assert(ccw(IPoint{0, 0}, IPoint{2, 0}, IPoint{1, 0}) == 0);
static_assert(sign(__int128_t(-1)) == -1);

bool exact_polar(geometry::IPoint a, geometry::IPoint b) {
	auto quadrant = [](geometry::IPoint p) {
		if (p == geometry::IPoint{}) return -1;
		if (p.y >= 0) return p.x > 0 || (!p.x && p.y > 0) ? 0 : 1;
		return p.x < 0 ? 2 : 3;
	};
	if (int x = quadrant(a), y = quadrant(b); x != y) return x < y;
	__int128_t turn = __int128_t(a.x) * b.y - __int128_t(a.y) * b.x;
	if (turn) return turn > 0;
	if (a.x != b.x) return a.x < 0 ? a.x > b.x : a.x < b.x;
	return a.y < 0 ? a.y > b.y : a.y < b.y;
}

void check_polar(std::vector<geometry::Point<std::int32_t>> values) {
	auto expected = values;
	std::sort(expected.begin(), expected.end(), [](auto a, auto b) { return exact_polar(geometry::IPoint(a), geometry::IPoint(b)); });
	geometry::polar_sort(values);
	assert(values == expected);
}

int main() {
	IPoint p{3, 4}, q{-2, 1};
	assert((p + q == IPoint{1, 5}) && (p - q == IPoint{5, 3}));
	assert((2 * p == IPoint{6, 8}) && (p * 2 / 2 == p));
	assert(rotate90(rotate90(p)) == -p);
	assert(norm2(p) == 25 && dot(p, q) == -2 && cross(p, q) == 11 && length(p) == 5);
	assert(!unit(IPoint{}));
	auto u = *unit(p);
	assert(std::abs(length(u) - 1) < 1e-15L);
	assert(std::abs(angle(IPoint{1, 0}, IPoint{0, 1}) - pi / 2) < 1e-15L);
	assert(almost_equal(rotate(IPoint{1, 0}, pi), FPoint{-1, 0}));
	assert(distance(IPoint{large, large}, IPoint{large - 1, large}) == 1);
	std::istringstream input("-3 4"); input >> p; assert((p == IPoint{-3, 4}));
	std::ostringstream output; output << p; assert(output.str() == "-3 4");
	assert(sign(1e-13L) == 0 && sign(1e-13L, 0) == 1 && sign(-1e-13L, 0) == -1);
	std::vector<IPoint> points{IPoint{}, IPoint{1, 0}, IPoint{-1, 0}, IPoint{0, 1}, IPoint{0, -1}};
	std::mt19937_64 random(731);
	std::vector<IPoint> full;
	for (std::int64_t x : {LLONG_MIN, LLONG_MIN + 1, -1LL, 0LL, 1LL, LLONG_MAX - 1, LLONG_MAX})
		for (std::int64_t y : {LLONG_MIN, LLONG_MIN + 1, -1LL, 0LL, 1LL, LLONG_MAX - 1, LLONG_MAX}) full.emplace_back(x, y);
	for (auto a : full) for (auto b : full)
		assert(PolarLess<std::int64_t>{}(a, b) == exact_polar(a, b));
	auto sorted = full;
	std::sort(sorted.begin(), sorted.end(), exact_polar);
	std::shuffle(full.begin(), full.end(), random);
	polar_sort(full);
	assert(full == sorted);
	std::vector<Point<std::int32_t>> boundary;
	for (std::int32_t x : {INT_MIN, INT_MIN + 1, -1, 0, 1, INT_MAX - 1, INT_MAX})
		for (std::int32_t y : {INT_MIN, INT_MIN + 1, -1, 0, 1, INT_MAX - 1, INT_MAX}) boundary.emplace_back(x, y);
	for (auto a : boundary) for (auto b : boundary)
		assert(PolarLess<std::int32_t>{}(a, b) == exact_polar(IPoint(a), IPoint(b)));
	for (std::size_t n : {0, 1, 2, 255, 256, 257, 4096}) {
		std::vector<Point<std::int32_t>> values;
		for (std::size_t i = 0; i < n; ++i) values.push_back(boundary[i % boundary.size()]);
		std::shuffle(values.begin(), values.end(), random);
		check_polar(std::move(values));
	}
	std::vector<Point<std::int32_t>> close;
	for (int i = 0; i < 4096; ++i) {
		std::int32_t x = INT_MAX - i, y = x - 1;
		for (int j = 0; j < 4; ++j) {
			close.emplace_back(x, y);
			auto next = rotate90(Point<std::int32_t>{x, y});
			x = next.x, y = next.y;
		}
	}
	std::shuffle(close.begin(), close.end(), random);
	check_polar(std::move(close));
	for (int i = 0; i < 10000; ++i) points.emplace_back(std::int64_t(random() % 200001) - 100000, std::int64_t(random() % 200001) - 100000);
	std::vector<Point<std::int32_t>> compact;
	for (auto point : points) compact.emplace_back(point);
	compact.emplace_back(INT_MIN, INT_MIN); compact.emplace_back(INT_MAX, INT_MIN);
	compact.emplace_back(INT_MIN, INT_MAX); compact.emplace_back(INT_MAX, INT_MAX);
	for (int trial = 0; trial < 10000; ++trial) {
		auto a = compact[random() % compact.size()], b = compact[random() % compact.size()];
		assert(PolarLess<std::int32_t>{}(a, b) == PolarLess<std::int64_t>{}(IPoint(a), IPoint(b)));
	}
	std::sort(points.begin(), points.end(), PolarLess<std::int64_t>{});
	assert(points.front() == IPoint{});
	Real previous = 0;
	for (auto x : points) {
		Real a = argument(x);
		if (a < 0) a += 2 * pi;
		assert(a + 1e-15L >= previous);
		previous = a;
	}
	for (int trial = 0; trial < 200; ++trial) {
		std::vector<Point<std::int32_t>> values;
		for (int i = 0; i < 2000; ++i) {
			std::int32_t x = random(), y = random();
			if (trial % 3 == 0) x = (i % 5 - 2) * (i % 1000), y = (i % 5 - 2) * (i % 1000);
			if (trial % 3 == 1) x = INT_MAX - i % 1000, y = x - 1;
			values.emplace_back(x, y);
		}
		values.emplace_back(0, 0);
		values.emplace_back(INT_MIN, INT_MIN), values.emplace_back(INT_MAX, INT_MAX);
		values.emplace_back(INT_MIN, INT_MAX), values.emplace_back(INT_MAX, INT_MIN);
		auto expected = values;
		std::sort(expected.begin(), expected.end(), PolarLess<std::int32_t>{});
		polar_sort(values);
		assert(values == expected);
		if (trial < 10) check_polar(std::move(values));
	}
	auto expected = points;
	polar_sort(points);
	assert(points == expected);

}
