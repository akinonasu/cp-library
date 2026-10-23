#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#include "../data_structure/persistent/persistent_union_find.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	PersistentUnionFind uf(n);
	std::vector<PersistentUnionFind::root_type> roots(q + 1, uf.initial_root());
	for (int i = 1; i <= q; ++i) {
		int type = 0, k = 0, a = 0, b = 0;
		input.read(type), input.read(k), input.read(a), input.read(b);
		roots[i] = roots[k + 1];
		if (type == 0) roots[i] = uf.merge(roots[i], a, b);
		else output.println(uf.same(roots[i], a, b));
	}
}
