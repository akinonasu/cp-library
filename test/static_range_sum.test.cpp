#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"
#include "data_structure/fenwick_tree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::int64_t> a(n);
	for (auto &x : a) input.read(x);
	FenwickTree tree(a);
	while (q--) {
		int l = 0, r = 0;
		input.read(l), input.read(r);
		output.println(tree.sum(l, r));
	}
}
