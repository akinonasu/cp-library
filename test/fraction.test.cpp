#define PROBLEM "https://judge.yosupo.jp/problem/rational_approximation"
#include "../math/fraction.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int t;
	std::cin >> t;
	while (t--) {
		std::int64_t n, x, y;
		std::cin >> n >> x >> y;
		Fraction target(x, y);
		x = target.numerator(), y = target.denominator();
		std::int64_t a = 0, b = 1, c = 1, d = 0;
		while (a + c <= n && b + d <= n) {
			Fraction middle(a + c, b + d);
			if (middle == target) { a = c = middle.numerator(); b = d = middle.denominator(); break; }
			if (middle < target) {
				std::int64_t k = (__int128_t(x) * b - __int128_t(y) * a) / (__int128_t(y) * c - __int128_t(x) * d);
				if (c) k = std::min(k, (n - a) / c);
				if (d) k = std::min(k, (n - b) / d);
				a += k * c, b += k * d;
				if (Fraction(a, b) == target) { c = a, d = b; break; }
			} else {
				std::int64_t k = (__int128_t(y) * c - __int128_t(x) * d) / (__int128_t(x) * b - __int128_t(y) * a);
				if (a) k = std::min(k, (n - c) / a);
				if (b) k = std::min(k, (n - d) / b);
				c += k * a, d += k * b;
				if (Fraction(c, d) == target) { a = c, b = d; break; }
			}
		}
		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
	}
}
