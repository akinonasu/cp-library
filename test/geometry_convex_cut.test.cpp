#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_4_C"
#define ERROR "1e-7"
#include "geometry/polygon.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n = 0;
	std::cin >> n;
	std::vector<geometry::IPoint> polygon(n);
	for (auto &p : polygon) std::cin >> p;
	int q = 0;
	std::cin >> q;
	std::cout << std::fixed << std::setprecision(15);
	while (q--) {
		geometry::Line<std::int64_t> line;
		std::cin >> line.a >> line.b;
		std::cout << geometry::area(geometry::convex_cut(polygon, line)) << '\n';
	}
}
