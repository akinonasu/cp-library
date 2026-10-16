#define PROBLEM "https://judge.yosupo.jp/problem/range_reverse_range_sum"
#include "../data_structure/lazy_reversible_treap.hpp"
#include "../components/fast_io.hpp"
#include <vector>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
long long mapping(long long f, long long x, int n) { return x + f * n; }
long long id() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	std::vector<long long> a(n);
	for (auto &x : a) input.read(x);
	LazyReversibleTreap<long long, op, e, long long, mapping, op, id, true> tree(a);
	while (q--) {
		int type = 0, l = 0, r = 0;
		input.read(type); input.read(l); input.read(r);
		if (type == 0) tree.reverse(l, r);
		else { output.write(tree.prod(l, r)); output.write('\n'); }
	}
}
