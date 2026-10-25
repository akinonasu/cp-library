#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#ifndef ROLLBACK_HEADER
#define ROLLBACK_HEADER "data_structure/rollback_union_find.hpp"
#endif
#include ROLLBACK_HEADER
#include "components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	struct Node { int u = 0, v = 0, child = -1, next = -1; };
	std::vector<Node> nodes(q + 1);
	std::vector<unsigned char> answer(q + 1);
	for (int i = 1; i <= q; ++i) {
		int type, version, u, v;
		input.read(type), input.read(version), input.read(u), input.read(v);
		nodes[i].u = type ? ~u : u, nodes[i].v = v;
		nodes[i].next = nodes[version + 1].child, nodes[version + 1].child = i;
	}
	RollbackUnionFind uf(n);
	uf.reserve(q);
	std::vector<int> stack;
	stack.reserve(q);
	int v = 0;
	while (true) {
		if (int u = nodes[v].child; u != -1) {
			nodes[v].child = nodes[u].next;
			stack.push_back(v), v = u;
			if (nodes[v].u < 0) answer[v] = uf.same(~nodes[v].u, nodes[v].v);
			else uf.merge(nodes[v].u, nodes[v].v);
		} else {
			if (v == 0) break;
			if (nodes[v].u >= 0) uf.undo();
			v = stack.back(), stack.pop_back();
		}
	}
	for (int i = 1; i <= q; ++i) if (nodes[i].u < 0) output.println(int(answer[i]));
}
