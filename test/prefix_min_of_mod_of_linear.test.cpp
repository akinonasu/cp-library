#define PROBLEM "https://judge.yosupo.jp/problem/min_of_mod_of_linear"
#include "../math/floor_sum/prefix_min_of_mod_of_linear.hpp"
#include <cstdint>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int t;
	std::cin >> t;
	while (t--) {
		std::uint64_t n, m, a, b, answer = 0;
		std::cin >> n >> m >> a >> b;
		prefix_min_of_mod_of_linear(n, m, a, b, [&](ModLinearPrefixMinSegment part) {
			answer = part.y - part.dy * (part.count - 1);
		});
		std::cout << answer << '\n';
	}
}
