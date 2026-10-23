#include "../../data_structure/persistent/persistent_array.hpp"
#include "../../data_structure/persistent/persistent_union_find.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

struct Value { int x; explicit Value(int value) : x(value) {} };

template <int Log> void arrays() {
	std::mt19937 random(Log);
	for (int n : {1, 2, 7, 8, 9, 31, 64, 513, 1000}) {
		std::vector<int> a(n);
		std::iota(a.begin(), a.end(), 10);
		PersistentArray<int, Log> array(a, -1);
		std::vector<typename decltype(array)::root_type> roots{array.initial_root()};
		std::vector<std::vector<int>> values{a};
		for (int p = 0; p < n; ++p) assert(array.get(roots[0], p) == a[p]);
		for (int t = 0; t < 1000; ++t) {
			unsigned k = random() % roots.size();
			int p = random() % n, x = random() % 1000;
			auto next = values[k]; next[p] = x;
			if (t % 2) roots.push_back(array.set(roots[k], p, x));
			else {
				int q = random() % n, y = random() % 1000;
				next[q] = y;
				roots.push_back(array.set_pair(roots[k], p, x, q, y));
			}
			values.push_back(next);
			k = random() % roots.size();
			auto before = array.node_count();
			for (int j = 0; j < n; ++j) assert(array.get(roots[k], j) == values[k][j]);
			assert(array.node_count() == before);
		}
	}
}

int main() {
	arrays<1>(); arrays<3>(); arrays<5>();
	PersistentArray<std::string> sparse(INT_MAX, "empty");
	auto a = sparse.initial_root(), b = sparse.set(a, INT_MAX - 1, "last"), c = sparse.set(b, 0, "first");
	assert(sparse.get(a, INT_MAX - 1) == "empty" && sparse.get(b, 0) == "empty");
	assert(sparse.get(c, INT_MAX - 1) == "last" && sparse.get(c, 0) == "first");
	assert(PersistentArray<int>().empty());
	PersistentArray<Value> nondefault(std::vector<Value>{Value(1), Value(2)}, Value(-1));
	auto v = nondefault.set(nondefault.initial_root(), 0, Value(9));
	assert(nondefault.get(v, 0).x == 9 && nondefault.get(nondefault.initial_root(), 0).x == 1);
	auto pair = nondefault.set_pair(v, 0, Value(8), 1, Value(7));
	assert(nondefault.get(pair, 0).x == 8 && nondefault.get(pair, 1).x == 7 && nondefault.get(v, 0).x == 9);
	PersistentArray<int> shared(INT_MAX, -1);
	auto before = shared.node_count();
	auto separate = shared.set(shared.set(0, 123456, 1), 123456 + (1 << 24), 2);
	auto separate_nodes = shared.node_count() - before;
	before = shared.node_count();
	auto together = shared.set_pair(0, 123456, 1, 123456 + (1 << 24), 2);
	assert(shared.node_count() - before < separate_nodes);
	for (int p : {0, 123456, 123456 + (1 << 24), INT_MAX - 1}) assert(shared.get(separate, p) == shared.get(together, p));
	auto last = shared.set_pair(together, 123456, 3, 123456, 4);
	assert(shared.get(last, 123456) == 4 && shared.get(together, 123456) == 1);
	std::mt19937 random(998);
	for (int n : {1, 2, 15, 80}) {
		PersistentUnionFind uf(n);
		std::vector<PersistentUnionFind::root_type> roots{uf.initial_root()};
		std::vector<std::vector<int>> values(1, std::vector<int>(n));
		std::iota(values[0].begin(), values[0].end(), 0);
		for (int t = 0; t < 2000; ++t) {
			unsigned k = random() % roots.size();
			int a = random() % n, b = random() % n;
			auto next = values[k]; int from = next[b], to = next[a];
			for (int &x : next) if (x == from) x = to;
			roots.push_back(uf.merge(roots[k], a, b)); values.push_back(next);
			k = random() % roots.size(); auto before = uf.node_count();
			for (int i = 0; i < n; ++i) {
				assert(uf.size(roots[k], i) == std::count(values[k].begin(), values[k].end(), values[k][i]));
				for (int j = 0; j < n; ++j) assert(uf.same(roots[k], i, j) == (values[k][i] == values[k][j]));
			}
			auto groups = uf.groups(roots[k]);
			assert(static_cast<int>(groups.size()) == uf.component_count(roots[k]));
			assert(uf.node_count() == before);
		}
	}
	PersistentUnionFind zero;
	assert(zero.groups(zero.initial_root()).empty());
	assert(zero.component_count(zero.initial_root()) == 0);
}
