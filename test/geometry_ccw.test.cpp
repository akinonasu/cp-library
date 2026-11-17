#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_1_C"
#include "geometry/point.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	geometry::IPoint a, b;
	std::cin >> a >> b;
	int q = 0;
	std::cin >> q;
	const char* names[] = {"ONLINE_FRONT", "CLOCKWISE", "ON_SEGMENT", "COUNTER_CLOCKWISE", "ONLINE_BACK"};
	while (q--) {
		geometry::IPoint point;
		std::cin >> point;
		std::cout << names[geometry::ccw(a, b, point) + 2] << '\n';
	}
}
