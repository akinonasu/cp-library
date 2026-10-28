#include "data_structure/kd_tree.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

template<class Tree>
typename Tree::distance_type distance(const typename Tree::point_type& a, const typename Tree::point_type& b) {
	using D = typename Tree::distance_type;
	using T = typename Tree::point_type::value_type;
	D sum = 0;
	for (std::size_t i = 0; i < a.size(); ++i) {
		if constexpr (std::is_integral_v<T>) {
			__int128_t delta = __int128_t(a[i]) - __int128_t(b[i]);
			__uint128_t magnitude = delta < 0 ? __uint128_t(-delta) : __uint128_t(delta);
			sum += D(magnitude * magnitude);
		} else {
			D delta = D(a[i]) - D(b[i]);
			sum += delta * delta;
		}
	}
	return sum;
}

template<class Tree>
void nearest_check(const Tree& tree, const std::vector<typename Tree::point_type>& points,
	const typename Tree::point_type& query, int exclude, int k) {
	using D = typename Tree::distance_type;
	std::vector<std::pair<D, int>> expected;
	for (int i = 0; i < int(points.size()); ++i)
		if (i != exclude) expected.emplace_back(distance<Tree>(points[i], query), i);
	std::sort(expected.begin(), expected.end());
	auto nearest = tree.nearest(query, exclude);
	assert(bool(nearest) == !expected.empty());
	if (nearest) {
		assert(nearest->index == expected[0].second);
		assert(nearest->distance_squared == expected[0].first);
	}
	auto actual = tree.k_nearest(query, k, exclude);
	assert(actual.size() == std::min(std::size_t(k), expected.size()));
	for (std::size_t i = 0; i < actual.size(); ++i) {
		assert(actual[i].index == expected[i].second);
		assert(actual[i].distance_squared == expected[i].first);
	}
}

template<class Tree>
void range_check(const Tree& tree, const std::vector<typename Tree::point_type>& points,
	const typename Tree::point_type& lower, const typename Tree::point_type& upper) {
	std::vector<int> expected;
	for (int i = 0; i < int(points.size()); ++i) {
		bool inside = true;
		for (std::size_t axis = 0; axis < lower.size(); ++axis)
			inside &= lower[axis] <= points[i][axis] && points[i][axis] < upper[axis];
		if (inside) expected.push_back(i);
	}
	assert(tree.range_count(lower, upper) == int(expected.size()));
	auto actual = tree.range_report(lower, upper);
	std::sort(actual.begin(), actual.end());
	assert(actual == expected);
	std::vector<int> visited;
	tree.range_visit(lower, upper, [&](int id) { visited.push_back(id); });
	std::sort(visited.begin(), visited.end());
	assert(visited == expected);
}

template<class Tree>
void random_cases(std::uint64_t seed) {
	using Point = typename Tree::point_type;
	using T = typename Point::value_type;
	std::mt19937_64 rng(seed);
	for (int trial = 0; trial < 100; ++trial) {
		int n = rng() % 100;
		std::vector<Point> points(n);
		for (auto& point : points)
			for (auto& coordinate : point) coordinate = T(int(rng() % 65) - 32) / T(std::is_floating_point_v<T> ? 4 : 1);
		if (trial % 5 == 0)
			for (auto& point : points) point.fill(T(3));
		else if (trial % 5 == 1)
			for (auto& point : points)
				for (std::size_t i = 1; i < point.size(); ++i) point[i] = point[0];
		auto original = points;
		int leaf = std::array{1, 2, 3, 8, 16, 32, 64, 1000}[trial % 8];
		Tree tree(points, leaf);
		assert(points == original);
		assert(tree.size() == n && tree.empty() == (n == 0) && tree.leaf_size() == leaf);
		for (int i = 0; i < n; ++i) assert(tree.point(i) == points[i]);
		for (int repeat = 0; repeat < 24; ++repeat) {
			Point query{}, lower{}, upper{};
			for (std::size_t axis = 0; axis < query.size(); ++axis) {
				query[axis] = T(int(rng() % 81) - 40) / T(std::is_floating_point_v<T> ? 4 : 1);
				lower[axis] = T(int(rng() % 81) - 40);
				upper[axis] = T(int(rng() % 81) - 40);
				if (repeat % 4 && upper[axis] < lower[axis]) std::swap(upper[axis], lower[axis]);
			}
			if (n && repeat % 3 == 0) query = points[rng() % n];
			int exclude = n && repeat % 2 ? int(rng() % n) : -1;
			int k = repeat == 0 ? std::numeric_limits<int>::max() : int(rng() % (n + 3));
			nearest_check(tree, points, query, exclude, k);
			range_check(tree, points, lower, upper);
		}
		Point lower{}, upper{};
		lower.fill(T(-100)), upper.fill(T(100));
		range_check(tree, points, lower, upper);
		range_check(tree, points, lower, lower);
		auto copied = tree;
		points.assign(3, Point{});
		assert(copied.size() == n);
		if (n) assert(copied.point(0) == original[0]);
		tree.build(points, 2);
		nearest_check(tree, points, Point{}, 1, 7);
		nearest_check(copied, original, Point{}, -1, 7);
		auto moved = std::move(copied);
		nearest_check(moved, original, Point{}, n ? 0 : -1, 7);
		copied.build({}, 1);
		assert(copied.empty() && copied.size() == 0 && copied.leaf_size() == 1);
		nearest_check(copied, {}, Point{}, -1, 4);
	}
}

template<class Distance>
void narrow_distance_cases(std::array<int, 4> endpoint) {
	using Tree = KDTree<int, 4, Distance>;
	using Point = typename Tree::point_type;
	std::vector<Point> points;
	for (int mask = 0; mask < 16; ++mask) {
		Point point{};
		for (int d = 0; d < 4; ++d) if (mask >> d & 1) point[d] = endpoint[d];
		points.push_back(point);
	}
	points.push_back({0, 0, 0, 0});
	points.push_back({1, 0, 0, 0});
	points.push_back({1, 1, 1, 1});
	assert(distance<Tree>(Point{}, endpoint) == std::numeric_limits<Distance>::max());
	for (int leaf : {1, 2, 4, 16, 64}) {
		Tree tree(points, leaf);
		for (const auto& query : points)
			for (int exclude : {-1, 0, 1, 15, 16, 17, 18})
				for (int k : {0, 1, 3, 30}) nearest_check(tree, points, query, exclude, k);
		Point lower{}, upper = endpoint;
		range_check(tree, points, lower, upper);
		lower.fill(-1);
		for (auto& value : upper) ++value;
		range_check(tree, points, lower, upper);
		range_check(tree, points, endpoint, upper);
		range_check(tree, points, upper, endpoint);
	}
}

int main() {
	using Default = KDTree<>;
	static_assert(std::is_same_v<Default::distance_type, __int128_t>);
	Default empty;
	assert(empty.empty() && empty.size() == 0);
	nearest_check(empty, {}, {0, 0}, -1, 0);
	range_check(empty, {}, {-1, -1}, {1, 1});
	random_cases<KDTree<int, 1>>(101);
	random_cases<KDTree<int, 2>>(102);
	random_cases<KDTree<int, 3>>(103);
	random_cases<KDTree<int, 5>>(105);
	random_cases<KDTree<int, 2, std::int64_t>>(106);
	random_cases<KDTree<double, 2>>(107);
	random_cases<KDTree<float, 3, double>>(108);
	narrow_distance_cases<std::uint8_t>({15, 5, 2, 1});
	narrow_distance_cases<std::uint16_t>({255, 22, 5, 1});
	narrow_distance_cases<std::int8_t>({11, 2, 1, 1});
	narrow_distance_cases<std::int16_t>({181, 2, 1, 1});

	using Tree = KDTree<std::int64_t, 2>;
	using Point = Tree::point_type;
	std::vector<Point> ties{ {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 0}, {0, 0} };
	for (int leaf : {1, 2, 4, 16}) {
		Tree tree(ties, leaf);
		for (int excluded = -1; excluded < int(ties.size()); ++excluded)
			for (int k = 0; k < 10; ++k) nearest_check(tree, ties, {0, 0}, excluded, k);
		range_check(tree, ties, {-1, -1}, {1, 1});
		range_check(tree, ties, {0, 0}, {1, 1});
	}
	std::vector<Point> identical(4096, {7, -9});
	Tree duplicates(identical, 1);
	for (int excluded : {-1, 0, 17, 4095})
		for (int k : {0, 1, 2, 33, 4095, 5000}) nearest_check(duplicates, identical, {7, -9}, excluded, k);
	range_check(duplicates, identical, {7, -9}, {8, -8});
	range_check(duplicates, identical, {6, -10}, {7, -9});

	constexpr std::int64_t limit = 1000000000000000000LL;
	std::vector<Point> wide{ {-limit, -limit}, {limit, limit}, {-limit, limit}, {limit, -limit}, {0, 0} };
	Tree wide_tree(wide, 1);
	for (const auto& point : wide)
		for (int excluded = -1; excluded < 5; ++excluded) nearest_check(wide_tree, wide, point, excluded, 8);
	range_check(wide_tree, wide, {-limit, -limit}, {limit, limit});
	using IntTree = KDTree<int, 2>;
	constexpr int low = std::numeric_limits<int>::min(), high = std::numeric_limits<int>::max();
	std::vector<IntTree::point_type> extremes{ {low, low}, {high, high}, {low, high}, {high, low} };
	IntTree extreme_tree(extremes, 1);
	nearest_check(extreme_tree, extremes, {low, low}, 0, 5);
	range_check(extreme_tree, extremes, {low, low}, {high, high});
	using UnsignedTree = KDTree<std::uint64_t, 1, __uint128_t>;
	std::vector<UnsignedTree::point_type> unsigned_points{ {0}, {std::numeric_limits<std::uint64_t>::max()} };
	UnsignedTree unsigned_tree(unsigned_points, 1);
	nearest_check(unsigned_tree, unsigned_points, {0}, 0, 3);

	using FloatingTree = KDTree<double, 2>;
	std::vector<FloatingTree::point_type> fractions{ {-0.0, 0.0}, {0.0, -0.0}, {0.25, 0.5}, {-0.5, -0.25} };
	FloatingTree real_tree(fractions, 1);
	nearest_check(real_tree, fractions, {0, 0}, -1, 10);
	double infinity = std::numeric_limits<double>::infinity();
	range_check(real_tree, fractions, {-infinity, -infinity}, {infinity, infinity});
	range_check(real_tree, fractions, {-infinity, -infinity}, {0.0, 0.0});

	std::vector<Point> line(20000);
	for (int i = 0; i < int(line.size()); ++i) line[i] = {i, 3LL * i};
	Tree line_tree(line);
	for (int i : {0, 1, 1234, 9999, 19999}) nearest_check(line_tree, line, line[i], i, 7);
	range_check(line_tree, line, {100, 300}, {200, 600});
}
