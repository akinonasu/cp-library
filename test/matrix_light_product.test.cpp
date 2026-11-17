#define PROBLEM "https://judge.yosupo.jp/problem/matrix_product"
#include "fixtures/matrix_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0, k = 0;
	input.read(n), input.read(m), input.read(k);
	auto a = read_light_matrix(input, n, m), b = read_light_matrix(input, m, k);
	write_light_matrix(output, a * b);
}
