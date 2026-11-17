#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_E"
#define ERROR "1e-6"
#include "geometry/circle.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	geometry::Circle a, b;
	std::cin >> a.center >> a.radius >> b.center >> b.radius;
	auto points = *geometry::intersection(a, b);
	std::cout << points.front() << ' ' << points.back() << '\n';
}
