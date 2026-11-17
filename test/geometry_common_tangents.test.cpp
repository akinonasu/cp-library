#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_G"
#define ERROR "1e-5"
#include "geometry/circle.hpp"
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cmath>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	geometry::Circle a, b;
	std::cin >> a.center >> a.radius >> b.center >> b.radius;
	std::vector<geometry::FPoint> points;
	auto lines = geometry::common_tangents(a, b);
	for (auto line : *lines) {
		auto point = geometry::projection(line, a.center);
		point.x = std::round(point.x * 10000000000.L) / 10000000000.L;
		point.y = std::round(point.y * 10000000000.L) / 10000000000.L;
		points.push_back(point);
	}
	std::sort(points.begin(), points.end());
	for (auto point : points) std::cout << point << '\n';
}
