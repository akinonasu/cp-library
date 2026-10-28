#define PROBLEM "https://judge.yosupo.jp/problem/closest_pair"
#include "../data_structure/kd_tree.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <limits>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int cases = 0;
	input.read(cases);
	while (cases--) {
		int n = 0;
		input.read(n);
		using Tree = KDTree<std::int64_t, 2, std::int64_t>;
		std::vector<Tree::point_type> points(n);
		for (auto& point : points) input.read(point[0]), input.read(point[1]);
		Tree tree(points);
		std::int64_t distance = std::numeric_limits<std::int64_t>::max();
		int a = -1, b = -1;
		for (int i = 0; i < n; ++i) {
			auto nearest = tree.nearest(points[i], i);
			if (nearest->distance_squared < distance) {
				distance = nearest->distance_squared;
				a = i;
				b = nearest->index;
			}
		}
		output.write(a);
		output.write(' ');
		output.println(b);
	}
}
