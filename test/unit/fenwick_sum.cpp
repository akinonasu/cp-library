#include "components/fenwick_sum.hpp"
#include <cstdlib>
#include <cstdint>
#include <numeric>
#include <random>

void require(bool value) { if (!value) std::abort(); }

struct Aggregate {
	std::int64_t count = 0;
	__int128_t sum = 0;
	Aggregate &operator+=(const Aggregate &other) { count += other.count; sum += other.sum; return *this; }
	Aggregate &operator-=(const Aggregate &other) { count -= other.count; sum -= other.sum; return *this; }
	bool operator==(const Aggregate &) const = default;
};

void rebuild() {
	FenwickSum<Aggregate> tree;
	for (int n : {0, 1, 2, 3, 8, 9, 1024, 1025, 1023, 3, 0, 33}) {
		std::vector<Aggregate> values(n);
		for (int i = 0; i < n; ++i) values[i] = {i % 7 - 3, (__int128_t(i % 11) - 5) * (__int128_t(1) << 80)};
		const auto saved = values;
		tree.build(values);
		require(tree.size() == n && values == saved);
		Aggregate prefix;
		for (int r = 0; r <= n; ++r) {
			require(tree.prefix(r) == prefix);
			if (r < n) prefix += values[r];
		}
		for (int l = 0; l < n; ++l) require(tree.prod(l, l + 1) == values[l]);
	}
}

int main() {
	rebuild();
	std::mt19937 random(765747);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 127, 128, 129, 255, 256, 257, 1023, 1024, 1025}) {
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
	FenwickSum<std::int64_t> extremes(std::vector<std::int64_t>{INT64_MIN, INT64_MAX});
	if (extremes.prod_as<__int128_t>(1, 2) != INT64_MAX) std::abort();
}
