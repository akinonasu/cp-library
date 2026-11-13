#include "tree/edge_link_cut_tree.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

constexpr std::uint64_t mod = 998244353;
struct Affine {
	std::uint64_t a, b;
	bool operator==(const Affine &) const = default;
};
Affine compose(Affine f, Affine g) { return {f.a * g.a % mod, (f.b * g.a + g.b) % mod}; }
Affine identity() { return {1, 0}; }
std::string concat(const std::string &a, const std::string &b) { return a + b; }
std::string empty_string() { return {}; }
std::int64_t add(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t zero() { return 0; }
using AffineTree = EdgeLinkCutTree<Affine, compose, identity>;
using WordTree = EdgeLinkCutTree<std::string, concat, empty_string>;
using SumTree = EdgeLinkCutTree<std::int64_t, add, zero>;

struct Model {
	struct Edge {
		int from, to;
		Affine forward, backward;
		std::string forward_word, backward_word;
	};
	std::vector<std::vector<std::optional<Edge>>> edges;
	explicit Model(int n) : edges(n, std::vector<std::optional<Edge>>(n)) {}
	int size() const { return int(edges.size()); }
	int count() const {
		int count = 0;
		for (int u = 0; u < size(); ++u) for (int v = u + 1; v < size(); ++v) count += bool(edges[u][v]);
		return count;
	}
	std::vector<int> path(int from, int to) const {
		std::vector<int> parent(size(), -1), queue{from};
		parent[from] = from;
		for (std::size_t i = 0; i < queue.size(); ++i)
			for (int v = 0; v < size(); ++v) if (edges[queue[i]][v] && parent[v] == -1) {
				parent[v] = queue[i];
				queue.push_back(v);
			}
		if (parent[to] == -1) return {};
		std::vector<int> result{to};
		while (result.back() != from) result.push_back(parent[result.back()]);
		std::reverse(result.begin(), result.end());
		return result;
	}
	std::uint64_t evaluate(const std::vector<int> &path, std::uint64_t value) const {
		for (std::size_t i = 1; i < path.size(); ++i) {
			const auto &edge = *edges[path[i - 1]][path[i]];
			const auto &f = edge.from == path[i - 1] ? edge.forward : edge.backward;
			value = (f.a * value + f.b) % mod;
		}
		return value;
	}
	std::string word(const std::vector<int> &path) const {
		std::string result;
		for (std::size_t i = 1; i < path.size(); ++i) {
			const auto &edge = *edges[path[i - 1]][path[i]];
			result += edge.from == path[i - 1] ? edge.forward_word : edge.backward_word;
		}
		return result;
	}
	void assign(const Edge &edge) { edges[edge.from][edge.to] = edges[edge.to][edge.from] = edge; }
};

void check_path(AffineTree &affines, WordTree &words, const Model &model, int u, int v) {
	auto path = model.path(u, v);
	assert(affines.same(u, v) == !path.empty());
	assert(words.same(u, v) == !path.empty());
	if (path.empty()) return;
	Affine actual = affines.prod(u, v);
	auto b = model.evaluate(path, 0), at_one = model.evaluate(path, 1);
	assert(actual.b == b && actual.a == (at_one + mod - b) % mod);
	for (std::uint64_t initial : {0ULL, 1ULL, 17ULL, 998244352ULL})
		assert((actual.a * initial + actual.b) % mod == model.evaluate(path, initial));
	assert(words.prod(u, v) == model.word(path));
	assert(affines.path_size(u, v) == int(path.size()) && words.path_size(u, v) == int(path.size()));
	assert(affines.distance(u, v) == int(path.size()) - 1 && words.distance(u, v) == int(path.size()) - 1);
}

void check_get(AffineTree &affines, WordTree &words, const Model &model, int u, int v) {
	auto a = affines.get(u, v);
	auto b = words.get(u, v);
	assert(bool(a) == bool(model.edges[u][v]) && bool(b) == bool(a));
	if (!a) return;
	const auto &expected = *model.edges[u][v];
	assert(a->from == expected.from && a->to == expected.to);
	assert(a->forward == expected.forward && a->backward == expected.backward);
	assert(b->from == expected.from && b->to == expected.to);
	assert(b->forward == expected.forward_word && b->backward == expected.backward_word);
}

void random_cases() {
	std::mt19937 random(20261113);
	auto function = [&] { return Affine{random() % 7, random() % mod}; };
	auto text = [&] {
		std::string result;
		for (unsigned i = random() % 4; i; --i) result += char('A' + random() % 26);
		return result;
	};
	for (int trial = 0; trial < 24; ++trial) {
		int n = 1 + random() % 16;
		AffineTree affines(n);
		WordTree words(n);
		Model model(n);
		for (int step = 0; step < 1400; ++step) {
			int u = random() % n, v = random() % n, type = random() % 12;
			if (type <= 4 && (random() & 1)) {
				std::vector<std::pair<int, int>> candidates;
				for (int a = 0; a < n; ++a) for (int b = a + 1; b < n; ++b)
					if (model.edges[a][b]) candidates.emplace_back(a, b);
				if (!candidates.empty()) {
					auto edge = candidates[random() % candidates.size()];
					u = edge.first, v = edge.second;
					if (random() & 1) std::swap(u, v);
				}
			}
			if (type == 0 || type == 11) {
				if (type == 11 && model.edges[u][v]) {
					assert(affines.cut(u, v) && words.cut(u, v));
					model.edges[u][v].reset(), model.edges[v][u].reset();
					std::swap(u, v);
				}
				bool expected = model.path(u, v).empty(), symmetric = random() % 3 == 0;
				Model::Edge edge{u, v, function(), function(), text(), text()};
				if (symmetric) edge.backward = edge.forward, edge.backward_word = edge.forward_word;
				bool a = symmetric ? affines.link(u, v, edge.forward) : affines.link(u, v, edge.forward, edge.backward);
				bool b = symmetric ? words.link(u, v, edge.forward_word) : words.link(u, v, edge.forward_word, edge.backward_word);
				assert(a == expected && b == expected);
				if (expected) model.assign(edge);
			} else if (type == 1) {
				bool expected = bool(model.edges[u][v]);
				assert(affines.cut(u, v) == expected && words.cut(u, v) == expected);
				if (expected) model.edges[u][v].reset(), model.edges[v][u].reset();
			} else if (type == 2 || type == 3) {
				bool expected = bool(model.edges[u][v]);
				Affine forward = function(), backward = type == 3 ? forward : function();
				std::string forward_word = text(), backward_word = type == 3 ? forward_word : text();
				bool a = type == 3 ? affines.set(u, v, forward) : affines.set(u, v, forward, backward);
				bool b = type == 3 ? words.set(u, v, forward_word) : words.set(u, v, forward_word, backward_word);
				assert(a == expected && b == expected);
				if (expected) {
					auto edge = *model.edges[u][v];
					edge.forward = forward, edge.backward = backward;
					edge.forward_word = forward_word, edge.backward_word = backward_word;
					model.assign(edge);
				}
			} else if (type == 4) check_get(affines, words, model, u, v);
			else if (type == 5) {
				check_path(affines, words, model, u, v);
				check_path(affines, words, model, v, u);
			} else if (type == 6) {
				bool expected = !model.path(u, v).empty();
				assert(affines.same(u, v) == expected && words.same(u, v) == expected);
			} else if (type == 7) {
				auto path = model.path(u, v);
				if (!path.empty()) {
					int k = int(random() % (path.size() + 4)) - 2;
					int expected = 0 <= k && k < int(path.size()) ? path[k] : -1;
					assert(affines.jump(u, v, k) == expected && words.jump(u, v, k) == expected);
				}
			} else if (type == 8) {
				affines.make_root(u), words.make_root(u);
				assert(affines.root(u) == u && words.root(u) == u);
				if (!model.path(u, v).empty()) assert(affines.root(v) == u && words.root(v) == u);
			} else if (type == 9) {
				int r = random() % n;
				auto a = model.path(r, u), b = model.path(r, v);
				if (!a.empty() && !b.empty()) {
					int i = 0;
					while (i < int(std::min(a.size(), b.size())) && a[i] == b[i]) ++i;
					assert(affines.lca(r, u, v) == a[i - 1] && words.lca(r, u, v) == a[i - 1]);
				}
			} else check_path(affines, words, model, u, v);
			assert(affines.size() == n && words.size() == n);
			assert(affines.edge_count() == model.count() && words.edge_count() == model.count());
			if (step % 100 == 0) {
				for (int a = 0; a < n; ++a) {
					check_get(affines, words, model, a, a);
					check_path(affines, words, model, a, a);
				}
			}
		}
		auto copied_affines = affines;
		auto copied_words = words;
		for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) {
			check_get(copied_affines, copied_words, model, u, v);
			check_path(copied_affines, copied_words, model, u, v);
		}
	}
}

void fixed_cases() {
	AffineTree empty;
	assert(empty.size() == 0 && empty.edge_count() == 0);
	AffineTree single(1);
	assert(single.prod(0, 0) == identity() && single.distance(0, 0) == 0 && single.path_size(0, 0) == 1);
	assert(!single.link(0, 0, identity()) && !single.cut(0, 0) && !single.set(0, 0, identity()) && !single.get(0, 0));
	assert(single.jump(0, 0, 0) == 0 && single.jump(0, 0, 1) == -1);
	AffineTree tree(3);
	assert(tree.link(0, 1, Affine{2, 1}, Affine{0, 7}));
	assert(tree.link(1, 2, Affine{3, 4}, Affine{5, 6}));
	auto f = tree.prod(0, 2), g = tree.prod(2, 0);
	assert((f.a * 10 + f.b) % mod == 67 && g.a == 0 && g.b == 7);
	assert(!tree.link(2, 0, identity()) && tree.edge_count() == 2);
	assert(tree.set(1, 0, Affine{4, 3}, Affine{0, 9}));
	f = tree.prod(0, 2), g = tree.prod(2, 0);
	assert((f.a * 2 + f.b) % mod == 37 && g.a == 0 && g.b == 9);
	assert(tree.cut(2, 1) && !tree.same(0, 2));
	assert(!tree.set(1, 2, identity()) && !tree.get(2, 1));
	assert(tree.prod(2, 2) == identity());
	assert(tree.link(2, 1, Affine{0, 8}, Affine{7, 2}));
	assert(tree.set(1, 2, Affine{6, 5}, Affine{2, 4}));
	f = tree.prod(0, 2), g = tree.prod(2, 0);
	assert((f.a * 2 + f.b) % mod == 26 && g.a == 0 && g.b == 9);
	auto edge = tree.get(1, 2);
	assert(edge && edge->from == 2 && edge->to == 1 && edge->forward == (Affine{6, 5}) && edge->backward == (Affine{2, 4}));
	WordTree words(3);
	assert(words.link(0, 1, "A", "a") && words.link(1, 2, "B", "b"));
	words.make_root(2);
	assert(words.prod(0, 2) == "AB" && words.prod(2, 0) == "ba");
	assert(words.set(1, 0, "X", "y"));
	assert(words.prod(0, 2) == "XB" && words.prod(2, 0) == "by");
}

void copying_and_reuse() {
	SumTree original(5);
	assert(original.link(0, 1, 2, 3) && original.link(1, 2, 5, 7) && original.link(2, 3, 11, 13));
	assert(original.prod(3, 0) == 23 && original.cut(2, 3));
	auto copy = original;
	assert(copy.set(2, 1, 17, 19) && copy.prod(0, 2) == 19);
	assert(original.prod(0, 2) == 7);
	assert(copy.link(4, 3, 23, 29) && !original.same(3, 4));
	assert(copy.link(2, 3, 31, 37) && copy.edge_count() == 4);
	SumTree assigned(1);
	assigned = copy;
	assert(assigned.size() == 5 && assigned.prod(0, 4) == 79);
	auto moved = std::move(copy);
	assert(copy.size() == 0 && copy.edge_count() == 0 && moved.prod(0, 4) == 79);
	SumTree move_assigned(2);
	assert(move_assigned.link(0, 1, 1));
	move_assigned = std::move(moved);
	assert(moved.size() == 0 && moved.edge_count() == 0 && move_assigned.prod(4, 0) == 82);
	for (int repeat = 0; repeat < 6000; ++repeat) {
		assert(assigned.cut(3, 4));
		bool reversed = repeat & 1;
		int from = reversed ? 3 : 4, to = reversed ? 4 : 3;
		assert(assigned.link(from, to, repeat + 1, -repeat - 2));
		assigned.make_root(repeat % 5);
		auto edge = assigned.get(to, from);
		assert(edge && edge->from == from && edge->to == to);
		assert(edge->forward == repeat + 1 && edge->backward == -repeat - 2);
		assert(assigned.prod(from, to) == repeat + 1 && assigned.prod(to, from) == -repeat - 2);
	}
	assert(move_assigned.prod(0, 4) == 79);
}

struct Value {
	int value;
	explicit Value(int x) : value(x) {}
	friend void swap(Value &a, Value &b) { std::swap(a.value, b.value); }
};

void custom_value() {
	constexpr auto combine = [](Value a, Value b) { return Value(a.value + b.value); };
	constexpr auto empty = [] { return Value(0); };
	EdgeLinkCutTree<Value, combine, empty> tree(2);
	assert(tree.link(0, 1, Value(4), Value(7)));
	assert(tree.prod(0, 1).value == 4 && tree.prod(1, 0).value == 7);
	assert(tree.set(1, 0, Value(5), Value(9)));
	auto edge = tree.get(1, 0);
	assert(edge && edge->forward.value == 5 && edge->backward.value == 9);
	assert(tree.prod(0, 1).value == 5 && tree.prod(1, 0).value == 9);
}

void long_chain() {
	const int n = 200000, middle = n / 2;
	SumTree tree(n);
	for (int v = 1; v < n; ++v) assert(tree.link(v - 1, v, 1, -2));
	assert(tree.edge_count() == n - 1 && tree.prod(0, n - 1) == n - 1);
	assert(tree.prod(n - 1, 0) == -2LL * (n - 1));
	assert(tree.distance(0, n - 1) == n - 1 && tree.path_size(0, n - 1) == n);
	assert(tree.jump(0, n - 1, middle) == middle);
	assert(tree.jump(n - 1, 0, middle) == middle - 1);
	assert(tree.jump(0, n - 1, std::numeric_limits<int>::max()) == -1);
	assert(tree.jump(0, n - 1, std::numeric_limits<int>::min()) == -1);
	assert(tree.lca(middle, 0, n - 1) == middle);
	tree.make_root(n - 1);
	assert(tree.root(0) == n - 1);
	assert(tree.cut(middle, middle - 1) && !tree.same(0, n - 1));
	assert(tree.link(middle, middle - 1, 3, 5));
	assert(tree.prod(0, n - 1) == n + 3 && tree.prod(n - 1, 0) == -2LL * (n - 2) + 3);
	assert(tree.set(middle - 1, middle, 7, 11));
	assert(tree.prod(0, n - 1) == n + 9 && tree.prod(n - 1, 0) == -2LL * (n - 2) + 7);
	auto edge = tree.get(middle - 1, middle);
	assert(edge && edge->from == middle && edge->to == middle - 1 && edge->forward == 7 && edge->backward == 11);
}

int main() {
	fixed_cases();
	random_cases();
	copying_and_reuse();
	custom_value();
	long_chain();
}
