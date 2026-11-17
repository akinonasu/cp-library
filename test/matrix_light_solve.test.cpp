#define PROBLEM "https://judge.yosupo.jp/problem/system_of_linear_equations"
#include "fixtures/matrix_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	auto a = read_light_matrix(input, n, m);
	std::vector<mint> b(n);
	for (auto &value : b) { unsigned x = 0; input.read(x); value = mint::raw(x); }
	auto solution = a.solve(b);
	if (!solution) { output.println(-1); return 0; }
	output.println(solution->basis.size());
	auto write = [&](const auto &v) {
		for (int i = 0; i < m; ++i) { output.write(v[i].val()); output.write(i + 1 == m ? '\n' : ' '); }
		if (!m) output.write('\n');
	};
	write(solution->particular);
	for (const auto &v : solution->basis) write(v);
}
