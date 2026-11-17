#define PROBLEM "https://judge.yosupo.jp/problem/predecessor_problem"
#include "data_structure/fenwick_tree.hpp"
#include "components/fast_io.hpp"
#include <string_view>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::string_view bits;
	input.read(bits);
	std::vector<int> values(n);
	for (int i = 0; i < n; ++i) values[i] = bits[i] - '0';
	FenwickTree<int> tree(values);
	while (q--) {
		int type = 0, k = 0;
		input.read(type), input.read(k);
		if (type <= 1) tree.set(k, type == 0);
		else if (type == 2) output.println(tree.get(k));
		else if (type == 3) {
			int next = tree.upper_bound(tree.sum(k));
			output.println(next == n ? -1 : next);
		} else {
			int count = tree.sum(k + 1);
			output.println(count ? tree.lower_bound(count) : -1);
		}
	}
}
