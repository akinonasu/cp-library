#define PROBLEM "https://judge.yosupo.jp/problem/frequency_table_of_tree_distance"
#include "../data_structure/centroid_decomposition.hpp"
#include "../convolution/convolution_mod.hpp"
#include "../components/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	CentroidDecomposition<> tree(n);
	for (int i = 1; i < n; ++i) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		tree.add_edge(u, v);
	}
	tree.build();
	std::vector<std::int64_t> answer(n);
	auto accumulate = [&](const std::vector<std::int64_t> &counts, int sign) {
		auto product = convolution_integer(counts, counts);
		for (std::size_t d = 1; d < std::min(product.size(), answer.size()); ++d)
			answer[d] += sign * product[d];
	};
	for (int c : tree.order) {
		if (tree.size[c] == 1) continue;
		std::vector<std::int64_t> total(1, 1);
		auto [begin, end] = tree.subtree(c);
		for (int first = begin + 1; first < end;) {
			auto [left, right] = tree.subtree(tree.order[first]);
			std::vector<std::int64_t> branch;
			for (int i = left; i < right; ++i) {
				int distance = tree.distance_to_ancestor(tree.order[i], c);
				if (int(branch.size()) <= distance) branch.resize(distance + 1);
				++branch[distance];
			}
			total.resize(std::max(total.size(), branch.size()));
			for (std::size_t d = 0; d < branch.size(); ++d) total[d] += branch[d];
			accumulate(branch, -1);
			first = right;
		}
		accumulate(total, 1);
	}
	for (int d = 1; d < n; ++d) {
		if (d != 1) output.write(' ');
		output.write(answer[d] / 2);
	}
	output.write('\n');
}
