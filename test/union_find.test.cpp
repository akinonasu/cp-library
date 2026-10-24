#define PROBLEM "https://judge.yosupo.jp/problem/unionfind"
#include "data_structure/union_find/union_find.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	UnionFind uf(n);
	read_unsigned_records<3>(in, q, [&](unsigned type, unsigned a, unsigned b) {
		if (type == 0) uf.merge(a, b);
		else out.println(int(uf.same(a, b)));
	});
}
