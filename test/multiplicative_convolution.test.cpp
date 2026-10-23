#define PROBLEM "https://judge.yosupo.jp/problem/mul_modp_convolution"
#include "convolution/multiplicative_convolution.hpp"
#include "test/fixtures/mint_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<mint> a(n), b(n);
	read_mints(input, a);
	read_mints(input, b);
	auto result = multiplicative_convolution(a, b);
	write_mints(output, result);
}
