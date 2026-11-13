#define PROBLEM "https://judge.yosupo.jp/problem/matrix_product_mod_2"
#include "fixtures/matrix_mod2_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0, k = 0;
	input.read(n); input.read(m); input.read(k);
	auto a = read_matrix_mod2(input, n, m);
	auto b = read_matrix_mod2(input, m, k);
	write_matrix_mod2(output, a * b);
}
