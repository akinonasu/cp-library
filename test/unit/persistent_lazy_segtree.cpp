#include "../../data_structure/segment_tree/persistent_lazy_segtree.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

struct F { long long a, b; };
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
long long mapping(F f, long long x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }
using Tree = PersistentLazySegtree<long long, op, e, F, mapping, composition, id>;
std::string concatenate(std::string a, std::string b) { return a + b; }
std::string empty_string() { return ""; }
struct Set { char c; };
std::string assign(Set f, std::string s, int n) { return f.c ? std::string(n, f.c) : s; }
Set compose_set(Set f, Set g) { return f.c ? f : g; }
Set id_set() { return {0}; }

int main() {
	std::mt19937 random(3456);
	for (int n : {0, 1, 2, 7, 32, 67}) {
		std::vector<long long> a(n);
		for (auto &x : a) x = random() % 10;
		Tree tree(a);
		std::vector<Tree::root_type> roots{tree.initial_root()};
		std::vector<std::vector<long long>> values{a};
		for (int t = 0; t < 5000; ++t) {
			unsigned k = random() % roots.size(), other = random() % roots.size();
			int l = random() % (n + 1), r = random() % (n + 1); if (l > r) std::swap(l, r);
			auto v = values[k]; auto root = roots[k];
			if (t % 3 == 0) {
				F f{static_cast<long long>(random() % 2), static_cast<long long>(random() % 7)};
				root = tree.apply(root, l, r, f);
				for (int i = l; i < r; ++i) v[i] = f.a * v[i] + f.b;
			} else if (t % 3 == 1 && n) {
				int p = random() % n; long long x = random() % 10;
				root = tree.set(root, p, x); v[p] = x;
			} else {
				root = tree.copy_range(root, roots[other], l, r);
				std::copy(values[other].begin() + l, values[other].begin() + r, v.begin() + l);
			}
			roots.push_back(root); values.push_back(v);
			k = random() % roots.size(); auto before = tree.node_count();
			assert(tree.prod(roots[k], l, r) == std::accumulate(values[k].begin() + l, values[k].begin() + r, 0LL));
			assert(tree.all_prod(roots[k]) == std::accumulate(values[k].begin(), values[k].end(), 0LL));
			for (int p = 0; p < n; ++p) assert(tree.get(roots[k], p) == values[k][p]);
			long long limit = random() % 100, sum = 0;
			int end = l, start = r;
			while (end < n && sum + values[k][end] <= limit) sum += values[k][end++];
			sum = 0;
			while (start && sum + values[k][start - 1] <= limit) sum += values[k][--start];
			auto pred = [limit](long long s) { return s <= limit; };
			assert(tree.max_right(roots[k], l, pred) == end && tree.min_left(roots[k], r, pred) == start);
			assert(tree.node_count() == before);
		}
	}
	Tree shared(1024);
	auto before = shared.node_count();
	auto point = shared.set(0, 100, 7);
	assert(shared.node_count() - before <= 11);
	before = shared.node_count();
	auto copied = shared.copy_range(0, point, 0, 512);
	assert(shared.node_count() - before <= 1 && shared.get(copied, 100) == 7);
	auto marked = shared.apply(point, 0, 1024, {2, 3});
	auto branch = shared.copy_range(marked, point, 80, 120);
	assert(shared.get(branch, 99) == 0 && shared.get(branch, 100) == 7 && shared.get(branch, 120) == 3);
	assert(shared.get(marked, 100) == 17 && shared.get(point, 100) == 7 && shared.all_prod(0) == 0);
	Tree huge(INT_MAX);
	auto a = huge.apply(0, 0, INT_MAX, {1, 2});
	auto b = huge.set(a, 123456, 99);
	auto c = huge.copy_range(b, 0, 0, 123456);
	assert(huge.all_prod(a) == 2LL * INT_MAX && huge.get(b, 123456) == 99 && huge.get(a, 123456) == 2);
	assert(huge.prod(c, 0, 123457) == 99 && huge.all_prod(0) == 0);
	PersistentLazySegtree<std::string, concatenate, empty_string, Set, assign, compose_set, id_set> text(std::vector<std::string>{"a", "b", "c", "d"});
	auto s = text.apply(text.initial_root(), 1, 4, {'x'});
	auto t = text.copy_range(s, text.initial_root(), 2, 4);
	assert(text.all_prod(t) == "axcd" && text.all_prod(s) == "axxx");
	auto p = [](const std::string &s) { return s.find("xc") == std::string::npos; };
	assert(text.max_right(t, 0, p) == 2 && text.min_left(t, 4, p) == 2);
}
