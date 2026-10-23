#include "data_structure/persistent/persistent_segtree.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <climits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

long long add(long long a, long long b) { return a + b; }
long long zero() { return 0; }
using Tree = PersistentSegtree<long long, add, zero>;
std::string concatenate(std::string a, const std::string &b) { return a + b; }
std::string blank() { return {}; }
using Strings = PersistentSegtree<std::string, concatenate, blank>;

struct Value {
	long long x;
	Value() = delete;
	explicit Value(long long x) : x(x) {}
};
Value sum(Value a, Value b) { return Value(a.x + b.x); }
Value identity() { return Value(0); }

int main() {
	Tree empty;
	auto empty_root = empty.initial_root();
	assert(empty.empty() && empty.size() == 0 && empty.node_count() == 0);
	assert(empty.prod(empty_root, 0, 0) == 0 && empty.all_prod(empty_root) == 0);
	assert(empty.copy_range(empty_root, empty_root, 0, 0) == empty_root);
	assert(empty.max_right(empty_root, 0, [](long long x) { return x == 0; }) == 0);
	assert(empty.min_left(empty_root, 0, [](long long x) { return x == 0; }) == 0);
	Tree empty_vector(std::vector<long long>{});
	assert(empty_vector.all_prod(empty_vector.initial_root()) == 0);
	std::mt19937 random(20261021);
	for (int n : {1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 99, 255, 256, 257, 1025}) {
		for (bool sparse : {false, true}) {
			std::vector<long long> a(n);
			if (!sparse) for (auto &x : a) x = random() % 10;
			Tree seg = sparse ? Tree(n) : Tree(a);
			assert(seg.size() == n && !seg.empty());
			std::size_t built = 0;
			for (unsigned count = n; count; count = count == 1 ? 0 : (count + 3) / 4) built += count;
			assert(seg.node_count() == (sparse ? 0u : built));
			std::vector<Tree::root_type> roots{seg.initial_root()};
			std::vector<std::vector<long long>> versions{a};
			for (int t = 0; t < 800; ++t) {
				unsigned k = random() % roots.size(), other = random() % roots.size();
				int p = random() % n, l = random() % (n + 1), r = random() % (n + 1);
				if (r < l) std::swap(l, r);
				long long x = random() % 15;
				auto next = versions[k];
				auto before = seg.node_count();
				Tree::root_type root;
				if (t % 3 == 0) { root = seg.set(roots[k], p, x); next[p] = x; }
				else if (t % 3 == 1) { root = seg.multiply(roots[k], p, x); next[p] += x; }
				else {
					root = seg.copy_range(roots[k], roots[other], l, r);
					std::copy(versions[other].begin() + l, versions[other].begin() + r, next.begin() + l);
					if (l == r || k == other) assert(root == roots[k] && seg.node_count() == before);
				}
				assert(seg.node_count() - before <= std::size_t(2) * std::bit_width(unsigned(n - 1)) + 1);
				roots.push_back(root);
				versions.push_back(std::move(next));
				before = seg.node_count();
				const Tree &read = seg;
				for (unsigned query : {k, other, unsigned(roots.size() - 1)}) {
					const auto &v = versions[query];
					assert(read.get(roots[query], p) == v[p]);
					assert(read.prod(roots[query], l, r) == std::accumulate(v.begin() + l, v.begin() + r, 0LL));
					assert(read.all_prod(roots[query]) == std::accumulate(v.begin(), v.end(), 0LL));
					long long limit = random() % 100, total = 0;
					int end = l, start = r;
					while (end < n && total + v[end] <= limit) total += v[end++];
					total = 0;
					while (start && total + v[start - 1] <= limit) total += v[--start];
					auto predicate = [limit](long long value) { return value <= limit; };
					assert(read.max_right(roots[query], l, predicate) == end);
					assert(read.min_left(roots[query], r, predicate) == start);
				}
				assert(seg.node_count() == before);
			}
			auto count = seg.node_count();
			seg.reserve(count + 1000);
			assert(seg.node_count() == count && seg.memory_usage() >= (count + 1001) * sizeof(long long));
			auto copy = seg;
			auto changed = copy.set(roots[0], 0, 123456);
			assert(copy.get(changed, 0) == 123456 && seg.get(roots[0], 0) == a[0]);
			Tree moved(std::move(copy));
			assert(moved.get(changed, 0) == 123456);
			for (int p = 0; p < n; ++p) assert(seg.get(roots[0], p) == a[p]);
		}
	}
	for (int n : {1, 3, 4, 5, 15, 16, 17, 35, 63, 64, 65}) {
		Strings seg(n);
		std::vector<Strings::root_type> roots{seg.initial_root()};
		std::vector<std::vector<std::string>> versions{std::vector<std::string>(n)};
		for (int t = 0; t < 500; ++t) {
			unsigned k = random() % roots.size(), other = random() % roots.size();
			int p = random() % n, l = random() % (n + 1), r = random() % (n + 1);
			if (r < l) std::swap(l, r);
			auto next = versions[k];
			std::string value(1, char('a' + random() % 3));
			Strings::root_type root;
			if (t % 3 == 0) { root = seg.set(roots[k], p, value); next[p] = value; }
			else if (t % 3 == 1) { root = seg.multiply(roots[k], p, value); next[p] += value; }
			else {
				root = seg.copy_range(roots[k], roots[other], l, r);
				std::copy(versions[other].begin() + l, versions[other].begin() + r, next.begin() + l);
			}
			roots.push_back(root); versions.push_back(next);
			assert(seg.prod(root, l, r) == std::accumulate(next.begin() + l, next.begin() + r, std::string{}));
			auto f = [](const std::string &s) { return s.find("ab") == std::string::npos; };
			int end = l, start = r;
			std::string total;
			while (end < n && f(total + next[end])) total += next[end++];
			total.clear();
			while (start && f(next[start - 1] + total)) total = next[--start] + total;
			assert(seg.max_right(root, l, f) == end && seg.min_left(root, r, f) == start);
		}
	}
	Tree shared(257);
	auto base = shared.set(0, 200, 5);
	auto changed = shared.set(base, 17, 9);
	auto before_restore = shared.node_count();
	auto restored = shared.copy_range(changed, base, 17, 18);
	assert(restored == base && shared.node_count() == before_restore);
	assert(shared.get(changed, 17) == 9 && shared.get(base, 17) == 0);
	Tree huge(INT_MAX);
	auto h0 = huge.initial_root(), h1 = huge.set(h0, INT_MAX - 1, 7), h2 = huge.set(h1, 0, 11);
	assert(huge.prod(h2, 0, INT_MAX) == 18 && huge.all_prod(h0) == 0);
	assert(huge.max_right(h2, 0, [](long long x) { return x <= 11; }) == INT_MAX - 1);
	assert(huge.min_left(h2, INT_MAX, [](long long x) { return x <= 7; }) == 1);
	auto h3 = huge.copy_range(h2, h0, 0, INT_MAX - 1);
	assert(huge.all_prod(h3) == 7 && huge.all_prod(h2) == 18);
	for (int n : {INT_MAX, INT_MAX - 1, 1 << 30, (1 << 30) + 1, (1 << 29) + 1}) {
		Tree seg(n);
		auto initial = seg.initial_root();
		auto left = seg.set(initial, 0, 3), right = seg.set(initial, n - 1, 5);
		auto combined = seg.copy_range(left, right, n - 1, n);
		assert(seg.all_prod(combined) == 8 && seg.all_prod(left) == 3 && seg.all_prod(right) == 5);
		assert(seg.prod(combined, 0, n - 1) == 3 && seg.get(combined, n - 1) == 5);
		assert(seg.max_right(combined, 1, [](long long x) { return x <= 4; }) == n - 1);
		assert(seg.min_left(combined, n - 1, [](long long x) { return x <= 2; }) == 1);
		assert(seg.max_right(right, n - 1, [](long long x) { return x <= 5; }) == n);
		assert(seg.min_left(left, 1, [](long long x) { return x <= 3; }) == 0);
		assert(seg.copy_range(combined, initial, 0, n) == initial);
	}
	PersistentSegtree<Value, sum, identity> no_default(std::vector<Value>{Value(2), Value(5)});
	auto v0 = no_default.initial_root(), v1 = no_default.multiply(v0, 1, Value(7));
	assert(no_default.prod(v0, 0, 2).x == 7 && no_default.all_prod(v1).x == 14);
	Strings long_values(std::vector<std::string>{std::string(50, 'a'), std::string(60, 'b'), std::string(70, 'c')});
	auto s0 = long_values.initial_root(), s1 = long_values.set(s0, 1, std::string(80, 'd'));
	auto s2 = long_values.copy_range(s1, s0, 1, 3);
	assert(long_values.get(s0, 1) == std::string(60, 'b') && long_values.get(s1, 1) == std::string(80, 'd'));
	assert(long_values.all_prod(s2) == std::string(50, 'a') + std::string(60, 'b') + std::string(70, 'c'));
}
