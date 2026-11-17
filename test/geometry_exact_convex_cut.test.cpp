#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_4_C"
#define ERROR "1e-7"
#include "geometry/exact_polygon.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	int n = 0;
	std::cin >> n;
	std::vector<geometry::IPoint> polygon(n);
	for (auto& point : polygon) std::cin >> point;
	int q = 0;
	std::cin >> q;
	while (q--) {
		geometry::Line<std::int64_t> line;
		std::cin >> line.a >> line.b;
		std::cout << static_cast<geometry::Real>(geometry::area_exact(geometry::convex_cut_exact(polygon, line))) << '\n';
	}
}
