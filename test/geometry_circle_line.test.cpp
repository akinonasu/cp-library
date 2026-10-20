#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_D"
#define ERROR "1e-6"
#include "geometry/circle.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	geometry::Circle circle;
	std::cin >> circle.center.x >> circle.center.y >> circle.radius;
	int q = 0; std::cin >> q;
	std::cout << std::fixed << std::setprecision(15);
	while (q--) {
		geometry::Line<geometry::Real> line;
		std::cin >> line.a.x >> line.a.y >> line.b.x >> line.b.y;
		auto points = geometry::intersection(circle, line);
		auto a = points.front(), b = points.back();
		std::cout << a.x << ' ' << a.y << ' ' << b.x << ' ' << b.y << '\n';
	}
}
