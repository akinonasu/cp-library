#include "tree/rooted_tree_hash.hpp"
#include "tree/tree_hash.hpp"
#include <array>
#include <cassert>
#include <map>
#include <memory_resource>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <vector>

using Graph = std::vector<std::vector<int>>;

template<class T>
struct ProxyLabels {
	using value_type = T;
	struct reference {
		T value;
		operator T() const { return value; }
	};
	std::array<T, 4> values;
	std::size_t size() const { return values.size(); }
	reference operator[](std::size_t p) const { return {values[p]}; }
};

struct IndexedLabels {
	std::array<std::int64_t, 4> values;
	std::size_t size() const { return values.size(); }
	const std::int64_t &operator[](std::size_t p) const { return values[p]; }
};

struct IndexedLabelsWithAlias : IndexedLabels {
	using value_type = double;
};

void proxy_labels() {
	static_assert(tree_hash_detail::LabelSequence<ProxyLabels<bool>>);
	static_assert(tree_hash_detail::LabelSequence<ProxyLabels<std::int64_t>>);
	static_assert(tree_hash_detail::LabelSequence<IndexedLabels>);
	static_assert(tree_hash_detail::LabelSequence<IndexedLabelsWithAlias>);
	static_assert(std::is_same_v<tree_hash_detail::Label<std::vector<bool>>, bool>);
	Graph graph{ {1}, {0, 2}, {1, 3}, {2} };
	std::vector<int> parents{-1, 0, 1, 2};
	constexpr std::uint64_t seed = 67231;
	ProxyLabels<bool> bits{ {false, true, true, false} };
	std::vector<bool> plain(bits.values.begin(), bits.values.end());
	std::pmr::vector<bool> allocated(bits.values.begin(), bits.values.end());
	RootedTreeHash expected(graph, bits.values, 0, seed);
	assert(RootedTreeHash(graph, bits, 0, seed).hash == expected.hash);
	assert(RootedTreeHash(graph, plain, 0, seed).hash == expected.hash);
	assert(RootedTreeHash(graph, allocated, 0, seed).hash == expected.hash);
	RootedTreeHash from_parents;
	from_parents.build_parents(parents, bits, seed);
	assert(from_parents.hash == expected.hash);
	assert(tree_hash(graph, bits, seed) == tree_hash(graph, bits.values, seed));
	assert(tree_hash(graph, plain, seed) == tree_hash(graph, bits.values, seed));
	assert(tree_hash(graph, allocated, seed) == tree_hash(graph, bits.values, seed));
	ProxyLabels<std::int64_t> signed_values{ {INT64_MIN, -7, 257, INT64_MAX} };
	expected.build(graph, signed_values.values, 0, seed);
	assert(RootedTreeHash(graph, signed_values, 0, seed).hash == expected.hash);
	from_parents.build_parents(parents, signed_values, seed);
	assert(from_parents.hash == expected.hash);
	assert(tree_hash(graph, signed_values, seed) == tree_hash(graph, signed_values.values, seed));
	IndexedLabels direct{signed_values.values};
	assert(RootedTreeHash(graph, direct, 0, seed).hash == expected.hash);
	assert(tree_hash(graph, direct, seed) == tree_hash(graph, signed_values.values, seed));
	IndexedLabelsWithAlias alternate{direct};
	assert(RootedTreeHash(graph, alternate, 0, seed).hash == expected.hash);
}

template<class T>
std::string canonical(const Graph &graph, const std::vector<T> &labels, int v, int parent = -1) {
	std::vector<std::string> children;
	for (int u : graph[v]) if (u != parent) children.push_back(canonical(graph, labels, u, v));
	std::sort(children.begin(), children.end());
	std::string result = "(" + std::to_string(labels[v]) + ":";
	for (const auto &child : children) result += child;
	return result + ")";
}

template<class T>
void check(const Graph &graph, const std::vector<T> &labels, std::uint64_t seed, std::mt19937_64 &random) {
	const auto saved_graph = graph;
	const auto saved_labels = labels;
	int n = int(graph.size());
	if (!n) {
		assert(RootedTreeHash(graph, labels).empty());
		assert((tree_hash(graph, labels) == std::array<TreeHashValue, 2>{}));
		return;
	}
	std::vector<int> permutation(n);
	std::iota(permutation.begin(), permutation.end(), 0);
	std::shuffle(permutation.begin(), permutation.end(), random);
	Graph other(n);
	std::vector<T> other_labels(n);
	for (int v = 0; v < n; ++v) {
		other_labels[permutation[v]] = labels[v];
		for (int u : graph[v]) other[permutation[v]].push_back(permutation[u]);
	}
	for (auto &neighbors : other) std::shuffle(neighbors.begin(), neighbors.end(), random);
	std::map<std::string, TreeHashValue> codes;
	std::map<TreeHashValue, std::string> inverse;
	std::vector<TreeHashValue> centroid_hashes;
	for (int root = 0; root < n; ++root) {
		RootedTreeHash tree(graph, labels, root, seed);
		RootedTreeHash renamed(other, other_labels, permutation[root], seed);
		assert(tree.get() == renamed.get());
		RootedTreeHash from_parents;
		from_parents.build_parents(tree.parent, labels, seed);
		assert(tree.hash == from_parents.hash && tree.parent == from_parents.parent && tree.height == from_parents.height);
		std::vector<int> traversal{root};
		for (std::size_t i = 0; i < traversal.size(); ++i)
			for (int u : graph[traversal[i]]) if (u != tree.parent[traversal[i]]) traversal.push_back(u);
		auto ids = tree.classify();
		for (int v = 0; v < n; ++v) {
			auto shape = canonical(graph, labels, v, tree.parent[v]);
			assert(codes.try_emplace(shape, tree[v]).first->second == tree[v]);
			assert(inverse.try_emplace(tree[v], shape).first->second == shape);
			for (int u = 0; u < v; ++u) assert((ids[v] == ids[u]) == (tree[v] == tree[u]));
			assert(tree[v] == renamed[permutation[v]]);
		}
		int maximum = 0;
		for (int u : graph[root]) maximum = std::max(maximum, tree.subtree_size[u]);
		if (maximum <= n / 2) centroid_hashes.push_back(tree.get());
	}
	if (centroid_hashes.size() == 1) centroid_hashes.push_back(centroid_hashes[0]);
	assert(centroid_hashes.size() == 2);
	std::sort(centroid_hashes.begin(), centroid_hashes.end());
	auto hash = tree_hash(graph, labels, seed);
	assert(hash[0] == centroid_hashes[0] && hash[1] == centroid_hashes[1]);
	assert(hash == tree_hash(other, other_labels, seed));
	assert(graph == saved_graph && labels == saved_labels);
}

template<class T>
void typed(std::mt19937_64 &random) {
	std::vector<T> palette{std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max(), T(0), T(1), T(257)};
	for (int trial = 0; trial < 20; ++trial) {
		int n = int(random() % 19);
		Graph graph(n);
		for (int v = 1; v < n; ++v) {
			int u = int(random() % v);
			graph[v].push_back(u), graph[u].push_back(v);
		}
		std::vector<T> labels(n);
		for (auto &label : labels) label = palette[random() % palette.size()];
		check(graph, labels, 192301, random);
	}
}

int main() {
	proxy_labels();
	std::mt19937_64 random(102443);
	typed<signed char>(random); typed<unsigned char>(random);
	typed<short>(random); typed<unsigned short>(random);
	typed<int>(random); typed<unsigned int>(random);
	typed<long>(random); typed<unsigned long>(random);
	typed<long long>(random); typed<unsigned long long>(random);
	typed<char8_t>(random); typed<char16_t>(random); typed<char32_t>(random); typed<wchar_t>(random);
	Graph graph{ {1, 2}, {0}, {0} };
	std::array<std::int64_t, 3> labels{0, INT64_MIN, INT64_MAX};
	RootedTreeHash tree(graph, std::span(labels), 0, 123);
	assert(tree.get() == RootedTreeHash(graph, labels, 0, 123).get());
	assert(tree.get() == RootedTreeHash(graph, std::span<const std::int64_t, 3>(labels), 0, 123).get());
	std::vector<std::int64_t> a{0, 1, 257}, b{0, 1, 1};
	assert(tree_hash(graph, a, 123) != tree_hash(graph, b, 123));
	assert(RootedTreeHash(Graph{ {} }, std::vector<int>{1}, 0, 123).get() != RootedTreeHash(Graph{ {} }, std::vector<int>{257}, 0, 123).get());
	std::vector<bool> bits{false, true, false};
	RootedTreeHash boolean(graph, bits);
	assert(boolean[1] != boolean[2]);
	RootedTreeHash characters(graph, std::string("abb"));
	assert(characters[1] == characters[2]);
	std::vector<std::uint64_t> many(1025);
	std::iota(many.begin(), many.end(), 0);
	Graph star(many.size());
	for (int v = 1; v < int(star.size()); ++v) star[v].push_back(0), star[0].push_back(v);
	RootedTreeHash distinct(star, many, 0, 123);
	std::map<TreeHashValue, int> seen;
	for (int v = 1; v < distinct.size(); ++v) assert(seen.emplace(distinct[v], v).second);
}
