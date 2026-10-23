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
}
