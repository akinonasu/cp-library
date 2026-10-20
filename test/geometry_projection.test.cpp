#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_1_A"
#define ERROR "1e-8"
#include "geometry/linear.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	geometry::Line<std::int64_t> line;
	std::cin >> line.a.x >> line.a.y >> line.b.x >> line.b.y;
	int q = 0; std::cin >> q;
	std::cout << std::fixed << std::setprecision(15);
	while (q--) {
		geometry::IPoint p;
		std::cin >> p.x >> p.y;
		auto x = geometry::projection(line, p);
		std::cout << x.x << ' ' << x.y << '\n';
	}
}
