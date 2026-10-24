#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#include "../tree/lca.hpp"
#include "components/fast_io_bulk.hpp"
#include <tuple>

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<int> parents(n, -1);
	int vertex = 1;
	read_unsigned_records<1>(input, n - 1, [&](unsigned p) { parents[vertex++] = p; });
	LCA tree;
	tree.build_from_parents(std::move(parents));
	std::vector<unsigned> answers;
	answers.reserve(q);
	read_unsigned_records<2>(input, q, [&](unsigned u, unsigned v) { answers.push_back(tree.lca(u, v)); });
	write_unsigned_records<1>(output, answers.size(), [&](std::size_t i) { return std::array{answers[i]}; });
}
