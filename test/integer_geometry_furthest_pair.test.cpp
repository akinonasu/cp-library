#define PROBLEM "https://judge.yosupo.jp/problem/furthest_pair"
#include "integer_geometry/convex.hpp"
#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int t = 0; input.read(t);
	while (t--) {
		int n = 0; input.read(n);
		std::vector<integer_geometry::Point> points(n);
		for (auto &p : points) {
			std::int64_t x = 0, y = 0;
			input.read(x), input.read(y);
			p = {x, y};
		}
		auto pair = integer_geometry::furthest_pair(points);
		output.write(pair->first), output.write(' '), output.println(pair->second);
	}
}
