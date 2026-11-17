#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_polynomials"
#include "fps/polynomial_gcd.hpp"
#include "fixtures/mint_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	fps a(n), b(m);
	read_mints(input, a), read_mints(input, b);
	auto result = polynomial_extended_gcd(std::move(a), b);
	if (m > 1 && result.gcd.size() != 1) { output.println(-1); return 0; }
	auto inverse = result.x.divmod(b).second;
	output.println(inverse.size());
	write_mints(output, inverse);
}
