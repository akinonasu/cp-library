#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree"
#include "tree/contour_sum.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	TreeContourSum<> sums;
	{
		std::vector<std::int64_t> values(n);
		for (auto& x : values) input.read(x);
		std::vector<std::pair<int, int>> edges;
		edges.reserve(n - 1);
		for (int i = 1, u, v; i < n; ++i) {
			input.read(u), input.read(v);
			edges.emplace_back(u, v);
		}
		sums.build(n, edges, values);
	}
	while (q--) {
		int type, v;
		input.read(type), input.read(v);
		if (type == 0) {
			std::int64_t x;
			input.read(x);
			sums.add(v, x);
		} else {
			int l, r;
			input.read(l), input.read(r);
			output.println(sums.prod(v, l, r));
		}
	}
}
