#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include "data_structure/persistent_segtree.hpp"
#include "components/fast_io.hpp"
#include <algorithm>
#include <bit>
#include <utility>
#include <vector>

int op(int a, int b) { return a + b; }
int e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> initial(n), coordinates;
	for (int &x : initial) input.read(x);
	coordinates = initial;
	std::vector<std::pair<int, int>> queries(q);
	for (auto &[type, x] : queries) {
		input.read(type), input.read(x);
		if (type != 2) coordinates.push_back(x);
	}
	std::sort(coordinates.begin(), coordinates.end());
	coordinates.erase(std::unique(coordinates.begin(), coordinates.end()), coordinates.end());
	int m = int(coordinates.size());
	auto lower = [&](int x) { return int(std::lower_bound(coordinates.begin(), coordinates.end(), x) - coordinates.begin()); };
	auto upper = [&](int x) { return int(std::upper_bound(coordinates.begin(), coordinates.end(), x) - coordinates.begin()); };
	std::vector<int> counts(m);
	for (int x : initial) counts[lower(x)] = 1;
	PersistentSegtree<int, op, e> tree(counts);
	tree.reserve(std::size_t(m) * 2 + std::size_t(q) * (std::bit_width(unsigned(m)) / 2 + 2));
	auto root = tree.initial_root();
	for (auto [type, x] : queries) {
		if (type == 0) { int p = lower(x); if (!tree.get(root, p)) root = tree.multiply(root, p, 1); }
		else if (type == 1) root = tree.set(root, lower(x), 0);
		else if (type == 2) {
			if (tree.all_prod(root) < x) output.println(-1);
			else output.println(coordinates[tree.max_right(root, 0, [x](int count) { return count < x; })]);
		} else if (type == 3) output.println(tree.prod(root, 0, upper(x)));
		else if (type == 4) {
			int p = tree.min_left(root, upper(x), [](int count) { return count == 0; });
			output.println(p ? coordinates[p - 1] : -1);
		} else {
			int p = tree.max_right(root, lower(x), [](int count) { return count == 0; });
			output.println(p == m ? -1 : coordinates[p]);
		}
	}
}
