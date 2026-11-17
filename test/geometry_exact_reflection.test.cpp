#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_1_B"
#define ERROR "1e-8"
#include "geometry/exact_linear.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	geometry::Line<std::int64_t> line;
	std::cin >> line.a >> line.b;
	int q = 0;
	std::cin >> q;
	while (q--) {
		geometry::IPoint point;
		std::cin >> point;
		std::cout << geometry::to_real(geometry::reflection_exact(line, point)) << '\n';
	}
}
