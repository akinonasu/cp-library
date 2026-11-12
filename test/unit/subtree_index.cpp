#include "tree/subtree_index.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>

void check(const std::vector<int> &parents) {
	SubtreeIndex tree(parents);
	int n = int(parents.size());
	assert(tree.size() == n && tree.empty() == (n == 0));
	if (!n) { assert(tree.root() == -1); return; }
	int root = int(std::find(parents.begin(), parents.end(), -1) - parents.begin());
	assert(tree.root() == root && tree.in[root] == 0 && tree.out[root] == n);
	std::vector<int> visited(n);
	for (int i = 0; i < n; ++i) {
		int v = tree.order[i];
		assert(0 <= v && v < n && !visited[v]++ && tree.in[v] == i);
	}
	for (int u = 0; u < n; ++u) {
		int descendants = 0;
		for (int v = 0; v < n; ++v) {
			bool ancestor = false;
			for (int p = v; p != -1; p = parents[p]) ancestor |= p == u;
			assert(tree.is_ancestor(u, v) == ancestor);
			assert((tree.in[u] <= tree.in[v] && tree.in[v] < tree.out[u]) == ancestor);
			descendants += ancestor;
		}
		auto [l, r] = tree.subtree(u);
		assert(l == tree.in[u] && r == tree.out[u] && r - l == descendants);
		assert((tree.subtree(u, true) == std::pair{l + 1, r}));
	}
	auto copy = tree;
	tree.clear();
	assert(tree.empty() && tree.root() == -1);
	tree.build_from_parents(parents);
	assert(tree.in == copy.in && tree.out == copy.out && tree.order == copy.order);
	SubtreeIndex moved(std::move(copy));
	assert(copy.empty() && copy.root() == -1 && moved.in == tree.in);
	copy = moved;
	auto *self = &copy;
	copy = *self;
	copy = std::move(*self);
	assert(copy.in == tree.in);
	copy.build_from_parents({-1});
	swap(copy, tree);
	assert(tree.size() == 1 && copy.in == moved.in);
}

int main() {
	std::mt19937 random(9348613);
	check({});
	for (int n = 1; n <= 7; ++n) {
		std::vector<int> parents(n, -1);
		auto enumerate = [&](auto &&self, int v) -> void {
			if (v == n) { check(parents); return; }
			for (int p = 0; p < v; ++p) { parents[v] = p; self(self, v + 1); }
		};
		enumerate(enumerate, 1);
	}
	for (int trial = 0; trial < 1000; ++trial) {
		int n = int(random() % 37);
		std::vector<int> parents(n, -1), labels(n);
		for (int v = 1; v < n; ++v) parents[v] = int(random() % v);
		if (trial % 5 == 0) for (int v = 1; v < n; ++v) parents[v] = v - 1;
		if (trial % 5 == 1) for (int v = 1; v < n; ++v) parents[v] = 0;
		check(parents);
		std::iota(labels.begin(), labels.end(), 0);
		std::shuffle(labels.begin(), labels.end(), random);
		std::vector<int> relabeled(n);
		for (int v = 0; v < n; ++v) relabeled[labels[v]] = parents[v] == -1 ? -1 : labels[parents[v]];
		check(relabeled);
	}
	constexpr int n = 250000;
	std::vector<int> parents(n, -1);
	for (int v = 1; v < n; ++v) parents[v] = v - 1;
	SubtreeIndex tree(parents);
	for (int v = 0; v < n; ++v) assert(tree.in[v] == v && tree.out[v] == n && tree.order[v] == v);
	for (int v = 0; v + 1 < n; ++v) parents[v] = v + 1;
	parents.back() = -1;
	tree.build_from_parents(std::move(parents));
	for (int v = 0; v < n; ++v) assert(tree.in[v] == n - 1 - v && tree.out[v] == n && tree.order[v] == n - 1 - v);
}
