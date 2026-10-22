#define PROBLEM "https://atcoder.jp/contests/abc281/tasks/abc281_e"
#include "../data_structure/top_k_sum.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, m, k;
	std::cin >> n >> m >> k;
	std::vector<long long> values(n);
	for (auto &value : values) std::cin >> value;
	SmallestKSum<long long> top(k, std::vector<long long>(values.begin(), values.begin() + m));
	std::cout << top.sum();
	for (int i = m; i < n; ++i) {
		top.replace(values[i - m], values[i]);
		std::cout << ' ' << top.sum();
	}
	std::cout << '\n';
}
