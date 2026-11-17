#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_1_C"
#include "integer_geometry/point.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	integer_geometry::Point a, b;
	std::cin >> a >> b;
	int q = 0;
	std::cin >> q;
	const char* names[] = {"ONLINE_FRONT", "CLOCKWISE", "ON_SEGMENT", "COUNTER_CLOCKWISE", "ONLINE_BACK"};
	while (q--) {
		integer_geometry::Point point;
		std::cin >> point;
		std::cout << names[integer_geometry::ccw(a, b, point) + 2] << '\n';
	}
}
