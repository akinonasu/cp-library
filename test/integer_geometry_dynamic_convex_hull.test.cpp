#define PROBLEM "https://judge.yosupo.jp/problem/static_convex_hull"
#include "integer_geometry/dynamic_convex_hull.hpp"
#include "components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int t = 0; input.read(t);
	while (t--) {
		int n = 0; input.read(n);
		std::vector<integer_geometry::Point> points(n);
		for (auto &p : points) input.read(p.x), input.read(p.y);
		integer_geometry::DynamicConvexHull<> dynamic(points);
		auto hull = dynamic.vertices();
		output.println(hull.size());
		for (auto p : hull) output.write(p.x), output.write(' '), output.println(p.y);
	}
}
