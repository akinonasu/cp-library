#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_polynomials"
#include "fps/polynomial_gcd.hpp"
#include "fixtures/fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	fps a = read_fps(input, n), b = read_fps(input, m);
	auto result = polynomial_inv_mod(std::move(a), std::move(b));
	if (!result) { output.println(-1); return 0; }
	output.println(result->size());
	write_fps(output, *result);
}
