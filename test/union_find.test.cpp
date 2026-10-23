#define PROBLEM "https://judge.yosupo.jp/problem/unionfind"
#include "data_structure/union_find/union_find.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	UnionFind uf(n);
	while (q--) {
		int type = 0, a = 0, b = 0;
		in.read(type), in.read(a), in.read(b);
		if (type == 0) uf.merge(a, b);
		else out.println(int(uf.same(a, b)));
	}
}
