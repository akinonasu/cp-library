#define PROBLEM "https://judge.yosupo.jp/problem/inverse_matrix_mod_2"
#include "fixtures/matrix_mod2_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	auto inverse = read_matrix_mod2(input, n, n).inverse();
	if (inverse) write_matrix_mod2(output, *inverse);
	else output.println(-1);
}
