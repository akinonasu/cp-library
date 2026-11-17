#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_F"
#define ERROR "1e-5"
#include "geometry/circle.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	geometry::IPoint point;
	geometry::Circle circle;
	std::cin >> point >> circle.center >> circle.radius;
	for (auto contact : geometry::tangent_points(circle, point)) std::cout << contact << '\n';
}
