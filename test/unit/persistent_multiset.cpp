#include "../../data_structure/persistent/persistent_multiset.hpp"
#include "../../data_structure/persistent/persistent_heap.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <vector>

struct Value { int x; explicit Value(int v) : x(v) {} };
struct Less { bool operator()(const Value &a, const Value &b) const { return a.x < b.x; } };

template <class Compare> void sets(Compare comp) {
	PersistentMultiset<int, Compare> set(comp);
	auto built = set.build({3, 1, 3, 2, 1});
	std::vector<int> sorted{1, 1, 2, 3, 3};
	std::sort(sorted.begin(), sorted.end(), comp);
	for (int i = 0; i < 5; ++i) assert(set.kth(built, i) == sorted[i]);
	assert(set.count(built, 3) == 2 && set.empty(set.build({})));
	std::vector<typename decltype(set)::root_type> roots{0};
	std::vector<std::multiset<int, Compare>> values(1, std::multiset<int, Compare>(comp));
	std::mt19937 random(334);
	for (int t = 0; t < 20000; ++t) {
		unsigned k = random() % roots.size();
		auto v = values[k]; auto r = roots[k];
		int x = int(random() % 30), count = random() % 4;
		if (random() % 2) { r = set.insert(r, x, count); for (int i = 0; i < count; ++i) v.insert(x); }
		else { r = set.erase(r, x, count); for (int i = 0; i < count; ++i) { auto it = v.find(x); if (it != v.end()) v.erase(it); } }
		roots.push_back(r); values.push_back(v);
		k = random() % roots.size(); auto before = set.node_count();
		assert(set.size(roots[k]) == static_cast<std::int64_t>(values[k].size()));
		std::int64_t index = 0;
		for (int value : values[k]) assert(set.kth(roots[k], index++) == value);
		for (int value = -1; value <= 30; ++value) {
			auto l = values[k].lower_bound(value), u = values[k].upper_bound(value);
			assert(set.count(roots[k], value) == static_cast<std::int64_t>(values[k].count(value)));
			assert(set.order_of_key(roots[k], value) == std::distance(values[k].begin(), l));
			assert(set.lower_bound(roots[k], value) == (l == values[k].end() ? std::nullopt : std::optional<int>(*l)));
			assert(set.upper_bound(roots[k], value) == (u == values[k].end() ? std::nullopt : std::optional<int>(*u)));
		}
		assert(set.node_count() == before);
	}
	auto r = set.initial_root();
	for (int i = 0; i < 10000; ++i) r = set.insert(r, i);
	auto old = r;
	for (int i = 0; i < 10000; i += 2) r = set.erase_all(r, i);
	assert(set.size(old) == 10000 && set.size(r) == 5000);
	for (int i = 1; i < 10000; i += 2) r = set.erase_all(r, i);
	assert(set.empty(r));
}

int main() {
	sets(std::less<int>{}); sets(std::greater<int>{});
	PersistentMultiset<std::string> repeated;
	auto r = repeated.insert(0, "x", std::numeric_limits<std::int64_t>::max());
	assert(repeated.kth(r, std::numeric_limits<std::int64_t>::max() - 1) == "x");
	assert(repeated.size(repeated.erase(r, "x", 100)) == std::numeric_limits<std::int64_t>::max() - 100);
	assert(repeated.empty(repeated.erase_all(r, "x")));
	PersistentHeap<int> heap;
	std::vector<decltype(heap)::root_type> roots{0};
	std::vector<std::multiset<int>> values(1);
	std::mt19937 random(1234);
	for (int t = 0; t < 15000; ++t) {
		unsigned k = random() % roots.size(), other = random() % roots.size();
		auto v = values[k]; auto root = roots[k];
		if (t % 3 == 0 && v.size() + values[other].size() < 1000) {
			root = heap.meld(root, roots[other]); v.insert(values[other].begin(), values[other].end());
		} else if (t % 3 == 1 && !v.empty()) { root = heap.pop(root); v.erase(v.begin()); }
		else { int x = random() % 1000; root = heap.push(root, x); v.insert(x); }
		roots.push_back(root); values.push_back(v);
		k = random() % roots.size();
		assert(heap.size(roots[k]) == static_cast<std::int64_t>(values[k].size()));
		if (!values[k].empty()) assert(heap.top(roots[k]) == *values[k].begin());
		if (t % 100 == 0) {
			auto q = roots[k];
			for (int x : values[k]) { assert(heap.top(q) == x); q = heap.pop(q); }
			assert(heap.empty(q));
		}
	}
	PersistentHeap<std::string, std::greater<std::string>> maximum;
	auto a = maximum.push(0, "a"), b = maximum.push(a, "b"), c = maximum.meld(b, b);
	assert(maximum.size(c) == 4 && maximum.top(c) == "b");
	assert(maximum.top(maximum.pop(maximum.pop(c))) == "a" && maximum.top(a) == "a");
	PersistentHeap<Value, Less> compact;
	auto r0 = compact.push(0, Value(7));
	for (int i = 0; i < 40; ++i) r0 = compact.meld(r0, r0);
	assert(compact.size(r0) == (std::int64_t(1) << 40) && compact.top(compact.pop(r0)).x == 7);
	assert(compact.node_count() < 5000);
	PersistentHeap<int> deep;
	auto perfect = deep.initial_root();
	std::int64_t length = 0;
	for (int i = 0; i < 63; ++i) {
		auto old = perfect;
		perfect = deep.push(deep.meld(perfect, perfect), 7);
		assert(deep.size(old) == length);
		length = 2 * length + 1;
		assert(deep.size(perfect) == length && deep.top(perfect) == 7);
	}
	assert(length == std::numeric_limits<std::int64_t>::max());
	auto shorter = deep.pop(perfect);
	assert(deep.size(shorter) == length - 1 && deep.top(shorter) == 7);
	auto changed = deep.push(shorter, -1);
	assert(deep.top(changed) == -1 && deep.top(perfect) == 7 && deep.size(changed) == length);
	PersistentMultiset<Value, Less> custom;
	auto r1 = custom.build({Value(2), Value(1), Value(2)});
	assert(custom.kth(r1, 0).x == 1 && custom.count(r1, Value(2)) == 2);
	auto r2 = custom.erase_all(r1, Value(2));
	assert(custom.size(r2) == 1 && custom.size(r1) == 3);
}
