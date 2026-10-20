#include "data_structure/segment_tree/sortable_segtree.hpp"
#include <cstdlib>
#include <cstdio>
#include <numeric>
#include <random>
#include <string>

void require(bool ok, int line = __builtin_LINE()) { if (!ok) { std::fprintf(stderr, "line %d\n", line); std::abort(); } }
std::string concatenate(std::string a, const std::string &b) { return a + b; }
std::string empty_string() { return {}; }
long long add(long long a, long long b) { return a + b; }
long long zero() { return 0; }

template <class Key> void random_test(bool duplicates) {
	using Tree = SortableSegtree<Key, std::string, concatenate, empty_string>;
	std::mt19937_64 rng(393919);
	for (int n : {0, 1, 2, 3, 7, 31, 32, 33, 63, 64, 65, 100}) {
		std::vector<std::pair<Key, std::string>> a;
		for (int i = 0; i < n; ++i) a.emplace_back(Key(duplicates ? i % 4 : i), std::string(1, char('a' + i % 26)));
		std::shuffle(a.begin(), a.end(), rng);
		Tree tree(a);
		Key fresh = Key(n);
		for (int query = 0; query < 1000; ++query) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			int type = rng() % 5;
			if (type == 0 && n) {
				int p = rng() % n;
				std::string value(1, char('a' + rng() % 26));
				if (rng() & 1) tree.set(p, value), a[p].second = value;
				else {
					if (duplicates) fresh = Key(rng() % 4);
					else { fresh = Key(fresh + 1); if constexpr (sizeof(Key) <= 1) fresh = a[p].first; }
					tree.set(p, fresh, value); a[p] = {fresh, value};
				}
			} else if (type <= 2) {
				bool descending = type == 2;
				tree.sort(l, r, descending);
				if (!duplicates) std::sort(a.begin() + l, a.begin() + r, [&](const auto &x, const auto &y) { return descending ? x.first > y.first : x.first < y.first; });
				else {
					auto actual = tree.to_vector();
					for (int i = 0; i < l; ++i) require(actual[i] == a[i]);
					for (int i = r; i < n; ++i) require(actual[i] == a[i]);
					auto before = std::vector(a.begin() + l, a.begin() + r), after = std::vector(actual.begin() + l, actual.begin() + r);
					std::sort(before.begin(), before.end()); std::sort(after.begin(), after.end()); require(before == after);
					for (int i = l + 1; i < r; ++i) require(descending ? actual[i - 1].first >= actual[i].first : actual[i - 1].first <= actual[i].first);
					a = std::move(actual);
				}
			} else {
				std::string expected;
				for (int i = l; i < r; ++i) expected += a[i].second;
				const auto &constant = tree;
				require(constant.prod(l, r) == expected);
			}
			require(tree.size() == n && tree.empty() == !n && tree.to_vector() == a);
			std::string all;
			for (int i = 0; i < n; ++i) { require(tree.get(i) == a[i] && tree.key(i) == a[i].first); all += a[i].second; }
			require(tree.all_prod() == all);
			if (query % 211 == 0) {
				auto copy = tree;
				Tree moved(std::move(copy)); require(copy.empty() && copy.prod(0, 0).empty() && copy.all_prod().empty());
				copy.assign(a); require(copy.to_vector() == a);
				copy = std::move(moved); require(moved.empty() && copy.to_vector() == a);
				tree = copy;
			}
		}
		tree.clear(); require(tree.empty() && tree.to_vector().empty() && tree.all_prod().empty());
	}
}

struct Value {
	long long x;
	Value() = delete;
	explicit Value(long long x) : x(x) {}
};
Value combine(Value a, Value b) { return Value(a.x + b.x); }
Value identity() { return Value(0); }

int main() {
	random_test<std::int64_t>(false);
	random_test<std::uint64_t>(false);
	random_test<std::int8_t>(false);
	random_test<std::int64_t>(true);
	random_test<std::uint64_t>(true);
	using Tree = SortableSegtree<std::int64_t, long long, add, zero, true>;
	Tree wide({{INT64_MAX, 3}, {0, 4}, {INT64_MIN, 5}, {-1, 6}});
	wide.sort(0, 4); require(wide.key(0) == INT64_MIN && wide.key(3) == INT64_MAX && wide.prod(1, 3) == 10);
	wide.sort(0, 4, true); require(wide.key(3) == INT64_MIN && wide.all_prod() == 18);
	SortableSegtree<std::uint64_t, Value, combine, identity, true> values({{UINT64_MAX, Value(2)}, {0, Value(3)}});
	values.sort(0, 2); require(values.key(0) == 0 && values.prod(0, 2).x == 5);
	values.set(0, UINT64_MAX, Value(7)); values.sort(0, 2, true); require(values.all_prod().x == 9);
	int n = 100000;
	std::vector<std::pair<std::int64_t, long long>> a;
	for (int i = 0; i < n; ++i) a.emplace_back(n - i, i);
	Tree tree(a);
	tree.sort(0, n); auto bytes = tree.memory_usage();
	long long sum = n * (n - 1LL) / 2;
	for (int i = 0; i < 3000; ++i) {
		tree.sort(0, n, i & 1);
		require(tree.all_prod() == sum);
		sum += i - tree.get(i).second;
		tree.set(i, std::int64_t(n + i + 1), i);
		tree.sort(i, n - i, i & 1);
		require(tree.prod(i, i + 1) == tree.get(i).second);
	}
	require(tree.memory_usage() == bytes);
}
