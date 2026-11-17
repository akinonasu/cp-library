#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_2_A"
#include "integer_geometry/linear.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q = 0;
	std::cin >> q;
	while (q--) {
		integer_geometry::Line<> a, b;
		std::cin >> a.a >> a.b >> b.a >> b.b;
		std::cout << (integer_geometry::parallel(a, b) ? 2 : integer_geometry::orthogonal(a, b) ? 1 : 0) << '\n';
	}
}
