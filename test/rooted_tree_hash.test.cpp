#define PROBLEM "https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification"
#include "tree/rooted_tree_hash.hpp"
#include "components/fast_io_bulk.hpp"


int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<int> parents(n, -1);
	int vertex = 1;
	read_unsigned_records<1>(input, n - 1, [&](unsigned p) { parents[vertex++] = p; });
	RootedTreeHash tree;
	tree.build_parents(parents);
	auto answer = tree.classify();
	int count = *std::max_element(answer.begin(), answer.end()) + 1;
	output.write(count);
	output.write('\n');
	write_unsigned_records<1>(output, n, [&](std::size_t v) { return std::array{unsigned(answer[v])}; });
}
