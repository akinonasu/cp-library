#include "tree/persistent_link_cut_tree.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace {
std::int64_t add(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t zero() { return 0; }
std::string concat(const std::string &a, const std::string &b) { return a + b; }
std::string empty_string() { return {}; }
using SumTree = PersistentLinkCutTree<std::int64_t, add, zero, true>;
using WordTree = PersistentLinkCutTree<std::string, concat, empty_string>;

struct Forest {
	std::vector<std::vector<unsigned char>> adjacent;
	std::vector<std::int64_t> values;
	std::vector<std::string> words;
	std::vector<int> roots;

	explicit Forest(int n) : adjacent(n, std::vector<unsigned char>(n)), values(n), words(n), roots(n) {
		for (int i = 0; i < n; ++i) roots[i] = i;
	}
	int size() const { return static_cast<int>(values.size()); }
	std::vector<int> path(int u, int v) const {
		std::vector<int> parent(size(), -1), queue{u};
		parent[u] = u;
		for (std::size_t at = 0; at < queue.size(); ++at)
			for (int to = 0; to < size(); ++to)
				if (adjacent[queue[at]][to] && parent[to] == -1) {
					parent[to] = queue[at];
					queue.push_back(to);
				}
		if (parent[v] == -1) return {};
		std::vector<int> result{v};
		while (result.back() != u) result.push_back(parent[result.back()]);
		std::reverse(result.begin(), result.end());
		return result;
	}
	void reroot(int u) {
		for (int v = 0; v < size(); ++v) if (!path(u, v).empty()) roots[v] = u;
	}
	bool link(int u, int v) {
		if (u == v) return false;
		const bool connected = !path(u, v).empty();
		const int root = roots[v];
		reroot(u);
		if (connected) return false;
		adjacent[u][v] = adjacent[v][u] = 1;
		reroot(root);
		return true;
	}
	bool cut(int u, int v) {
		if (u == v) return false;
		reroot(u);
		if (!adjacent[u][v]) return false;
		adjacent[u][v] = adjacent[v][u] = 0;
		reroot(v);
		return true;
	}
	int lca(int root, int u, int v) const {
		const auto a = path(root, u), b = path(root, v);
		int at = 0;
		while (at < static_cast<int>(std::min(a.size(), b.size())) && a[at] == b[at]) ++at;
		assert(at);
		return a[at - 1];
	}
	std::int64_t sum(const std::vector<int> &route) const {
		std::int64_t value = 0;
		for (int v : route) value += values[v];
		return value;
	}
	std::string word(const std::vector<int> &route) const {
		std::string value;
		for (int v : route) value += words[v];
		return value;
	}
};

struct Version {
	SumTree::root_type sums;
	WordTree::root_type words;
	Forest forest;
};

void inspect(SumTree &sums, WordTree &words, const Version &version, int u, int v) {
	const auto &forest = version.forest;
	const auto route = forest.path(u, v);
	assert(sums.get(version.sums, u) == forest.values[u]);
	assert(words.get(version.words, u) == forest.words[u]);
	const auto sum_same = sums.same(version.sums, u, v);
	const auto word_same = words.same(version.words, u, v);
	assert(sum_same.value == !route.empty() && word_same.value == !route.empty());
	assert(sums.root(sum_same.root, u).value == forest.roots[u]);
	assert(words.root(word_same.root, v).value == forest.roots[v]);
	if (!route.empty()) {
		const auto sum = sums.prod(version.sums, u, v);
		const auto word = words.prod(version.words, u, v);
		assert(sum.value == forest.sum(route) && word.value == forest.word(route));
		assert(sums.root(sum.root, v).value == u && words.root(word.root, v).value == u);
		auto reverse = route;
		std::reverse(reverse.begin(), reverse.end());
		assert(words.prod(version.words, v, u).value == forest.word(reverse));
	}
	assert(sums.root(version.sums, u).value == forest.roots[u]);
	assert(words.root(version.words, v).value == forest.roots[v]);
}

void normalize(SumTree &sums, WordTree &words, Version &version) {
	std::vector<bool> visited(version.forest.size());
	for (int u = 0; u < version.forest.size(); ++u) if (!visited[u]) {
		const int a = sums.root(version.sums, u).value, b = words.root(version.words, u).value;
		assert(!version.forest.path(u, a).empty() && !version.forest.path(u, b).empty());
		version.sums = sums.make_root(version.sums, u);
		version.words = words.make_root(version.words, u);
		version.forest.reroot(u);
		for (int v = 0; v < version.forest.size(); ++v) if (!version.forest.path(u, v).empty()) visited[v] = true;
	}
}

void branching() {
	std::mt19937 random(934817);
	for (int trial = 0; trial < 12; ++trial) {
		const int n = 1 + random() % 19;
		Forest initial(n);
		std::vector<std::pair<int, int>> edges;
		for (int v = 0; v < n; ++v) {
			initial.values[v] = static_cast<int>(random() % 101) - 50;
			initial.words[v] = std::string(1, 'A' + random() % 26);
			if (v && random() % 4) {
				const int u = random() % v;
				edges.emplace_back(u, v);
				initial.adjacent[u][v] = initial.adjacent[v][u] = 1;
			}
		}
		SumTree sums(initial.values, edges);
		WordTree words(initial.words, edges);
		assert(sums.size() == n && !sums.empty() && words.size() == n);
		Version first{sums.initial_root(), words.initial_root(), initial};
		normalize(sums, words, first);
		std::vector<Version> versions{first};
		for (int step = 0; step < 700; ++step) {
			const std::size_t source = random() % 3 ? versions.size() - 1 : random() % versions.size();
			Version next = versions[source];
			auto &forest = next.forest;
			const int u = random() % n, v = random() % n, operation = random() % 12;
			const auto route = forest.path(u, v);
			if (operation == 0) {
				const bool expected = forest.link(u, v);
				auto [sr, a] = sums.link(next.sums, u, v);
				auto [wr, b] = words.link(next.words, u, v);
				assert(a == expected && b == expected);
				next.sums = sr, next.words = wr;
			} else if (operation == 1) {
				const bool expected = forest.cut(u, v);
				auto [sr, a] = sums.cut(next.sums, u, v);
				auto [wr, b] = words.cut(next.words, u, v);
				assert(a == expected && b == expected);
				next.sums = sr, next.words = wr;
			} else if (operation == 2) {
				forest.values[u] = static_cast<int>(random() % 2001) - 1000;
				forest.words[u].resize(1 + random() % 3);
				for (char &letter : forest.words[u]) letter = char('A' + random() % 26);
				next.sums = sums.set(next.sums, u, forest.values[u]);
				next.words = words.set(next.words, u, forest.words[u]);
			} else if (operation == 3 && !route.empty()) {
				auto [sr, a] = sums.prod(next.sums, u, v);
				auto [wr, b] = words.prod(next.words, u, v);
				assert(a == forest.sum(route) && b == forest.word(route));
				next.sums = sr, next.words = wr;
				forest.reroot(u);
			} else if (operation == 4 && !route.empty()) {
				const int k = static_cast<int>(random() % (route.size() + 4)) - 2;
				const int expected = k < 0 || k >= static_cast<int>(route.size()) ? -1 : route[k];
				auto [sr, a] = sums.jump(next.sums, u, v, k);
				auto [wr, b] = words.jump(next.words, u, v, k);
				assert(a == expected && b == expected);
				next.sums = sr, next.words = wr;
				forest.reroot(u);
			} else if (operation == 5) {
				auto [sr, a] = sums.same(next.sums, u, v);
				auto [wr, b] = words.same(next.words, u, v);
				assert(a == !route.empty() && b == !route.empty());
				next.sums = sr, next.words = wr;
			} else if (operation == 6) {
				next.sums = sums.make_root(next.sums, u);
				next.words = words.make_root(next.words, u);
				forest.reroot(u);
			} else if (operation == 7 && !route.empty()) {
				int root = random() % n;
				if (forest.path(root, u).empty()) root = u;
				const int expected = forest.lca(root, u, v);
				auto [sr, a] = sums.lca(next.sums, root, u, v);
				auto [wr, b] = words.lca(next.words, root, u, v);
				assert(a == expected && b == expected);
				next.sums = sr, next.words = wr;
				forest.reroot(root);
			} else if (operation == 8 && !route.empty()) {
				auto [sr, a] = sums.path_size(next.sums, u, v);
				auto [wr, b] = words.distance(next.words, u, v);
				assert(a == static_cast<int>(route.size()) && b == a - 1);
				next.sums = sr, next.words = wr;
				forest.reroot(u);
			} else if (operation == 9) {
				auto [sr, a] = sums.root(next.sums, u);
				auto [wr, b] = words.root(next.words, v);
				assert(a == forest.roots[u] && b == forest.roots[v]);
				next.sums = sr, next.words = wr;
			} else if (operation == 10) {
				edges.clear();
				for (auto &row : forest.adjacent) std::fill(row.begin(), row.end(), 0);
				for (int to = 1; to < n; ++to) if (random() % 3) {
					const int from = random() % to;
					edges.emplace_back(from, to);
					forest.adjacent[from][to] = forest.adjacent[to][from] = 1;
				}
				next.sums = sums.build_forest(next.sums, edges);
				next.words = words.build_forest(next.words, edges);
				normalize(sums, words, next);
			} else {
				assert(sums.get(next.sums, u) == forest.values[u]);
				assert(words.get(next.words, v) == forest.words[v]);
			}
			inspect(sums, words, versions[source], u, v);
			inspect(sums, words, next, u, v);
			versions.push_back(std::move(next));
			if (step % 19 == 0) {
				const auto &old = versions[random() % versions.size()];
				for (int x = 0; x < n; ++x) inspect(sums, words, old, x, random() % n);
			}
		}
	}
}

struct Value {
	int number;
	explicit Value(int number) : number(number) {}
};
Value combine(Value a, Value b) { return Value(a.number + b.number); }
Value identity() { return Value(0); }
int minimum(int a, int b) { return std::min(a, b); }
int infinity() { return std::numeric_limits<int>::max(); }
bool either(bool a, bool b) { return a || b; }
bool no() { return false; }

void boundaries() {
	SumTree empty;
	assert(empty.empty() && empty.size() == 0);
	const auto empty_version = empty.build_forest(empty.initial_root(), {});
	[[maybe_unused]] const auto empty_again = empty.build_forest(empty_version, {});
	SumTree singleton(1);
	const auto root = singleton.initial_root();
	assert(!singleton.empty() && singleton.get(root, 0) == 0);
	assert(!singleton.link(root, 0, 0).value && !singleton.cut(root, 0, 0).value);
	assert(singleton.same(root, 0, 0).value);
	assert(singleton.prod(root, 0, 0).value == 0);
	assert(singleton.path_size(root, 0, 0).value == 1);
	assert(singleton.distance(root, 0, 0).value == 0);
	assert(singleton.jump(root, 0, 0, 0).value == 0);
	assert(singleton.jump(root, 0, 0, -1).value == -1);
	assert(singleton.jump(root, 0, 0, 1).value == -1);
	assert(singleton.root(singleton.make_root(root, 0), 0).value == 0);
	assert(singleton.lca(root, 0, 0, 0).value == 0);
	assert(singleton.get(singleton.set(root, 0, 8), 0) == 8);
	assert(singleton.get(root, 0) == 0);
}

void values_and_chain() {
	PersistentLinkCutTree<Value, combine, identity> custom(std::vector<Value>{Value(4), Value(7)});
	const auto before = custom.initial_root();
	const auto linked = custom.link(before, 0, 1).root;
	assert(custom.prod(linked, 0, 1).value.number == 11);
	assert(!custom.same(before, 0, 1).value);
	assert(custom.prod(custom.set(linked, 1, Value(-2)), 1, 0).value.number == 2);
	assert(custom.get(linked, 1).number == 7);
	PersistentLinkCutTree<Value, combine, identity> initialized(3);
	assert(initialized.get(initialized.initial_root(), 2).number == 0);
	PersistentLinkCutTree<bool, either, no, true> bits(std::vector<bool>{false, true});
	const auto connected = bits.link(bits.initial_root(), 0, 1).root;
	assert(bits.prod(connected, 1, 0).value);
	PersistentLinkCutTree<int, minimum, infinity, true> minima(3);
	auto root = minima.initial_root();
	root = minima.set(root, 0, 7);
	root = minima.set(root, 1, -3);
	root = minima.link(root, 0, 1).root;
	root = minima.link(root, 1, 2).root;
	assert(minima.prod(root, 2, 0).value == -3);
	assert(minima.prod(minima.set(root, 1, 9), 2, 0).value == 7);
	assert(minima.get(minima.initial_root(), 0) == infinity());

	constexpr int n = 4096;
	std::vector<std::pair<int, int>> edges;
	for (int i = 1; i < n; ++i) edges.emplace_back(i - 1, i);
	SumTree tree(std::vector<std::int64_t>(n, 1), edges);
	const auto cold = tree.initial_root();
	auto [warm, total] = tree.prod(cold, 0, n - 1);
	assert(total == n);
	assert(tree.path_size(cold, n - 1, 0).value == n);
	assert(tree.distance(warm, 0, n - 1).value == n - 1);
	assert(tree.jump(cold, 0, n - 1, n / 2).value == n / 2);
	assert(tree.jump(warm, n - 1, 0, n / 2).value == n / 2 - 1);
	assert(tree.jump(warm, 0, n - 1, -1).value == -1);
	assert(tree.jump(warm, 0, n - 1, n).value == -1);
	auto split = tree.cut(cold, n / 2 - 1, n / 2);
	assert(split.value && !tree.same(split.root, 0, n - 1).value);
	assert(tree.prod(cold, 0, n - 1).value == n);
	const auto changed = tree.set(warm, n / 2, 100);
	assert(tree.prod(changed, 0, n - 1).value == n + 99);
	assert(tree.get(cold, n / 2) == 1);
	const auto joined = tree.link(split.root, 0, n - 1);
	assert(joined.value && tree.prod(joined.root, n / 2 - 1, n / 2).value == n);
	assert(tree.lca(warm, n / 2, 0, n - 1).value == n / 2);
}
}

int main() {
	boundaries();
	branching();
	values_and_chain();
}
