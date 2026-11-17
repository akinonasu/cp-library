#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_3_B"
#include "geometry/polygon.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n = 0;
	std::cin >> n;
	std::vector<geometry::IPoint> polygon(n);
	for (auto& point : polygon) std::cin >> point;
	std::cout << geometry::is_convex(polygon) << '\n';
}
