#include "graph/two_sat.hpp"
#include <array>
#include <random>

void check(int n, const std::vector<std::array<int, 4>>& clauses) {
	TwoSAT solver(n);
	solver.reserve(clauses.size());
	assert(solver.size() == n);
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
		assert(answer.size() == std::size_t(n));
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
	}
	std::mt19937 rng(71937);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = 1 + rng() % 10;
		std::vector<std::array<int, 4>> clauses;
		for (int i = 0, m = rng() % 50; i < m; ++i) clauses.push_back({int(rng() % n), int(rng() & 1), int(rng() % n), int(rng() & 1)});
		check(n, clauses);
	}
	TwoSAT changed(1);
	assert(changed.satisfiable());
	changed.add_clause(0, true, 0, true);
	assert(changed.satisfiable() && changed.answer()[0]);
	changed.add_clause(0, false, 0, false);
	assert(!changed.satisfiable());
}
