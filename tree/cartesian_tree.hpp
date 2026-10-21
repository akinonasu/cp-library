#ifndef CARTESIAN_TREE_HPP
#define CARTESIAN_TREE_HPP

#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <ranges>
#include <vector>

struct CartesianTree {
	int root = -1;
	std::vector<int> parent, left, right;
	int size() const { return static_cast<int>(parent.size()); }
	bool empty() const { return parent.empty(); }
};

template<bool Rightmost = false, class Range, class Compare = std::less<>>
	requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
CartesianTree cartesian_tree(const Range &a, Compare compare = {}) {
	assert(std::ranges::size(a) <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(std::ranges::size(a));
	CartesianTree tree;
	tree.parent.assign(n, -1);
	tree.left.assign(n, -1);
	tree.right.assign(n, -1);
	auto first = std::ranges::begin(a);
	int top = -1;
	for (int i = 0; i < n; ++i) {
		int last = -1;
		auto &&value = first[i];
		while (top != -1) {
			bool before;
			if constexpr (Rightmost) before = !compare(first[top], value);
			else before = compare(value, first[top]);
			if (!before) break;
			last = top;
			top = tree.parent[top];
		}
		tree.parent[i] = top;
		if (top == -1) tree.root = i;
		else tree.right[top] = i;
		if (last != -1) tree.parent[last] = i, tree.left[i] = last;
		top = i;
	}
	return tree;
}

#endif
