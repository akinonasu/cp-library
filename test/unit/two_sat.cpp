#include "graph/two_sat.hpp"
#include <array>
#include <random>

using Clause = std::array<int, 4>;

void verify(TwoSAT &solver, const std::vector<Clause> &clauses, bool expected) {
	assert(solver.satisfiable() == expected);
	if (expected) {
		auto answer = solver.answer();
		assert(answer.size() == std::size_t(solver.size()));
		for (auto [u, f, v, g] : clauses) assert(answer[u] == bool(f) || answer[v] == bool(g));
	}
	assert(solver.satisfiable() == expected);
}

void check_reachability(int n, const std::vector<Clause> &clauses, int padding) {
	std::vector<std::vector<int>> graph(2 * n);
	TwoSAT solver(n + padding);
	solver.reserve(clauses.size());
	for (auto [u, f, v, g] : clauses) {
		solver.add_clause(u, f, v, g);
		graph[2 * u + !f].push_back(2 * v + g);
		graph[2 * v + !g].push_back(2 * u + f);
	}
	std::vector<bool> opposite(2 * n);
	for (int root = 0; root < 2 * n; ++root) {
		std::vector<bool> seen(2 * n);
		std::vector<int> queue{root};
		seen[root] = true;
		for (std::size_t head = 0; head < queue.size(); ++head)
			for (int to : graph[queue[head]]) if (!seen[to]) {
				seen[to] = true;
				queue.push_back(to);
			}
		opposite[root] = seen[root ^ 1];
	}
	bool expected = true;
	for (int i = 0; i < n; ++i) expected &= !opposite[2 * i] || !opposite[2 * i + 1];
	verify(solver, clauses, expected);
}

void check(int n, const std::vector<std::array<int, 4>>& clauses, int padding = 0) {
	TwoSAT solver(n + padding);
	solver.reserve(clauses.size());
	assert(solver.size() == n + padding);
	for (auto [u, f, v, g] : clauses) solver.add_clause(u, f, v, g);
	int witness = -1;
	for (int mask = 0; mask < (1 << n); ++mask) {
		bool valid = true;
		for (auto [u, f, v, g] : clauses) valid &= bool(mask >> u & 1) == bool(f) || bool(mask >> v & 1) == bool(g);
		if (valid) { witness = mask; break; }
	}
	assert(solver.satisfiable() == (witness != -1));
	if (witness != -1) {
		auto answer = solver.answer();
		assert(answer.size() == std::size_t(n + padding));
		for (auto [u, f, v, g] : clauses) assert(answer[u] == bool(f) || answer[v] == bool(g));
	}
	assert(solver.satisfiable() == (witness != -1));
}

int main() {
	check(0, {});
	{
		TwoSAT empty;
		empty.reserve(0);
		empty.reserve(100);
		assert(empty.size() == 0 && empty.satisfiable() && empty.answer().empty());
		assert(empty.satisfiable());
		auto copy = empty;
		assert(copy.satisfiable() && copy.answer().empty());
	}
	for (int mask = 0; mask < 4096; ++mask) {
		std::vector<std::array<int, 4>> clauses;
		for (int i = 0; i < 3; ++i) {
			int code = mask >> (i * 4);
			clauses.push_back({code & 1, code >> 1 & 1, code >> 2 & 1, code >> 3 & 1});
		}
		check(2, clauses);
		check(2, clauses, 1024);
	}
	std::mt19937 rng(71937);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = 1 + rng() % 10;
		std::vector<std::array<int, 4>> clauses;
		for (int i = 0, m = rng() % 50; i < m; ++i) clauses.push_back({int(rng() % n), int(rng() & 1), int(rng() % n), int(rng() & 1)});
		check(n, clauses);
		check(n, clauses, 1024);
	}
	for (int total : {1023, 1024, 1025}) {
		check(4, {{0, 1, 0, 1}, {0, 0, 1, 1}, {1, 0, 2, 1}, {2, 0, 3, 1}}, total - 4);
		check(4, {{0, 1, 1, 1}, {0, 1, 1, 0}, {0, 0, 1, 1}, {0, 0, 1, 0}}, total - 4);
		for (int trial = 0; trial < 30; ++trial) {
			std::vector<Clause> clauses;
			for (int i = 0; i < 20; ++i) clauses.push_back({int(rng() % 5), int(rng() & 1), int(rng() % 5), int(rng() & 1)});
			check(5, clauses, total - 5);
		}
	}
	for (int trial = 0; trial < 150; ++trial) {
		int n = 20 + rng() % 31;
		std::vector<Clause> clauses;
		for (int i = 0, m = rng() % (5 * n); i < m; ++i)
			clauses.push_back({int(rng() % n), int(rng() & 1), int(rng() % n), int(rng() & 1)});
		check_reachability(n, clauses, trial % 2 ? 1024 : 0);
	}
	TwoSAT fallback(4096);
	std::vector<std::array<int, 4>> many;
	auto add = [&](int u, bool f, int v, bool g) {
		fallback.add_clause(u, f, v, g);
		many.push_back({u, f, v, g});
	};
	for (int i = 0; i < 1024; ++i) {
		add(i, true, 1024, false);
		add(i, false, 3074, true);
	}
	for (int i = 1024; i < 3072; ++i) add(i, true, i + 1, false);
	add(3072, true, 3073, true);
	add(3072, true, 3073, false);
	assert(fallback.satisfiable());
	auto solution = fallback.answer();
	for (auto [u, f, v, g] : many) assert(solution[u] == bool(f) || solution[v] == bool(g));
	auto copied = fallback;
	assert(copied.satisfiable());
	copied.add_clause(0, false, 0, false);
	assert(!copied.satisfiable());
	assert(fallback.satisfiable());
	TwoSAT changed(1);
	assert(changed.satisfiable());
	changed.add_clause(0, true, 0, true);
	assert(changed.satisfiable() && changed.answer()[0]);
	changed.add_clause(0, false, 0, false);
	assert(!changed.satisfiable());
	assert(!changed.satisfiable());
	changed.add_clause(0, true, 0, false);
	assert(!changed.satisfiable());
	changed.reserve(100);
	assert(!changed.satisfiable());
	{
		TwoSAT original(2);
		assert(original.satisfiable());
		bool before = original.answer()[0];
		auto copy = original;
		original.add_clause(0, !before, 0, !before);
		copy.add_clause(0, before, 0, before);
		assert(original.satisfiable() && original.answer()[0] == !before);
		assert(copy.satisfiable() && copy.answer()[0] == before);
		original.add_clause(0, before, 0, before);
		assert(!original.satisfiable() && !original.satisfiable());
		auto impossible = original;
		impossible.add_clause(1, true, 1, true);
		assert(!impossible.satisfiable());
		copy.add_clause(1, false, 1, false);
		assert(copy.satisfiable() && !copy.answer()[1]);
		assert(!original.satisfiable());
	}
	{
		TwoSAT units(2048);
		std::vector<Clause> clauses;
		for (int repeat = 0; repeat < 5; ++repeat) for (int i = 0; i < units.size(); ++i) {
			units.add_clause(i, i & 1, i, i & 1);
			clauses.push_back({i, i & 1, i, i & 1});
		}
		verify(units, clauses, true);
		units.add_clause(1024, true, 1024, true);
		clauses.push_back({1024, 1, 1024, 1});
		verify(units, clauses, false);
	}
	{
		TwoSAT tautologies(2048);
		for (int repeat = 0; repeat < 20; ++repeat)
			for (int i = 0; i < tautologies.size(); ++i) tautologies.add_clause(i, true, i, false);
		assert(tautologies.satisfiable());
		tautologies.add_clause(17, true, 17, true);
		assert(tautologies.satisfiable() && tautologies.answer()[17]);
		tautologies.add_clause(17, true, 17, false);
		assert(tautologies.satisfiable() && tautologies.answer()[17]);
	}
	for (unsigned seed : {17U, 293U, 50177U}) {
		int n = 1200, m = 12000;
		std::mt19937 generator(seed);
		std::vector<bool> witness(n);
		for (int i = 0; i < n; ++i) witness[i] = generator() & 1;
		TwoSAT dense(n);
		dense.reserve(m + 4);
		std::vector<Clause> clauses;
		for (int i = 0; i < m; ++i) {
			int u = generator() % n, v = generator() % (n - 1);
			if (v >= u) ++v;
			bool f = generator() & 1, g = generator() & 1;
			if (witness[u] != f && witness[v] != g) {
				if (generator() & 1) f = witness[u];
				else g = witness[v];
			}
			dense.add_clause(u, f, v, g);
			clauses.push_back({u, f, v, g});
		}
		for (auto [u, f, v, g] : clauses) assert(witness[u] == bool(f) || witness[v] == bool(g));
		verify(dense, clauses, true);
		auto copy = dense;
		for (Clause clause : {Clause{0, 1, 1, 1}, Clause{0, 1, 1, 0}, Clause{0, 0, 2, 1}, Clause{0, 0, 2, 0}})
			dense.add_clause(clause[0], clause[1], clause[2], clause[3]);
		assert(!dense.satisfiable() && !dense.satisfiable());
		verify(copy, clauses, true);
	}
	{
		int n = 100000, roots = 16, last = n - 3;
		TwoSAT deep(n);
		std::vector<Clause> clauses;
		auto append = [&](int u, bool f, int v, bool g) {
			deep.add_clause(u, f, v, g);
			clauses.push_back({u, f, v, g});
		};
		for (int i = 0; i < roots; ++i) {
			append(i, true, roots, false);
			append(i, false, n - 1, true);
		}
		for (int i = roots; i < last; ++i) append(i, true, i + 1, false);
		append(last, true, n - 2, true);
		append(last, true, n - 2, false);
		verify(deep, clauses, true);
		deep.add_clause(0, false, 0, false);
		clauses.push_back({0, 0, 0, 0});
		verify(deep, clauses, false);
	}
	{
		int n = 100000;
		TwoSAT cycle(n);
		std::vector<Clause> clauses;
		for (int i = 0; i < n; ++i) {
			int j = i + 1 == n ? 0 : i + 1;
			cycle.add_clause(i, false, j, true);
			clauses.push_back({i, 0, j, 1});
		}
		verify(cycle, clauses, true);
		cycle.add_clause(0, true, 0, true);
		clauses.push_back({0, 1, 0, 1});
		verify(cycle, clauses, true);
		cycle.add_clause(n - 1, false, n - 1, false);
		clauses.push_back({n - 1, 0, n - 1, 0});
		verify(cycle, clauses, false);
	}
}
