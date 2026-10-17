#define PROBLEM "https://judge.yosupo.jp/problem/system_of_linear_equations"
#include "matrix_common.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n); in.read(m);
	auto a = read_matrix(in, n, m);
	std::vector<mint> b(n);
	for (auto &x : b) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	auto solution = a.solve(b);
	if (!solution) { out.println(-1); return 0; }
	out.println(solution->basis.size());
	auto write = [&](const auto &v) { for (int i = 0; i < m; ++i) { out.write(v[i].val()); out.write(i + 1 == m ? '\n' : ' '); } if (!m) out.write('\n'); };
	write(solution->particular);
	for (const auto &v : solution->basis) write(v);
}
