#define PROBLEM "https://judge.yosupo.jp/problem/pow_of_matrix"
#include "fixtures/matrix_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	std::uint64_t exponent = 0;
	input.read(n), input.read(exponent);
	write_light_matrix(output, read_light_matrix(input, n, n).pow(exponent));
}
