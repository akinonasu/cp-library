#include "data_structure/fenwick_tree.hpp"
#include "math/modint.hpp"
#include <cstdint>
#include <random>
#include <vector>

template<class T, bool Padded> void check() {
	std::mt19937_64 rng(671924);
	for (int n : {0, 1, 2, 3, 4, 5, 7, 8, 9, 1023, 1024, 1025, 2047, 2048, 2049}) {
		std::vector<T> a(n);
		for (auto &x : a) x = T(rng());
		FenwickTree<T, Padded> tree(a), reference(n);
		for (int i = 0; i < n; ++i) reference.add(i, a[i]);
		assert(tree.size() == n);
		for (int r = 0; r <= n; ++r) assert(tree.sum(r) == reference.sum(r));
		for (int q = 0; q < 1000; ++q) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			assert(tree.sum(l, r) == reference.sum(l, r));
			if (n) {
				int p = rng() % n;
				T x = T(rng());
				tree.add(p, x), reference.add(p, x);
			}
		}
	}
}

int main() {
	check<std::int64_t, true>();
	check<std::int64_t, false>();
	check<unsigned, true>();
	check<unsigned, false>();
	check<StaticModint<998244353>, true>();
	check<StaticModint<998244353>, false>();
#if defined(__SIZEOF_INT128__)
	check<__int128_t, true>();
	check<__int128_t, false>();
#endif
}
