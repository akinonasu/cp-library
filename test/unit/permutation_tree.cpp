#include "../../tree/permutation_tree.hpp"
#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

void require(bool condition) { if (!condition) std::abort(); }
using Tree = PermutationTree;
using Type = Tree::Type;

void structure(const std::vector<int> &p, const Tree &tree) {
	int n = int(p.size()), m = tree.size();
	require(tree.length() == n && tree.empty() == p.empty());
	require(tree.nodes().size() == std::size_t(m) && tree.order().size() == std::size_t(m));
	if (!n) {
		require(m == 0 && tree.root() == -1 && tree.count_intervals() == 0);
		return;
	}
	require(n <= m && m <= 2 * n - 1);
	int root = tree.root();
	require(0 <= root && root < m);
	require(tree[root].parent == -1 && tree[root].l == 0 && tree[root].r == n);
	require(tree[root].lo == 0 && tree[root].hi == n);
	std::vector<int> seen(m), position(m, -1);
	for (int i = 0; i < m; ++i) {
		int v = tree.order()[i];
		require(0 <= v && v < m && position[v] == -1);
		position[v] = i;
	}
	require(tree.order()[0] == root);
	std::int64_t count = n;
	for (int v = 0; v < m; ++v) {
		const auto &node = tree[v];
		require(0 <= node.l && node.l < node.r && node.r <= n);
		require(node.hi - node.lo == node.r - node.l);
		auto children = tree.children(v);
		if (children.empty()) {
			require(node.type == Type::Leaf && node.r == node.l + 1);
			require(v == tree.leaf(node.l));
			require(node.lo == p[node.l] && node.hi == p[node.l] + 1);
			continue;
		}
		require(children.size() >= 2 && node.type != Type::Leaf);
		int end = node.l, lo = n, hi = 0;
		bool increasing = true, decreasing = true;
		for (int i = 0; i < int(children.size()); ++i) {
			int child = children[i];
			require(0 <= child && child < m && ++seen[child] == 1);
			const auto &part = tree[child];
			require(part.parent == v && part.l == end && position[v] < position[child]);
			end = part.r;
			lo = std::min(lo, part.lo), hi = std::max(hi, part.hi);
			if (i) {
				increasing &= tree[children[i - 1]].hi == part.lo;
				decreasing &= tree[children[i - 1]].lo == part.hi;
			}
		}
		require(end == node.r && lo == node.lo && hi == node.hi);
		Type expected = increasing ? Type::Increasing : decreasing ? Type::Decreasing : Type::Prime;
		require(node.type == expected);
		if (node.type == Type::Prime) {
			require(children.size() >= 4);
			++count;
		} else {
			for (int child : children) require(tree[child].type != node.type);
			std::int64_t k = children.size();
			count += k * (k - 1) / 2;
		}
	}
	for (int v = 0; v < m; ++v) require(seen[v] == (v != root));
	require(tree.count_intervals() == count);
}

void brute(const std::vector<int> &p) {
	Tree tree(p);
	structure(p, tree);
	int n = int(p.size());
	std::vector<std::pair<int, int>> common, strong, actual;
	for (int l = 0; l < n; ++l) {
		int lo = n, hi = -1;
		for (int r = l + 1; r <= n; ++r) {
			lo = std::min(lo, p[r - 1]);
			hi = std::max(hi, p[r - 1]);
			if (hi - lo == r - l - 1) common.emplace_back(l, r);
		}
	}
	for (auto [l, r] : common) {
		bool good = true;
		for (auto [a, b] : common)
			if (l < b && a < r && !(l <= a && b <= r) && !(a <= l && r <= b)) good = false;
		if (good) strong.emplace_back(l, r);
	}
	for (auto &node : tree.nodes()) actual.emplace_back(node.l, node.r);
	std::sort(actual.begin(), actual.end());
	require(actual == strong);
	require(tree.count_intervals() == std::int64_t(common.size()));
	std::vector<std::pair<int, int>> represented;
	for (int v = 0; v < tree.size(); ++v) {
		const auto &node = tree[v];
		represented.emplace_back(node.l, node.r);
		if (node.type == Type::Increasing || node.type == Type::Decreasing) {
			auto children = tree.children(v);
			for (int l = 0; l < int(children.size()); ++l)
				for (int r = l + 1; r < int(children.size()); ++r)
					represented.emplace_back(tree[children[l]].l, tree[children[r]].r);
		}
	}
	std::sort(represented.begin(), represented.end());
	represented.erase(std::unique(represented.begin(), represented.end()), represented.end());
	require(represented == common);
	Tree rebuilt;
	rebuilt.build(p);
	require(rebuilt.count_intervals() == tree.count_intervals());
	auto copy = tree;
	auto moved = std::move(copy);
	structure(p, moved);
	copy.build(p);
	structure(p, copy);
	rebuilt.build({});
	structure({}, rebuilt);
}

int main() {
	for (int n = 0; n <= 8; ++n) {
		std::vector<int> p(n);
		std::iota(p.begin(), p.end(), 0);
		do { brute(p); } while (std::next_permutation(p.begin(), p.end()));
	}
	std::mt19937 rng(279529);
	for (int trial = 0; trial < 1000; ++trial) {
		std::vector<int> p(9 + rng() % 16);
		std::iota(p.begin(), p.end(), 0);
		std::shuffle(p.begin(), p.end(), rng);
		brute(p);
	}
	brute({5, 1, 3, 0, 2, 4});
	for (int kind = 0; kind < 5; ++kind) {
		int n = 300000;
		std::vector<int> p(n);
		std::iota(p.begin(), p.end(), 0);
		if (kind == 1) std::reverse(p.begin(), p.end());
		if (kind == 2) std::shuffle(p.begin(), p.end(), rng);
		if (kind == 3) for (int i = 0; i + 1 < n; i += 2) std::swap(p[i], p[i + 1]);
		if (kind == 4) for (int i = 0; i < n; ++i) p[i] = i % 2 ? n - 1 - i / 2 : i / 2;
		Tree tree(p);
		structure(p, tree);
		if (kind < 2) require(tree.count_intervals() == std::int64_t(n) * (n + 1) / 2);
		tree.build(p);
		structure(p, tree);
	}
}
