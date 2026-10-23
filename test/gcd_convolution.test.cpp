#define PROBLEM "https://judge.yosupo.jp/problem/gcd_convolution"
#include "convolution/gcd_lcm_convolution.hpp"
#include "test/fixtures/mint_io.hpp"
#include <span>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<mint> a(n + 1), b(n + 1);
	read_mints(input, std::span(a).subspan(1));
	read_mints(input, std::span(b).subspan(1));
	auto result = gcd_convolution(a, b);
	write_mints(output, std::span(result).subspan(1));
}
