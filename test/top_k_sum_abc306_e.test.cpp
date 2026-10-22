#define PROBLEM "https://atcoder.jp/contests/abc306/tasks/abc306_e"
#include "../data_structure/top_k_sum.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, k, q;
	std::cin >> n >> k >> q;
	std::vector<long long> values(n);
	TopKSum<long long> top(k, values);
	while (q--) {
		int index;
		long long value;
		std::cin >> index >> value;
		--index;
		top.replace(values[index], value);
		values[index] = value;
		std::cout << top.sum() << '\n';
	}
}
