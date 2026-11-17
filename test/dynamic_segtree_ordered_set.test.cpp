#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include "data_structure/dynamic_segtree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int op(int a, int b) { return a + b; }
int e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	constexpr std::int64_t limit = 1000000001;
	DynamicSegtree<int, op, e> tree(limit);
	tree.reserve(n + q);
	for (int i = 0; i < n; ++i) {
		int x = 0;
		input.read(x);
		tree.set(x, 1);
	}
	while (q--) {
		int type = 0, x = 0;
		input.read(type), input.read(x);
		if (type == 0) { if (!tree.get(x)) tree.multiply(x, 1); }
		else if (type == 1) tree.set(x, 0);
		else if (type == 2) output.println(tree.all_prod() < x ? -1 : tree.max_right(0, [x](int count) { return count < x; }));
		else if (type == 3) output.println(tree.prod(0, std::int64_t(x) + 1));
		else if (type == 4) output.println(tree.min_left(std::int64_t(x) + 1, [](int count) { return count == 0; }) - 1);
		else {
			auto next = tree.max_right(x, [](int count) { return count == 0; });
			output.println(next == limit ? -1 : next);
		}
	}
}
