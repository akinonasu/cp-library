#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_2_E"
#include "data_structure/dynamic_lazy_segtree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

struct Sum { std::int64_t value, length; };
Sum op(Sum a, Sum b) { return {a.value + b.value, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(std::int64_t f, Sum x) { return {x.value + f * x.length, x.length}; }
std::int64_t composition(std::int64_t f, std::int64_t g) { return f + g; }
std::int64_t id() { return 0; }
Sum initial(std::int64_t l, std::int64_t r) { return {0, r - l}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	DynamicLazySegtree<Sum, op, e, std::int64_t, mapping, composition, id, initial> tree(n);
	const auto &query = tree;
	while (q--) {
		int type = 0, l = 0;
		input.read(type), input.read(l);
		--l;
		if (type == 0) {
			int r = 0;
			std::int64_t value = 0;
			input.read(r), input.read(value);
			tree.apply(l, r, value);
		} else output.println(query.get(l).value);
	}
}
