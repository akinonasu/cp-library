#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_2_C"
#define ERROR "1e-8"
#include "geometry/exact_linear.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	int q = 0;
	std::cin >> q;
	while (q--) {
		geometry::Line<std::int64_t> a, b;
		std::cin >> a.a >> a.b >> b.a >> b.b;
		std::cout << geometry::to_real(*geometry::intersection_exact(a, b)) << '\n';
	}
}
