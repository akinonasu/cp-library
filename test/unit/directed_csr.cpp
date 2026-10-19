#include "components/directed_csr.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;

template<class T>
void check_graph(const DirectedCSR<T> &graph, const vector<vector<pair<int, T>>> &expected) {
	assert(graph.size() == int(expected.size()));
	assert(graph.empty() == expected.empty());
	size_t count = 0;
	const typename DirectedCSR<T>::Arc *base = nullptr;
	for (int v = 0; v < graph.size(); ++v) {
		auto row = graph[v];
		assert(row.size() == expected[v].size());
		assert(equal(row.begin(), row.end(), expected[v].begin()));
		if (!row.empty() && !base) base = row.data();
		if (base) assert(row.data() == base + count);
		count += row.size();
	}
	assert(graph.edge_count() == count);
}

template<class T>
void random_graphs(mt19937_64 &random) {
	using Graph = DirectedCSR<T>;
	for (int trial = 0; trial < 250; ++trial) {
		int n = int(random() % 41), m = n ? int(random() % 500) : 0;
		vector<typename Graph::Edge> edges;
		vector<vector<pair<int, T>>> expected(n);
		for (int i = 0; i < m; ++i) {
			int from = int(random() % n), to = int(random() % n);
			T weight;
			if constexpr (is_floating_point_v<T>) weight = T(random() % 1000) / 4;
			else weight = T(random());
			edges.push_back({from, to, weight});
			expected[from].emplace_back(to, weight);
		}
		auto original = edges;
		Graph graph(n, edges);
		for (size_t i = 0; i < edges.size(); ++i)
			assert(edges[i].from == original[i].from && edges[i].to == original[i].to && edges[i].weight == original[i].weight);
		check_graph(graph, expected);
		edges.clear();
		edges.shrink_to_fit();
		check_graph(graph, expected);
		Graph copied = graph;
		check_graph(copied, expected);
		Graph moved = std::move(copied);
		check_graph(moved, expected);
		copied.build(0, {});
		check_graph(copied, {});
		copied = graph;
		check_graph(copied, expected);
		moved = std::move(copied);
		check_graph(moved, expected);
		graph.build(n, original);
		check_graph(graph, expected);
	}
}

int main() {
	static_assert(is_same_v<DirectedCSR<uint32_t>::weight_type, uint32_t>);
	static_assert(is_same_v<DirectedCSR<uint32_t>::distance_type, int64_t>);
	static_assert(is_same_v<DirectedCSR<uint64_t>::distance_type, uint64_t>);
	static_assert(is_same_v<DirectedCSR<int64_t>::distance_type, int64_t>);
	static_assert(is_same_v<DirectedCSR<double>::distance_type, double>);
	static_assert(is_same_v<decltype(declval<DirectedCSR<int> &>()[0]), span<const DirectedCSR<int>::Arc>>);
	DirectedCSR<int> empty;
	check_graph(empty, {});
	empty.build(5, {});
	check_graph(empty, vector<vector<pair<int, int>>>(5));
	empty.build(0, {});
	check_graph(empty, {});
	using Graph = DirectedCSR<int>;
	vector<Graph::Edge> edges = {{2, 2, 7}, {0, 1, 9}, {0, 0, 4}, {2, 1, 5}, {0, 1, 3}};
	const vector<vector<pair<int, int>>> expected = {{{1, 9}, {0, 4}, {1, 3}}, {}, {{2, 7}, {1, 5}}, {}};
	Graph graph(4, edges);
	check_graph(graph, expected);
	auto row = graph[0];
	const auto &arc = row[1];
	const auto *data = row.data();
	Graph copied = graph;
	const array<Graph::Edge, 1> replacement = {{{0, 0, 99}}};
	copied.build(1, replacement);
	assert(graph[0].data() == data && arc == make_pair(0, 4));
	check_graph(graph, expected);
	check_graph(copied, {{{0, 99}}});
	edges[1] = {1, 3, -10};
	check_graph(graph, expected);
	DirectedCSR<string> text;
	{
		vector<DirectedCSR<string>::Edge> words = {{1, 0, "two"}, {0, 1, "one"}};
		text.build(2, words);
	}
	check_graph(text, {{{1, "one"}}, {{0, "two"}}});
	mt19937_64 random(982731);
	random_graphs<int>(random);
	random_graphs<uint32_t>(random);
	random_graphs<uint64_t>(random);
	random_graphs<double>(random);
}
