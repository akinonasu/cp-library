#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_C"
#define ERROR "1e-6"
#include "geometry/exact_linear.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	geometry::IPoint a, b, c;
	std::cin >> a >> b >> c;
	auto first = *geometry::perpendicular_bisector_exact(a, b);
	auto second = *geometry::perpendicular_bisector_exact(b, c);
	auto center = *geometry::intersection_exact(first, second);
	auto squared = geometry::distance_squared_exact(center, a);
	std::cout << geometry::to_real(center) << ' ' << std::sqrt(static_cast<geometry::Real>(squared)) << '\n';
}
