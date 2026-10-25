#define PROBLEM "https://judge.yosupo.jp/problem/range_add_range_min"
#include "data_structure/lazy_segtree.hpp"
#include "components/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

using S = std::int64_t;
S op(S a, S b) { return std::min(a, b); }
S e() { return std::numeric_limits<S>::max(); }
S mapping(S f, S x) { return x == e() ? x : x + f; }
S composition(S f, S g) { return f + g; }
S id() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<S> values(n);
	for (auto &value : values) input.read(value);
	LazySegtree<S, op, e, S, mapping, composition, id> tree(values);
	while (q--) {
		int type = 0, l = 0, r = 0;
		input.read(type), input.read(l), input.read(r);
		if (type == 0) { S value = 0; input.read(value); tree.apply(l, r, value); }
		else output.println(tree.prod(l, r));
	}
}
