#define PROBLEM "https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series_large"
#include "fps/power_projection.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n;
	std::cin >> n;
	fps f(n);
	for (auto &x : f) std::cin >> x;
	if (n == 1) {
		std::cout << "0\n";
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
	std::cout << "0 ";
	for (auto x : h) std::cout << x << ' ';
	std::cout << '\n';
}
