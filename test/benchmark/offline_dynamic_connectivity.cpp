#ifndef OFFLINE_CONNECTIVITY_HEADER
#define OFFLINE_CONNECTIVITY_HEADER "graph/connectivity/offline_dynamic_connectivity.hpp"
#endif
#include OFFLINE_CONNECTIVITY_HEADER
#include "components/fast_io.hpp"
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
	FastInput input;
	int n, q;
	input.read(n), input.read(q);
	std::vector<std::int64_t> initial(n);
	for (auto &x : initial) input.read(x);
	struct Operation { int type, u; std::int64_t x; };
	std::vector<Operation> operations(q);
	for (auto &op : operations) {
		input.read(op.type), input.read(op.u);
		if (op.type != 3) input.read(op.x);
	}
	auto start = std::chrono::steady_clock::now();
	OfflineDynamicConnectivity graph(n);
	graph.reserve(q, q);
	std::vector<Operation> queries;
	queries.reserve(q);
	for (auto op : operations) {
		if (op.type == 0) graph.link(op.u, static_cast<int>(op.x));
		else if (op.type == 1) graph.cut(op.u, static_cast<int>(op.x));
#if OFFLINE_UNCOMPRESSED
		graph.query(), queries.push_back(op);
#else
		if (op.type >= 2) graph.query(), queries.push_back(op);
#endif
	}
	auto sum = initial;
	std::vector<int> parent(n, -1);
	std::uint64_t checksum = 0, answers = 0;
	graph.run([&](int root, int child) {
		parent[child] = root, sum[root] += sum[child];
	}, [&](int root, int child) {
		sum[root] -= sum[child], parent[child] = -1;
	}, [&](int id, const RollbackUnionFind &uf) {
		auto [type, vertex, delta] = queries[id];
		if (type == 2) {
			for (int v = vertex; v != -1; v = parent[v]) sum[v] += delta;
		} else if (type == 3) {
			checksum = (checksum ^ static_cast<std::uint64_t>(sum[uf.leader(vertex)])) * 1099511628211ULL;
			++answers;
		}
	});
	auto end = std::chrono::steady_clock::now();
	std::cout << std::setprecision(17) << std::chrono::duration<double, std::milli>(end - start).count()
		<< ' ' << checksum << ' ' << answers << '\n';
}
