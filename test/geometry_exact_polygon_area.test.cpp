#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_3_A"
#include "geometry/exact_polygon.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(1);
	int n = 0;
	std::cin >> n;
	std::vector<geometry::IPoint> polygon(n);
	for (auto& point : polygon) std::cin >> point;
	std::cout << static_cast<geometry::Real>(geometry::area_exact(polygon)) << '\n';
}
