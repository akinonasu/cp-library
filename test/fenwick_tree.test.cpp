#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"
#include "data_structure/fenwick_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<long long> a(n);
	for (auto &x : a) in.read(x);
	FenwickTree<long long> tree(a);
	while (q--) {
		int type = 0, l = 0;
		long long r = 0;
		in.read(type), in.read(l), in.read(r);
		if (type == 0) tree.add(l, r);
		else out.println(tree.sum(l, int(r)));
	}
}
