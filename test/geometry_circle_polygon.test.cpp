#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_H"
#define ERROR "1e-7"
#include "geometry/circle.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n = 0;
	geometry::Circle circle;
	std::cin >> n >> circle.radius;
	std::vector<geometry::IPoint> polygon(n);
	for (auto &p : polygon) std::cin >> p;
	std::cout << std::fixed << std::setprecision(15) << geometry::intersection_area(circle, polygon) << '\n';
}
