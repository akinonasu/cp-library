#define PROBLEM "https://judge.yosupo.jp/problem/predecessor_problem"
#include "data_structure/segtree.hpp"
#include "components/fast_io.hpp"
#include <string_view>
#include <vector>

int op(int a, int b) { return a | b; }
int e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::string_view bits;
	input.read(bits);
	std::vector<int> values(n);
	for (int i = 0; i < n; ++i) values[i] = bits[i] - '0';
	Segtree<int, op, e> tree(values);
	while (q--) {
		int type = 0, k = 0;
		input.read(type), input.read(k);
		if (type <= 1) tree.set(k, type == 0);
		else if (type == 2) output.println(tree.get(k));
		else if (type == 3) {
			int next = tree.max_right(k, [](int value) { return value == 0; });
			output.println(next == n ? -1 : next);
		} else output.println(tree.min_left(k + 1, [](int value) { return value == 0; }) - 1);
	}
}
