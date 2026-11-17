#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_3_A"
#include "integer_geometry/polygon.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n = 0;
	std::cin >> n;
	std::vector<integer_geometry::Point> polygon(n);
	for (auto& point : polygon) std::cin >> point;
	auto twice = integer_geometry::doubled_area(polygon);
	std::cout << static_cast<std::int64_t>(twice / 2) << (twice % 2 ? ".5\n" : ".0\n");
}
