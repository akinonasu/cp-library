#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"
#include "../convolution/convolution_light.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	std::cin >> n >> m;
	std::vector<mint> a(n), b(m);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : light_convolution::convolution(a, b)) std::cout << x << ' ';
	std::cout << '\n';
}
