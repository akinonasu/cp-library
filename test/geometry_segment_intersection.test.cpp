#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_2_B"
#include "geometry/linear.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q = 0;
	std::cin >> q;
	while (q--) {
		geometry::Segment<std::int64_t> a, b;
		std::cin >> a.a >> a.b >> b.a >> b.b;
		std::cout << geometry::intersects(a, b) << '\n';
	}
}
