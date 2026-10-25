#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum"
#include "graph/offline_dynamic_connectivity.hpp"
#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<std::int64_t> sum(n);
	for (auto &x : sum) input.read(x);
	OfflineDynamicConnectivity graph(n);
	graph.reserve(q, q);
	struct Query { int type, vertex; std::int64_t delta; };
	std::vector<Query> queries;
	queries.reserve(q);
	for (int i = 0; i < q; ++i) {
		int type, u;
		input.read(type), input.read(u);
		if (type < 2) {
			int v;
			input.read(v);
			if (type == 0) graph.link(u, v);
			else graph.cut(u, v);
		} else {
			std::int64_t delta = 0;
			if (type == 2) input.read(delta);
			graph.query(), queries.push_back({type, u, delta});
		}
	}
	std::vector<int> parent(n, -1);
	graph.run([&](int root, int child) {
		parent[child] = root, sum[root] += sum[child];
	}, [&](int root, int child) {
		sum[root] -= sum[child], parent[child] = -1;
	}, [&](int id, const RollbackUnionFind &uf) {
		auto [type, vertex, delta] = queries[id];
		if (type == 2) {
			for (int v = vertex; v != -1; v = parent[v]) sum[v] += delta;
		} else output.println(sum[uf.leader(vertex)]);
	});
}
