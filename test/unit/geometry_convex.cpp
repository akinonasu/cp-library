#include "geometry/convex.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

using namespace geometry;

void check_pairs(const std::vector<IPoint> &p) {
	auto near = closest_pair(p), far = furthest_pair(p);
	if (p.size() < 2) { assert(!near && !far); return; }
	auto minimum = distance_squared(p[0], p[1]), maximum = minimum;
	for (std::size_t i = 0; i < p.size(); ++i) for (std::size_t j = i + 1; j < p.size(); ++j) {
		auto d = distance_squared(p[i], p[j]);
		minimum = std::min(minimum, d), maximum = std::max(maximum, d);
	}
	assert(near && far && near->first != near->second && far->first != far->second);
	assert(near->squared_distance == minimum && far->squared_distance == maximum);
	assert(distance_squared(p[near->first], p[near->second]) == minimum);
	assert(distance_squared(p[far->first], p[far->second]) == maximum);
	std::vector<FPoint> f;
	for (auto q : p) f.push_back(to_real(q));
	assert(closest_pair(f)->squared_distance == Real(minimum));
	assert(furthest_pair(f)->squared_distance == Real(maximum));
}

int main() {
	std::vector<IPoint> empty;
	assert(convex_hull(empty).empty() && convex_hull_indices(empty).empty());
	assert(!convex_diameter(empty) && diameter_squared(empty) == 0);
	assert(convex_contains(empty, IPoint(0, 0)) == Location::outside);
	std::vector<IPoint> line{{2, 0}, {0, 0}, {1, 0}, {2, 0}};
	assert((convex_hull(line) == std::vector<IPoint>{{0, 0}, {2, 0}}));
	assert((convex_hull(line, true) == std::vector<IPoint>{{0, 0}, {1, 0}, {2, 0}}));
	check_pairs({}); check_pairs({{0, 0}}); check_pairs({{0, 0}, {0, 0}, {0, 0}});
	check_pairs(line);
	check_pairs({{-1000000000000000000LL, -1000000000000000000LL}, {1000000000000000000LL, 1000000000000000000LL}, {1, 2}});
	std::mt19937_64 random(20261021);
	for (int trial = 0; trial < 120; ++trial) {
		std::vector<IPoint> points(64 + random() % 200);
		for (auto &p : points) {
			p.x = std::int64_t(random() % (std::uint64_t(INT32_MAX) + 1));
			p.y = std::int64_t(random() % (std::uint64_t(INT32_MAX) + 1));
			if (trial % 3 == 0) p.y = p.x;
			p.x += INT64_MIN + 100;
			p.y += INT64_MAX - std::int64_t(INT32_MAX);
		}
		for (bool collinear : {false, true}) {
			auto result = convex_hull(points, collinear);
			auto ids = convex_hull_indices(points, collinear);
			std::vector<FPoint> real;
			for (auto p : points) real.emplace_back(Real(__int128_t(p.x) - INT64_MIN), Real(__int128_t(p.y) - INT64_MAX));
			auto reference = convex_hull(real, collinear, 0);
			assert(ids.size() == result.size() && reference.size() == result.size());
			for (std::size_t i = 0; i < result.size(); ++i) {
				assert(points[ids[i]] == result[i]);
				assert(reference[i].x == Real(__int128_t(result[i].x) - INT64_MIN));
				assert(reference[i].y == Real(__int128_t(result[i].y) - INT64_MAX));
			}
			auto copy = points;
			assert(convex_hull(std::move(copy), collinear) == result);
		}
		auto near = closest_pair(points), far = furthest_pair(points);
		auto minimum = distance_squared(points[0], points[1]), maximum = minimum;
		for (std::size_t i = 0; i < points.size(); ++i) for (std::size_t j = i + 1; j < points.size(); ++j) {
			auto value = distance_squared(points[i], points[j]);
			minimum = std::min(minimum, value); maximum = std::max(maximum, value);
		}
		assert(near->squared_distance == minimum && far->squared_distance == maximum);
	}
	for (int trial = 0; trial < 1500; ++trial) {
		std::vector<IPoint> p(random() % 60);
		for (auto &q : p) q = {std::int64_t(random() % 101) - 50, std::int64_t(random() % 101) - 50};
		check_pairs(p);
		auto hull = convex_hull(p);
		auto ids = convex_hull_indices(p);
		assert(hull.size() == ids.size());
		for (std::size_t i = 0; i < ids.size(); ++i) assert(p[ids[i]] == hull[i]);
		if (hull.size() >= 3) assert(is_convex(hull, true) && signed_area2(hull) > 0);
		if (!hull.empty()) assert(hull.front() == *std::min_element(hull.begin(), hull.end()));
		auto diameter = convex_diameter(hull);
		if (p.size() >= 2) assert(diameter && diameter->squared_distance == furthest_pair(p)->squared_distance);
		for (int query = 0; query < 30; ++query) {
			IPoint q(std::int64_t(random() % 121) - 60, std::int64_t(random() % 121) - 60);
			assert(convex_contains(hull, q) == contains(hull, q));
		}
		if (hull.size() > 2) {
			std::rotate(hull.begin(), hull.begin() + random() % hull.size(), hull.end());
			assert(diameter_squared(hull) == diameter->squared_distance);
			for (auto q : p) assert(convex_contains(hull, q) == contains(hull, q));
		}
	}
	for (int trial = 0; trial < 1500; ++trial) {
		std::vector<IPoint> a(random() % 9), b(random() % 9), sums;
		for (auto &p : a) p = {std::int64_t(random() % 21) - 10, std::int64_t(random() % 21) - 10};
		for (auto &p : b) p = {std::int64_t(random() % 21) - 10, std::int64_t(random() % 21) - 10};
		for (auto x : a) for (auto y : b) sums.push_back(x + y);
		a = convex_hull(a, true), b = convex_hull(b, true);
		if (trial & 1) std::reverse(a.begin(), a.end());
		if (!a.empty() && trial % 3 == 0) a.push_back(a.front());
		assert(minkowski_sum(a, b) == convex_hull(sums));
	}
	for (int trial = 0; trial < 240; ++trial) {
		int n = 64 + random() % 500;
		std::vector<IPoint> points(n);
		std::int64_t offset = trial & 1 ? 1000000000000000000LL : -1000000000000000000LL;
		for (int i = 0; i < n; ++i) {
			std::int64_t x = random() % 2147483648ULL, y = random() % 2147483648ULL;
			if (trial % 5 == 0) x = 0;
			if (trial % 5 == 1) y = 0;
			if (trial % 5 == 2) x = i % 16, y = i / 16;
			points[i] = {offset + x, offset + y};
		}
		if (trial % 7 == 0) points[n - 1] = points[0];
		if (trial % 7 == 1) points[0] = {offset, offset}, points[1] = {offset + 2147483647LL, offset + 2147483647LL};
		if (trial % 7 == 2) points[0] = {offset, offset}, points[1] = {offset + 2147483648LL, offset + 2147483648LL};
		auto expected = distance_squared(points[0], points[1]);
		for (int i = 0; i < n; ++i) for (int j = 0; j < i; ++j) expected = std::min(expected, distance_squared(points[i], points[j]));
		std::shuffle(points.begin(), points.end(), random);
		auto answer = closest_pair(points);
		assert(answer && answer->first != answer->second && answer->squared_distance == expected);
		assert(distance_squared(points[answer->first], points[answer->second]) == expected);
	}
	std::vector<std::uint64_t> packed;
	for (unsigned i = 0; i < 256; ++i) packed.push_back((std::uint64_t(i) << 32) | i);
	assert(!closest_pair_detail::packed(packed, 255, 0));
	auto pair = closest_pair_detail::packed(packed, 255, 32 * packed.size());
	assert(pair && pair->first != pair->second);
	std::vector<IPoint> vertical;
	for (int i = 0; i < 10000; ++i) vertical.push_back({0, i});
	assert(closest_pair(vertical)->squared_distance == 1 && furthest_pair(vertical)->squared_distance == 9999LL * 9999);
	std::vector<FPoint> small{{0, 0}, {1e-10L, 0}, {2e-10L, 1e-10L}};
	assert(std::abs(closest_pair(small)->squared_distance - 1e-20L) < 1e-30L);
	std::vector<FPoint> fa{{0, 0}, {2, 0}, {2, 1}, {0, 1}}, fb{{0, 0}, {1, 0}, {0, 1}};
	std::vector<FPoint> sums;
	for (auto a : fa) for (auto b : fb) sums.push_back(a + b);
	assert(minkowski_sum(fa, fb) == convex_hull(sums));
	std::vector<FPoint> tiny{ {0, 0}, {1e-8L, 0}, {1e-8L, 1e-8L}, {0, 1e-8L} };
	assert(minkowski_sum(tiny, std::vector<FPoint>{ {0, 0} }) == tiny);
	std::vector<FPoint> circle, translated;
	for (int i = 0; i < 100000; ++i) {
		Real theta = 2 * pi * i / 100000;
		circle.push_back({std::cos(theta), std::sin(theta)});
		translated.push_back(circle.back() + FPoint(2, 3));
	}
	std::rotate(translated.begin(), std::min_element(translated.begin(), translated.end()), translated.end());
	assert(minkowski_sum(circle, std::vector<FPoint>{{2, 3}}) == translated);
}
