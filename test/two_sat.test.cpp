#define PROBLEM "https://judge.yosupo.jp/problem/two_sat"
#include "graph/two_sat.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string token;
	in.read(token), in.read(token);
	int n = 0, m = 0;
	in.read(n), in.read(m);
	TwoSAT solver(n);
	solver.reserve(m);
	while (m--) {
		int a = 0, b = 0, zero = 0;
		in.read(a), in.read(b), in.read(zero);
		solver.add_clause((a > 0 ? a : -a) - 1, a > 0, (b > 0 ? b : -b) - 1, b > 0);
	}
	if (!solver.satisfiable()) { out.println("s UNSATISFIABLE"); return 0; }
	out.println("s SATISFIABLE");
	out.write('v');
	auto answer = solver.answer();
	for (int i = 0; i < n; ++i) { out.write(' '); out.write(answer[i] ? i + 1 : -i - 1); }
	out.println(" 0");
}
