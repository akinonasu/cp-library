#include "data_structure/range_query/sparse_table.hpp"
#include "data_structure/range_query/disjoint_sparse_table.hpp"
#include "data_structure/segment_tree/dynamic_segtree.hpp"
#include "data_structure/weighted_union_find.hpp"
#include "tree/rerooting.hpp"
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

using I = std::int64_t;
using Clock = std::chrono::steady_clock;
I sum(I a, I b) { return a + b; }
I zero() { return 0; }
I minimum(I a, I b) { return std::min(a, b); }
const std::string &minimum_string(const std::string &a, const std::string &b) { return std::min(a, b); }
double milliseconds(Clock::time_point start) { return std::chrono::duration<double, std::milli>(Clock::now() - start).count(); }

template<class Table, class T>
void tables(const std::vector<T> &values, int rounds, bool reuse) {
	std::mt19937 rng(20261020);
	std::vector<std::pair<int, int>> ranges(200000);
	for (auto &[l, r] : ranges) {
		l = rng() % values.size(), r = rng() % values.size();
		if (l > r) std::swap(l, r);
		++r;
	}
	Table table;
	double build = 0, query = 0;
	std::uint64_t checksum = 0;
	for (int trial = 0; trial < rounds; ++trial) {
		if (!reuse) table = Table{};
		auto start = Clock::now();
		table.build(values);
		build += milliseconds(start);
		start = Clock::now();
		for (auto [l, r] : ranges) checksum = checksum * 1000003 + std::hash<T>{}(table.prod(l, r));
		query += milliseconds(start);
	}
	std::cout << "build_ms=" << build << " query_ms=" << query << " checksum=" << checksum << '\n';
}

int main(int argc, char **argv) {
	if (argc != 6) return 1;
	std::string mode = argv[1], shape = argv[2];
	int n = std::stoi(argv[3]), rounds = std::stoi(argv[4]);
	bool reuse = std::stoi(argv[5]);
	std::mt19937_64 rng(20261020);
	std::cout << std::fixed << std::setprecision(6);
	if (mode == "sparse_strings") {
		std::vector<std::string> values(n, std::string(64, 'a'));
		for (auto &s : values) for (char &c : s) c += rng() % 26;
		tables<SparseTable<std::string, minimum_string>>(values, rounds, reuse);
	} else if (mode == "sparse" || mode == "disjoint") {
		std::vector<I> values(n);
		for (auto &x : values) x = rng() % 1000000;
		if (mode == "sparse") tables<SparseTable<I, minimum>>(values, rounds, reuse);
		else tables<DisjointSparseTable<I, sum>>(values, rounds, reuse);
	} else if (mode == "dynamic") {
		constexpr I limit = I(1) << 62;
		std::vector<I> keys(n);
		for (int i = 0; i < n; ++i) keys[i] = shape == "random" ? rng() % limit : shape == "cluster" ? (I(1) << 60) + i : i;
		if (shape == "decreasing") std::reverse(keys.begin(), keys.end());
		if (shape == "cluster") std::shuffle(keys.begin(), keys.end(), rng);
		std::vector<int> changes(4 * n);
		for (int &x : changes) x = rng() % n;
		DynamicSegtree<I, sum, zero> tree(limit);
		tree.reserve(n);
		auto start = Clock::now();
		for (int i = 0; i < n; ++i) tree.set(keys[i], i);
		double build = milliseconds(start);
		start = Clock::now();
		for (int x : changes) tree.multiply(keys[x], 1);
		double update = milliseconds(start);
		std::uint64_t checksum = tree.all_prod();
		for (int i = 0; i < n; ++i) checksum = checksum * 1000003 + tree.get(keys[i]);
		std::cout << "build_ms=" << build << " update_ms=" << update << " checksum=" << checksum << '\n';
	} else if (mode == "rerooting") {
		Rerooting<I> tree(n);
		std::vector<int> labels(n);
		std::iota(labels.begin(), labels.end(), 0);
		std::shuffle(labels.begin(), labels.end(), rng);
		std::vector<std::pair<int, int>> edges;
		for (int v = 1; v < n; ++v) edges.emplace_back(labels[shape == "path" ? v - 1 : shape == "star" ? 0 : rng() % v], labels[v]);
		std::shuffle(edges.begin(), edges.end(), rng);
		for (auto [u, v] : edges) tree.add_edge(u, v);
		double build = 0;
		std::uint64_t checksum = 0;
		for (int trial = 0; trial < rounds; ++trial) {
			auto run = [&](auto &target) {
				auto start = Clock::now();
				target.build(I(0), sum, [](I x, int) { return x + 1; }, [](I x, int, int, int) { return x; }, trial % n);
				build += milliseconds(start);
				for (auto x : target.values()) checksum = checksum * 1000003 + x;
			};
			if (reuse) run(tree);
			else { auto copy = tree; run(copy); }
		}
		std::cout << "build_ms=" << build << " checksum=" << checksum << '\n';
	} else if (mode == "weighted_union_find") {
		WeightedUnionFind<I> uf(n);
		for (int step = 1; step < n; step *= 2)
			for (int v = 0; v + step < n; v += 2 * step) uf.merge(v, v + step, step);
		std::vector<std::pair<int, int>> queries(1000000);
		for (auto &[u, v] : queries) u = rng() % n, v = rng() % n;
		auto start = Clock::now();
		std::uint64_t checksum = 0;
		for (auto [u, v] : queries) checksum = checksum * 1000003 + uf.diff(u, v);
		std::cout << "query_ms=" << milliseconds(start) << " checksum=" << checksum << '\n';
	} else return 1;
}
