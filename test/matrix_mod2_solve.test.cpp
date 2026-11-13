#define PROBLEM "https://judge.yosupo.jp/problem/system_of_linear_equations_mod_2"
#include "fixtures/matrix_mod2_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n); input.read(m);
	auto a = read_matrix_mod2(input, n, m);
	auto b = read_vector_mod2(input, n);
	auto solution = a.solve(b);
	if (!solution) { output.println(-1); return 0; }
	output.println(solution->basis.size());
	write_vector_mod2(output, solution->particular);
	for (const auto& vector : solution->basis) write_vector_mod2(output, vector);
}
