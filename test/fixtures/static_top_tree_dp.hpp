#ifndef TEST_STATIC_TOP_TREE_DP_HPP
#define TEST_STATIC_TOP_TREE_DP_HPP

#include "math/mint.hpp"

struct StaticTopTreeSumDP {
	struct Vertex { mint value = 0, a = 1, b = 0; };
	struct Point { mint sum, count; };
	struct Path { mint a, b, sum, count; };
	static Point point_identity() { return {0, 0}; }
	static Path path_identity() { return {1, 0, 0, 0}; }
	static Point rake(const Point& x, const Point& y) { return {x.sum + y.sum, x.count + y.count}; }
	static Point add_edge(const Path& x) { return {x.sum, x.count}; }
	static Path add_vertex(const Point& p, const Vertex& v) {
		mint count = p.count + 1;
		return {v.a, v.b, v.a * (p.sum + v.value) + v.b * count, count};
	}
	static Path compress(const Path& x, const Path& y) {
		return {x.a * y.a, x.a * y.b + x.b, x.sum + x.a * y.sum + x.b * y.count, x.count + y.count};
	}
};

struct StaticTopTreeRerootDP {
	using Vertex = StaticTopTreeSumDP::Vertex;
	using Point = StaticTopTreeSumDP::Point;
	using Half = StaticTopTreeSumDP::Path;
	struct Path { Half forward, reverse; };
	static Point point_identity() { return {0, 0}; }
	static Path path_identity() { return {StaticTopTreeSumDP::path_identity(), StaticTopTreeSumDP::path_identity()}; }
	static Point rake(const Point& x, const Point& y) { return StaticTopTreeSumDP::rake(x, y); }
	static Point add_edge(const Path& x) { return {x.forward.sum, x.forward.count}; }
	static Path add_vertex(const Point& p, const Vertex& v) {
		return {StaticTopTreeSumDP::add_vertex(p, v), {v.a, v.b, p.sum + v.value, p.count + 1}};
	}
	static Path compress(const Path& x, const Path& y) {
		return {StaticTopTreeSumDP::compress(x.forward, y.forward), StaticTopTreeSumDP::compress(y.reverse, x.reverse)};
	}
};

#endif
