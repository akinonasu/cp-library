#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_polynomials"
#include "fps/polynomial_gcd_light.hpp"
#include "fixtures/fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	light_fps a = read_light_fps(input, n), b = read_light_fps(input, m);
	auto gcd = light_fps_algorithms::polynomial_gcd(a, b);
	if (m > 1 && gcd.size() != 1) { output.println(-1); return 0; }
	auto result = light_fps_algorithms::polynomial_extended_gcd(std::move(a), b);
	auto inverse = result.x.divmod(b).second;
	output.println(inverse.size());
	write_light_fps(output, inverse);
}
