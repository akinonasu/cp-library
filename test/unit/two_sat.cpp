#include "graph/two_sat.hpp"
#include <array>
#include <random>

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
}
