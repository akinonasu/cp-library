#include "tree/cartesian_tree.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <span>
#include <string>

template<bool Rightmost, class Range, class Compare>
CartesianTree reference(const Range &a, Compare compare) {
	int n = std::ranges::size(a);
	auto first = std::ranges::begin(a);
	CartesianTree tree;
	tree.parent.assign(n, -1);
	tree.left.assign(n, -1);
	tree.right.assign(n, -1);
	struct Frame { int l, r, parent; bool right; };
	std::vector<Frame> pending{ {0, n, -1, false} };
	while (!pending.empty()) {
		auto [l, r, parent, right] = pending.back();
		pending.pop_back();
		if (l == r) continue;
		int best = l;
		for (int i = l + 1; i < r; ++i) {
			bool better = Rightmost ? !compare(first[best], first[i]) : compare(first[i], first[best]);
			if (better) best = i;
		}
		tree.parent[best] = parent;
		if (parent == -1) tree.root = best;
		else (right ? tree.right : tree.left)[parent] = best;
		pending.push_back({l, best, best, false});
		pending.push_back({best + 1, r, best, true});
	}
	return tree;
}

template<bool Rightmost, class Range, class Compare>
void check(const Range &a, Compare compare) {
	auto actual = cartesian_tree<Rightmost>(a, compare);
	auto expected = reference<Rightmost>(a, compare);
	assert(actual.root == expected.root);
	assert(actual.parent == expected.parent);
	assert(actual.left == expected.left && actual.right == expected.right);
	assert(actual.size() == int(std::ranges::size(a)));
	assert(actual.empty() == std::ranges::empty(a));
}

template<class Range>
void check(const Range &a) {
	check<false>(a, std::less<>{});
	check<true>(a, std::less<>{});
	check<false>(a, std::greater<>{});
	check<true>(a, std::greater<>{});
}

void large(const std::vector<int> &a) {
	std::size_t comparisons = 0;
	auto tree = cartesian_tree(a, [&](int x, int y) { ++comparisons; return x < y; });
	assert(comparisons <= 2 * a.size());
	assert(tree.parent[tree.root] == -1);
	std::vector<int> stack;
	int v = tree.root, index = 0;
	while (v != -1 || !stack.empty()) {
		while (v != -1) { stack.push_back(v); v = tree.left[v]; }
		v = stack.back();
		stack.pop_back();
		assert(v == index++);
		for (int child : {tree.left[v], tree.right[v]}) if (child != -1) {
			assert(tree.parent[child] == v && a[v] <= a[child]);
			if (a[v] == a[child]) assert(v < child);
		}
		v = tree.right[v];
	}
	assert(index == int(a.size()));
}

int main() {
	CartesianTree empty;
	assert(empty.empty() && empty.root == -1);
	for (int n = 0; n <= 8; ++n) {
		std::vector<int> a(n);
		do {
			check(a);
			int i = 0;
			while (i < n && ++a[i] == 3) a[i++] = 0;
			if (i == n) break;
		} while (true);
	}
	for (int n = 1; n <= 8; ++n) {
		std::vector<int> a(n);
		std::iota(a.begin(), a.end(), 0);
		do { check(a); } while (std::next_permutation(a.begin(), a.end()));
	}
	std::mt19937 random(20261021);
	for (int trial = 0; trial < 1000; ++trial) {
		std::vector<int> a(random() % 101);
		for (int &x : a) x = int(random() % 31) - 15;
		const auto saved = a;
		check(a);
		assert(a == saved);
	}
	check(std::array<int, 0>{});
	check(std::array<int, 4>{std::numeric_limits<int>::min(), 0, std::numeric_limits<int>::max(), 0});
	check(std::deque<int>{3, 2, 1, 1, 2});
	check(std::vector<bool>{true, false, true, false});
	check(std::string("banana"));
	check(std::views::iota(0, 30));
	std::vector<int> values{5, 3, 4, 1, 2};
	check(std::span<const int>(values));
	const int array[]{3, 1, 2};
	check(array);
	std::vector<std::unique_ptr<int>> pointers;
	for (int x : {3, 1, 2, 1}) pointers.push_back(std::make_unique<int>(x));
	auto compare = [](const auto &a, const auto &b) { return *a < *b; };
	check<false>(pointers, compare);
	check<true>(pointers, compare);
	std::vector<int> equal(7, 4);
	assert(cartesian_tree(equal).root == 0);
	assert(cartesian_tree<true>(equal).root == 6);
	std::vector<int> big(1000000);
	std::iota(big.begin(), big.end(), 0);
	large(big);
	std::reverse(big.begin(), big.end());
	large(big);
	std::shuffle(big.begin(), big.end(), random);
	large(big);
	std::fill(big.begin(), big.end(), 1);
	large(big);
}
