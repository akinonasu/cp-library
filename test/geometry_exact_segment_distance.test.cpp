#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_2_D"
#define ERROR "1e-8"
#include "geometry/exact_linear.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	int q = 0;
	std::cin >> q;
	while (q--) {
		geometry::Segment<std::int64_t> a, b;
		std::cin >> a.a >> a.b >> b.a >> b.b;
		auto squared = geometry::distance_squared_exact(a, b);
		std::cout << std::sqrt(static_cast<geometry::Real>(squared)) << '\n';
	}
}
