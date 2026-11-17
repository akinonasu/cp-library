#define PROBLEM "https://judge.yosupo.jp/problem/characteristic_polynomial"
#include "fixtures/matrix_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	auto polynomial = read_light_matrix(input, n, n).characteristic_polynomial();
	for (int i = 0; i <= n; ++i) {
		output.write(polynomial[i].val());
		output.write(i == n ? '\n' : ' ');
	}
}
