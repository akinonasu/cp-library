#ifndef TEST_TOP_TREE_DP_HPP
#define TEST_TOP_TREE_DP_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include "math/mint.hpp"

struct TopTreeSumDP {
	using Vertex = std::int64_t;
	using Point = std::int64_t;
	using Path = std::int64_t;
	static Point point_identity() { return 0; }
	static Point rake(Point a, Point b) { return a + b; }
	static Path compress(Path a, Path b) { return a + b; }
	static Point add_edge(Path a) { return a; }
	static Path add_vertex(Point a, Vertex v) { return a + v; }
};

struct TopTreeCompositeDP {
	using Vertex = std::array<mint, 2>;
	using Path = Vertex;
	struct Point {};
	static Point point_identity() { return {}; }
	static Point rake(Point, Point) { return {}; }
	static Point add_edge(const Path&) { return {}; }
	static Path add_vertex(Point, const Vertex& v) { return v; }
	static Path compress(const Path& a, const Path& b) { return {a[0] * b[0], a[1] * b[0] + b[1]}; }
};

struct TopTreeDiameterDP {
	using Vertex = std::int64_t;
	struct Point { std::int64_t first = 0, second = 0, diameter = 0; };
	struct Path { std::int64_t length, left, right, diameter; };
	static Point point_identity() { return {}; }
	static Point rake(Point a, Point b) {
		return {std::max(a.first, b.first), std::max({a.second, b.second, std::min(a.first, b.first)}), std::max(a.diameter, b.diameter)};
	}
	static Point add_edge(const Path& a) { return {a.left, 0, a.diameter}; }
	static Path add_vertex(Point a, Vertex v) { return {v, v + a.first, v + a.first, std::max(a.diameter, v + a.first + a.second)}; }
	static Path compress(const Path& a, const Path& b) {
		return {a.length + b.length, std::max(a.left, a.length + b.left), std::max(b.right, b.length + a.right), std::max({a.diameter, b.diameter, a.right + b.left})};
	}
};

struct TopTreeIndependentSetDP {
	using Vertex = mint;
	using Point = std::array<mint, 2>;
	using Path = std::array<mint, 4>;
	static Point point_identity() { return {1, 1}; }
	static Point rake(const Point& a, const Point& b) { return {a[0] * b[0], a[1] * b[1]}; }
	static Point add_edge(const Path& a) { return {a[0] + a[2], a[0]}; }
	static Path add_vertex(const Point& p, Vertex v) { return {p[0], p[0], v * p[1], 0}; }
	static Path compress(const Path& a, const Path& b) {
		return {a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3],
			a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3]};
	}
};

#endif
