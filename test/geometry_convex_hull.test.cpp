#define PROBLEM "https://judge.yosupo.jp/problem/static_convex_hull"
#include "geometry/convex.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0; input.read(t);
	while (t--) {
		int n = 0; input.read(n);
		std::vector<geometry::IPoint> points(n);
		for (auto &p : points) input.read(p.x), input.read(p.y);
		auto hull = geometry::convex_hull(std::move(points));
		output.println(hull.size());
		for (auto p : hull) output.write(p.x), output.write(' '), output.println(p.y);
	}
}
