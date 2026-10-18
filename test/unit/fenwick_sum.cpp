#include "components/fenwick_sum.hpp"
#include <cstdlib>
#include <cstdint>
#include <numeric>
#include <random>

void require(bool value) { if (!value) std::abort(); }

int main() {
	std::mt19937 random(765747);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 127, 129}) {
		std::vector<std::int64_t> a(n);
		for (auto &x : a) x = int(random() % 100) - 50;
		FenwickSum<std::int64_t> tree(a);
		for (int repeat = 0; repeat < 1000; ++repeat) {
			if (n) {
				int p = random() % n, q = random() % n;
				std::int64_t x = int(random() % 100) - 50, y = int(random() % 100) - 50;
				if (repeat & 1) tree.add(p, x), a[p] += x;
				else tree.add_pair(p, x, q, y), a[p] += x, a[q] += y;
			}
			int l = random() % (n + 1), r = random() % (n + 1);
			if (l > r) std::swap(l, r);
			require(tree.prod(l, r) == std::accumulate(a.begin() + l, a.begin() + r, std::int64_t(0)));
			require(tree.prefix(r) == std::accumulate(a.begin(), a.begin() + r, std::int64_t(0)));
		}
		tree.build(a);
		require(tree.prod(0, n) == std::accumulate(a.begin(), a.end(), std::int64_t(0)));
	}
}
