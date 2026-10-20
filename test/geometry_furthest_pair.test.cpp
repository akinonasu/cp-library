#define PROBLEM "https://judge.yosupo.jp/problem/furthest_pair"
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
		auto pair = geometry::furthest_pair(points);
		output.write(pair->first), output.write(' '), output.println(pair->second);
	}
}
