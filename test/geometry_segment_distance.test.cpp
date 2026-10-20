#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_2_D"
#define ERROR "1e-8"
#include "geometry/linear.hpp"
#include <iomanip>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q = 0; std::cin >> q;
	std::cout << std::fixed << std::setprecision(15);
	while (q--) {
		geometry::Segment<std::int64_t> s, t;
		std::cin >> s.a.x >> s.a.y >> s.b.x >> s.b.y >> t.a.x >> t.a.y >> t.b.x >> t.b.y;
		std::cout << geometry::distance(s, t) << '\n';
	}
}
