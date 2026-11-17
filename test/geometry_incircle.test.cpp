#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_B"
#define ERROR "1e-6"
#include "geometry/circle.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	geometry::IPoint a, b, c;
	std::cin >> a >> b >> c;
	auto circle = *geometry::incircle(a, b, c);
	std::cout << circle.center << ' ' << circle.radius << '\n';
}
