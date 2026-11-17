#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_3_C"
#include "integer_geometry/polygon.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n = 0;
	std::cin >> n;
	std::vector<integer_geometry::Point> polygon(n);
	for (auto& point : polygon) std::cin >> point;
	int q = 0;
	std::cin >> q;
	while (q--) {
		integer_geometry::Point point;
		std::cin >> point;
		std::cout << static_cast<int>(integer_geometry::contains(polygon, point)) + 1 << '\n';
	}
}
