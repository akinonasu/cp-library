#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_polynomials"
#include "fps/polynomial_gcd_light.hpp"
#include "fixtures/fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	light_fps a = read_light_fps(input, n), b = read_light_fps(input, m);
	auto result = light_fps_algorithms::polynomial_inv_mod(std::move(a), std::move(b));
	if (!result) { output.println(-1); return 0; }
	output.println(result->size());
	write_light_fps(output, *result);
}
