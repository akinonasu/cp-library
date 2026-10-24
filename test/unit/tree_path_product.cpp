#include "tree/path_product.hpp"
#include <cstdint>
#include <numeric>
#include <random>
#include <string>

std::string concatenate(std::string a, const std::string& b) { return a + b; }
std::string empty_string() { return {}; }
std::string reverse_string(std::string value) { std::reverse(value.begin(), value.end()); return value; }
std::int64_t sum(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t zero() { return 0; }
std::int64_t same(std::int64_t value) { return value; }

int main() {
	HLD empty;
	empty.build();
	TreePathProduct<std::string, concatenate, empty_string> initial(empty, {});
	assert(initial.empty() && initial.size() == 0);
	HLD words(3);
	words.add_edge(0, 1), words.add_edge(1, 2);
	words.build();
	TreePathProduct<std::string, concatenate, empty_string> word_values(words, {"abc", "de", "fg"});
	assert(word_values.prod(2, 0) == "fgdeabc");
	std::mt19937 rng(89271);
	for (int trial = 0; trial < 250; ++trial) {
		int n = 1 + rng() % 150;
		HLD tree(n);
		std::vector<int> labels(n);
		std::iota(labels.begin(), labels.end(), 0);
		std::shuffle(labels.begin(), labels.end(), rng);
		for (int v = 1; v < n; ++v) {
			int p = trial % 5 == 0 ? v - 1 : trial % 5 == 1 ? 0 : trial % 5 == 2 ? (v - 1) / 2 : rng() % v;
			tree.add_edge(labels[p], labels[v]);
		}
		tree.build(rng() % n);
		std::vector<std::string> values(n);
		for (int v = 0; v < n; ++v) values[v] = char('a' + v % 26);
		TreePathProduct<std::string, concatenate, empty_string> plain(tree, values);
		TreePathProduct<std::string, concatenate, empty_string, reverse_string> reversible(tree, values);
		assert(plain.size() == n && !plain.empty());
		for (int q = 0; q < 500; ++q) {
			int u = rng() % n, v = rng() % n;
			if (q % 5 == 0) {
				values[u] = char('A' + rng() % 26);
				plain.set(u, values[u]), reversible.set(u, values[u]);
			}
			assert(plain.get(u) == values[u] && reversible.get(u) == values[u]);
			std::string expected;
			int lca = tree.lca(u, v);
			for (int x = u; x != lca; x = tree.parent[x]) expected += values[x];
			expected += values[lca];
			std::vector<int> suffix;
			for (int x = v; x != lca; x = tree.parent[x]) suffix.push_back(x);
			for (auto it = suffix.rbegin(); it != suffix.rend(); ++it) expected += values[*it];
			assert(plain.prod(u, v) == expected && reversible.prod(u, v) == expected);
			auto apply = [](const std::string& word, std::string value) { return value + word; };
			assert(plain.apply(u, v, std::string("0"), apply) == "0" + expected);
			assert(reversible.apply(u, v, std::string("0"), apply) == "0" + expected);
		}
		auto copy = reversible;
		copy.set(0, "!");
		assert(copy.get(0) == "!" && reversible.get(0) == values[0]);
		plain.build(empty, {});
		assert(plain.empty());
	}
	for (int shape = 0; shape < 3; ++shape) {
		constexpr int n = 300000;
		HLD tree(n);
		for (int v = 1; v < n; ++v) tree.add_edge(shape == 0 ? v - 1 : shape == 1 ? 0 : (v - 1) / 2, v);
		tree.build(n / 3);
		TreePathProduct<std::int64_t, sum, zero, same> values(tree, std::vector<std::int64_t>(n, 1));
		for (int q = 0; q < 1000; ++q) {
			int u = rng() % n, v = rng() % n;
			assert(values.prod(u, v) == tree.dist(u, v) + 1);
			values.set(u, 10);
			assert(values.prod(u, v) == tree.dist(u, v) + 10);
			values.set(u, 1);
		}
	}
}
