#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#include "data_structure/union_find/rollback_union_find.hpp"
#include "components/fast_io.hpp"
#include <cstddef>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	struct Node { int a = 0, b = 0, parent = 0, first = -1, next = -1, query = -1, next_query = -1, type = -1; };
	std::vector<Node> nodes(q + 1);
	std::vector<int> version(q + 1), answer(q + 1);
	for (int i = 1; i <= q; ++i) {
		auto &node = nodes[i];
		int k = 0;
		input.read(node.type), input.read(k), input.read(node.a), input.read(node.b);
		node.parent = version[k + 1];
		if (node.type == 0) version[i] = i;
		else {
			version[i] = node.parent;
			if (node.a == node.b) answer[i] = 1;
			else if (node.parent) {
				node.next_query = nodes[node.parent].query;
				nodes[node.parent].query = i;
			}
		}
	}
	for (int i = q; i; --i) {
		auto &node = nodes[i];
		if (node.type == 0 && (node.first != -1 || node.query != -1)) {
			node.next = nodes[node.parent].first;
			nodes[node.parent].first = i;
		}
	}
	RollbackUnionFind uf(n);
	uf.reserve(q);
	struct Frame { int vertex, next; std::size_t state; };
	std::vector<Frame> stack{{0, nodes[0].first, 0}};
	while (!stack.empty()) {
		auto &frame = stack.back();
		if (frame.next == -1) {
			uf.rollback(frame.state);
			stack.pop_back();
			continue;
		}
		int v = frame.next;
		frame.next = nodes[v].next;
		auto state = uf.snapshot();
		uf.merge(nodes[v].a, nodes[v].b);
		for (int i = nodes[v].query; i != -1; i = nodes[i].next_query) answer[i] = uf.same(nodes[i].a, nodes[i].b);
		stack.push_back({v, nodes[v].first, state});
	}
	for (int i = 1; i <= q; ++i) if (nodes[i].type == 1) output.println(answer[i]);
}
