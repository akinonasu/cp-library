#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_2_A"
#include "geometry/linear.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q = 0;
	std::cin >> q;
	while (q--) {
		geometry::Line<std::int64_t> a, b;
		std::cin >> a.a >> a.b >> b.a >> b.b;
		std::cout << (geometry::parallel(a, b) ? 2 : geometry::orthogonal(a, b) ? 1 : 0) << '\n';
	}
}
