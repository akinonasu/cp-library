#define PROBLEM "https://judge.yosupo.jp/problem/inverse_matrix"
#include "fixtures/matrix_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	auto inverse = read_light_matrix(input, n, n).inverse();
	if (inverse) write_light_matrix(output, *inverse);
	else output.println(-1);
}
