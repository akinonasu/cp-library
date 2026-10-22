#define PROBLEM "https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series_large"
#include "fps/power_projection.hpp"
#include "fixtures/fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	auto f = read_fps(input, n);
	if (n == 1) {
		output.println(0);
		return 0;
	}
	auto p = power_projection(f, n - 1);
	fps inv(n), h(n - 1);
	inv[1] = 1;
	for (int i = 2; i < n; ++i) inv[i] = -mint(mint::mod() / i) * inv[mint::mod() % i];
	for (int i = 1; i < n; ++i) h[n - 1 - i] = p[i] * inv[i];
	h *= h[0].inv();
	h = h.log(n - 1);
	h *= -inv[n - 1];
	h = h.exp(n - 1);
	h *= f[1].inv();
	output.write('0'); output.write(' ');
	write_fps(output, h);
}
