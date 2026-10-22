#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_polynomials"
#include "fps/polynomial_gcd_light.hpp"
#include "fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	light_fps a = read_light_fps(input, n), b = read_light_fps(input, m);
	auto result = light_fps_algorithms::polynomial_extended_gcd(std::move(a), b);
	if (m > 1 && result.gcd.size() != 1) { output.println(-1); return 0; }
	auto inverse = result.x.divmod(b).second;
	output.println(inverse.size());
	write_light_fps(output, inverse);
}
