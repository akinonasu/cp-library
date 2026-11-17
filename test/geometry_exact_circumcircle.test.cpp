#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_C"
#define ERROR "1e-6"
#include "geometry/exact_circle.hpp"
#include <iomanip>
#include <iostream>
#include <cmath>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	geometry::IPoint a, b, c;
	std::cin >> a >> b >> c;
	auto circle = *geometry::circumcircle_exact(a, b, c);
	std::cout << geometry::to_real(circle.center) << ' ' << std::sqrt(static_cast<geometry::Real>(circle.radius_squared)) << '\n';
}
