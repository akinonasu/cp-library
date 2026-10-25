#include "data_structure/rollback_union_find.hpp"
#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

void require(bool value) { if (!value) std::abort(); }

void check(const RollbackUnionFind &uf, const std::vector<int> &label, std::size_t state) {
	int n = static_cast<int>(label.size());
	require(uf.vertex_count() == n && uf.snapshot() == state);
	std::vector<std::vector<int>> groups;
	for (int v = 0; v < n; ++v) {
		int size = 0;
		for (int u = 0; u < n; ++u) {
			require(uf.same(v, u) == (label[v] == label[u]));
			size += label[v] == label[u];
		}
		require(uf.size(v) == size && uf.leader(uf.leader(v)) == uf.leader(v));
		if (label[v] == v) {
			groups.emplace_back();
			for (int u = 0; u < n; ++u) if (label[u] == v) groups.back().push_back(u);
		}
	}
	require(uf.groups() == groups && uf.component_count() == static_cast<int>(groups.size()));
	require(uf.snapshot() == state);
}

std::vector<int> merge(std::vector<int> label, int x, int y) {
	int a = label[x], b = label[y];
	for (int &v : label) if (v == a || v == b) v = std::min(a, b);
	return label;
}

void exhaustive(RollbackUnionFind &uf, const std::vector<int> &label, int remaining) {
	auto state = uf.snapshot();
	check(uf, label, state);
	if (!remaining) return;
	for (int x = 0; x < uf.vertex_count(); ++x)
		for (int y = 0; y < uf.vertex_count(); ++y) {
			require(uf.merge(x, y) == (label[x] != label[y]));
			exhaustive(uf, merge(label, x, y), remaining - 1);
			uf.rollback(state);
			check(uf, label, state);
		}
}

int main() {
	RollbackUnionFind empty;
	check(empty, {}, 0);
	empty.reserve(100), empty.rollback();
	empty.reset(3);
	require(empty.merge(1, 2));
	auto state = empty.snapshot();
	int root = empty.leader(1);
	require(!empty.merge(1, 2) && !empty.merge(0, 0) && empty.snapshot() == state + 2);
	empty.undo();
	require(empty.same(1, 2) && empty.component_count() == 2);
	empty.undo();
	require(empty.snapshot() == state && empty.leader(1) == root);
	empty.undo();
	check(empty, {0, 1, 2}, 0);
	RollbackUnionFind small(4);
	exhaustive(small, {0, 1, 2, 3}, 3);
	std::mt19937 rng(20261020);
	for (int trial = 0; trial < 200; ++trial) {
		int n = 1 + rng() % 20;
		RollbackUnionFind uf(n);
		if (trial & 1) uf.reserve(1000);
		std::vector<int> initial(n);
		std::iota(initial.begin(), initial.end(), 0);
		std::vector<std::vector<int>> versions{initial};
		for (int step = 0; step < 500; ++step) {
			int type = rng() % 10;
			if (type < 6) {
				int x = rng() % n, y = rng() % n;
				require(uf.merge(x, y) == (versions.back()[x] != versions.back()[y]));
				versions.push_back(merge(versions.back(), x, y));
			} else if (type == 6 && versions.size() > 1) {
				uf.undo(), versions.pop_back();
			} else if (type < 9) {
				std::size_t target = rng() % versions.size();
				uf.rollback(target), versions.resize(target + 1);
			} else {
				auto copied = uf;
				copied.rollback();
				check(copied, initial, 0);
				copied.reset(0);
				check(copied, {}, 0);
			}
			check(uf, versions.back(), versions.size() - 1);
		}
	}
	constexpr int n = 1 << 18;
	RollbackUnionFind balanced(n);
	balanced.reserve(n);
	for (int width = 1; width < n; width *= 2)
		for (int v = 0; v < n; v += width * 2) require(balanced.merge(v, v + width));
	require(balanced.size(n - 1) == n && balanced.component_count() == 1);
	for (int i = n - 1; i > 0; --i) balanced.undo();
	require(balanced.component_count() == n && balanced.snapshot() == 0);
	for (int v = 0; v < n; ++v) require(balanced.leader(v) == v && balanced.size(v) == 1);
	balanced.reset(2);
	require(balanced.vertex_count() == 2 && balanced.merge(1, 0));
	balanced.rollback();
	require(!balanced.same(0, 1));
}
