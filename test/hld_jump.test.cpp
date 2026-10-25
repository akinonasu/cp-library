#define PROBLEM "https://judge.yosupo.jp/problem/jump_on_tree"
#include "../data_structure/hld.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	HLD tree(n);
	read_unsigned_records<2>(input, n - 1, [&](unsigned u, unsigned v) { tree.add_edge(u, v); });
	tree.build();
	read_unsigned_records<3>(input, q, [&](unsigned u, unsigned v, unsigned k) { output.println(tree.jump(u, v, k)); });
}
