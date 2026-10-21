#include "data_structure/union_find.hpp"
#include "data_structure/fenwick_tree.hpp"
#include "data_structure/segment_tree/lazy_segtree.hpp"
#include "graph/connectivity/scc.hpp"
#include "graph/two_sat.hpp"
#include <atcoder/dsu>
#include <atcoder/fenwicktree>
#include <atcoder/lazysegtree>
#include <atcoder/modint>
#include <atcoder/scc>
#include <atcoder/twosat>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>

using Clock = std::chrono::steady_clock;
using U = std::uint64_t;
U state = 17293822569102704641ULL;
U random_value() {
	U z = (state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}
void emit(Clock::time_point start, Clock::time_point end, U checksum) {
	std::cout << std::setprecision(17) << std::chrono::duration<double, std::milli>(end - start).count() << ' ' << checksum << '\n';
}
struct Query { int type, a, b; U x, y; };

template<class DSU> void dsu_case(std::string_view scenario, int n, int q) {
	DSU uf(n);
	if (scenario != "dsu_mixed") {
		for (int width = 1; width < n; width *= 2)
			for (int v = 0; v + width < n; v += width * 2) uf.merge(v, v + width);
	}
	if (scenario == "dsu_groups") {
		auto start = Clock::now();
		auto groups = uf.groups();
		auto end = Clock::now();
		U checksum = groups.size();
		for (const auto &g : groups) for (int v : g) checksum = checksum * 1000003 + v;
		emit(start, end, checksum);
		return;
	}
	std::vector<Query> queries(q);
	for (auto &x : queries) x = {int(random_value() & 1), int(random_value() % n), int(random_value() % n), 0, 0};
	U checksum = 0;
	auto start = Clock::now();
	for (auto x : queries) {
		if (scenario == "dsu_queries" || x.type) checksum = checksum * 1000003 + uf.same(x.a, x.b);
		else uf.merge(x.a, x.b);
	}
	auto end = Clock::now();
	emit(start, end, checksum);
}

template<class BIT> void bit_case(std::string_view scenario, int n, int q) {
	BIT tree(n);
	std::vector<Query> queries(q);
	for (auto &x : queries) {
		int l = random_value() % n, r = random_value() % (n + 1);
		if (scenario == "fenwick_local") r = std::min(n, l + 1 + int(random_value() % 128));
		else if (scenario == "fenwick_prefix") l = 0;
		else if (l > r) std::swap(l, r);
		x = {int(random_value() & 1), l, r, random_value() % 201, random_value() % n};
	}
	U checksum = 0;
	auto start = Clock::now();
	for (auto x : queries) {
		if (x.type) checksum = checksum * 1000003 + U(tree.sum(x.a, x.b));
		else tree.add(int(x.y), std::int64_t(x.x) - 100);
	}
	auto end = Clock::now();
	emit(start, end, checksum);
}

using M = atcoder::modint998244353;
struct Value { M sum; int length; };
struct Action { M a, b; };
Value op(Value a, Value b) { return {a.sum + b.sum, a.length + b.length}; }
Value identity() { return {0, 0}; }
Value mapping(Action f, Value x) { return {f.a * x.sum + f.b * x.length, x.length}; }
Action composition(Action f, Action g) { return {f.a * g.a, f.a * g.b + f.b}; }
Action lazy_id() { return {1, 0}; }
using OursLazy = LazySegtree<Value, op, identity, Action, mapping, composition, lazy_id>;
using ACLLazy = atcoder::lazy_segtree<Value, op, identity, Action, mapping, composition, lazy_id>;

template<class Tree> void lazy_case(std::string_view scenario, int n, int q) {
	std::vector<Value> values(n);
	for (auto &x : values) x = {M(random_value() % 100), 1};
	Tree tree(values);
	std::vector<Query> queries(q);
	for (auto &x : queries) {
		int l = random_value() % n, r = random_value() % (n + 1);
		if (l > r) std::swap(l, r);
		int type = scenario == "lazy_read_heavy" ? random_value() % 10 != 0 : random_value() & 1;
		x = {type, l, r, random_value() % M::mod(), random_value() % M::mod()};
	}
	U checksum = 0;
	auto start = Clock::now();
	for (auto x : queries) {
		if (x.type) checksum = checksum * 1000003 + tree.prod(x.a, x.b).sum.val();
		else if (scenario == "lazy_point") tree.set(x.a == n ? n - 1 : x.a, {M(x.x), 1});
		else tree.apply(x.a, x.b, {M(x.x), M(x.y)});
	}
	auto end = Clock::now();
	emit(start, end, checksum);
}

template<class Graph> void scc_case(std::string_view scenario, int n) {
	Graph graph(n);
	if (scenario == "scc_cycle") {
		for (int v = 0; v < n; ++v) graph.add_edge(v, (v + 1) % n);
	} else if (scenario == "scc_path") {
		for (int v = 0; v + 1 < n; ++v) graph.add_edge(v, v + 1);
	} else {
		std::vector<int> perm(n);
		std::iota(perm.begin(), perm.end(), 0);
		for (int i = n - 1; i > 0; --i) std::swap(perm[i], perm[random_value() % (i + 1)]);
		for (int i = 0; i < n * 3; ++i) {
			int a = random_value() % n, b = random_value() % n;
			if (scenario == "scc_dag") {
				if (a == b) continue;
				if (a > b) std::swap(a, b);
				a = perm[a], b = perm[b];
			}
			graph.add_edge(a, b);
		}
	}
	auto start = Clock::now();
	auto groups = graph.scc();
	auto end = Clock::now();
	std::vector<int> representative(n);
	for (const auto &g : groups) for (int v : g) representative[v] = g.front();
	U checksum = groups.size();
	for (int v : representative) checksum = checksum * 1000003 + v;
	emit(start, end, checksum);
}

template<class SAT> void sat_case(std::string_view scenario, int n) {
	n /= 2;
	SAT solver(n);
	std::vector<bool> planted(n);
	for (int i = 0; i < n; ++i) planted[i] = random_value() & 1;
	std::vector<Query> clauses;
	for (int q = 0; q < n * 3; ++q) {
		int a = random_value() % n, b = random_value() % n;
		bool f = random_value() & 1, g = random_value() & 1;
		if (scenario == "sat_units") b = a, f = g = planted[a];
		else if (scenario == "sat_sat" && f != planted[a] && g != planted[b]) f = planted[a];
		solver.add_clause(a, f, b, g);
		clauses.push_back({0, a, b, U(f), U(g)});
	}
	auto start = Clock::now();
	bool sat = solver.satisfiable();
	auto end = Clock::now();
	if (sat) {
		auto answer = solver.answer();
		for (auto x : clauses) if (answer[x.a] != bool(x.x) && answer[x.b] != bool(x.y)) std::abort();
	}
	emit(start, end, sat);
}

int main(int argc, char **argv) {
	if (argc != 5) return 1;
	std::string_view implementation(argv[1]), scenario(argv[2]);
	int n = std::atoi(argv[3]), q = std::atoi(argv[4]);
	bool acl = implementation == "acl";
	if (scenario.starts_with("dsu")) {
		if (acl) dsu_case<atcoder::dsu>(scenario, n, q);
		else dsu_case<UnionFind>(scenario, n, q);
	} else if (scenario.starts_with("fenwick")) {
		if (acl) bit_case<atcoder::fenwick_tree<std::int64_t>>(scenario, n, q);
		else if (implementation == "unpadded") bit_case<FenwickTree<std::int64_t, false>>(scenario, n, q);
		else bit_case<FenwickTree<std::int64_t>>(scenario, n, q);
	} else if (scenario.starts_with("lazy")) {
		if (acl) lazy_case<ACLLazy>(scenario, n, q);
		else lazy_case<OursLazy>(scenario, n, q);
	} else if (scenario.starts_with("scc")) {
		if (acl) scc_case<atcoder::scc_graph>(scenario, n);
		else scc_case<SCCGraph>(scenario, n);
	} else if (scenario.starts_with("sat")) {
		if (acl) sat_case<atcoder::two_sat>(scenario, n);
		else sat_case<TwoSAT>(scenario, n);
	} else return 1;
}
