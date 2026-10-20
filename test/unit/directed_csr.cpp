#include "components/directed_csr.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;

struct alignas(128) AlignedWeight {
	uint64_t value;
	bool operator==(const AlignedWeight &) const = default;
};

struct TrackedWeight {
	static inline int live = 0, copies = -1;
	int value = 0;
	TrackedWeight() { ++live; }
	explicit TrackedWeight(int x) : value(x) { ++live; }
	TrackedWeight(const TrackedWeight &other) : value(other.value) {
		if (copies == 0) throw runtime_error("weight copy");
		if (copies > 0) --copies;
		++live;
	}
	TrackedWeight(TrackedWeight &&other) noexcept : value(other.value) { ++live; }
	TrackedWeight &operator=(const TrackedWeight &other) {
		if (copies == 0) throw runtime_error("weight copy");
		if (copies > 0) --copies;
		value = other.value;
		return *this;
	}
	TrackedWeight &operator=(TrackedWeight &&other) noexcept {
		value = other.value;
		return *this;
	}
	~TrackedWeight() { --live; }
	bool operator==(const TrackedWeight &) const = default;
};

template<class T>
void check_graph(const DirectedCSR<T> &graph, const vector<vector<pair<int, T>>> &expected) {
	assert(graph.size() == int(expected.size()));
	assert(graph.empty() == expected.empty());
	size_t count = 0;
	vector<unsigned> incoming(expected.size());
	bool unique_incoming = true;
	const typename DirectedCSR<T>::Arc *base = nullptr;
	for (int v = 0; v < graph.size(); ++v) {
		auto row = graph[v];
		assert(row.size() == expected[v].size());
		assert(equal(row.begin(), row.end(), expected[v].begin()));
		for (const auto &arc : expected[v]) if (++incoming[arc.first] > 1) unique_incoming = false;
		if (!row.empty() && !base) base = row.data();
		if (base) assert(row.data() == base + count);
		count += row.size();
	}
	assert(graph.edge_count() == count);
	assert(directed_csr_detail::Access::unique_incoming(graph) == unique_incoming);
	if (directed_csr_detail::Access::has_single_edge_layout(graph)) {
		auto direct = directed_csr_detail::Access::single_edge_view(graph);
		assert(direct.size() == graph.size());
		assert(count <= expected.size() && expected.size() - count <= 1);
		for (int v = 0; v < graph.size(); ++v) {
			assert(expected[v].size() <= 1);
			assert(equal(direct[v].begin(), direct[v].end(), expected[v].begin(), expected[v].end()));
			if (!direct[v].empty()) assert(direct[v].data() == graph[v].data());
		}
	}
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
		std::size_t emitted = 0;
		Graph produced(n, edges.size(), [&](auto emit) {
			for (const auto &edge : edges) {
				emit(edge);
				++emitted;
			}
		});
		assert(emitted == edges.size());
		check_graph(produced, expected);
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
		produced.build(n, original.size(), [&](auto emit) {
			for (const auto &edge : original) emit(edge);
		});
		check_graph(produced, expected);
	}
}

template<class T>
void sparse_graphs() {
	using Graph = DirectedCSR<T>;
	constexpr int n = 65536;
	Graph produced;
	for (int m : {0, 31, 32, 33, 65, 4099, 16385}) {
		for (int mode = 0; mode < 4; ++mode) {
			vector<typename Graph::Edge> edges;
			vector<vector<pair<int, T>>> expected(n);
			for (int i = 0; i < m; ++i) {
				const array<int, 5> starts = {0, n - 1, n / 2, 1, n - 2};
				int from = mode == 0 ? 0 : mode == 1 ? n - 1 : starts[i % starts.size()];
				if (mode == 3) from = i % 4 == 0 ? 0 : i + 1;
				int to = i % 7 == 0 ? n - 1 : (i * 7919 + 13) % n;
				T weight;
				if constexpr (is_same_v<T, string>) weight = to_string(i) + string(40, char('a' + i % 26));
				else weight = i % 2 ? -i : i;
				edges.push_back({from, to, weight});
				expected[from].emplace_back(to, weight);
			}
			Graph graph(n, edges);
			check_graph(graph, expected);
			int calls = 0;
			produced.build(n, edges.size(), [&](auto emit) {
				++calls;
				for (const auto &edge : edges) emit(edge);
			});
			assert(calls == 1);
			check_graph(produced, expected);
			Graph copied = graph;
			Graph moved = std::move(copied);
			check_graph(moved, expected);
			check_graph(copied, {});
			copied = produced;
			check_graph(copied, expected);
			moved = std::move(copied);
			check_graph(moved, expected);
			copied.build(2, {});
			check_graph(copied, vector<vector<pair<int, T>>>(2));
			copied.build(n, edges);
			check_graph(copied, expected);
			edges.clear();
			edges.shrink_to_fit();
			check_graph(graph, expected);
			check_graph(produced, expected);
		}
	}
	bool failed = false;
	try {
		produced.build(n, 65, [&](auto emit) {
			for (int i = 0; i < 33; ++i) emit(typename Graph::Edge{i % 2 ? 0 : n - 1, n - 1, T{}});
			throw runtime_error("producer");
		});
	} catch (const runtime_error &) { failed = true; }
	assert(failed);
	check_graph(produced, vector<vector<pair<int, T>>>(n));
	produced.build(0, 0, [](auto) {});
	check_graph(produced, {});
}

void aligned_weights() {
	static_assert(is_trivial_v<AlignedWeight>);
	using Graph = DirectedCSR<AlignedWeight>;
	constexpr int n = 8193;
	vector<Graph::Edge> edges;
	vector<vector<pair<int, AlignedWeight>>> expected(n);
	for (int i = 0; i <= n; ++i) {
		int from = (i * 7919 + 23) % n, to = (i * 31 + 7) % n;
		edges.push_back({from, to, {uint64_t(i) * 999983}});
		expected[from].emplace_back(to, edges.back().weight);
	}
	Graph graph(n, edges);
	check_graph(graph, expected);
	for (int v = 0; v < n; ++v)
		assert(reinterpret_cast<uintptr_t>(graph[v].data()) % alignof(Graph::Arc) == 0);
	Graph copied = graph;
	Graph moved = std::move(copied);
	check_graph(moved, expected);
	check_graph(copied, {});
	const array<Graph::Edge, 3> small = { Graph::Edge{2, 0, {31}}, Graph::Edge{0, 1, {17}}, Graph::Edge{1, 2, {19}} };
	graph.build(3, small.size(), [&](auto emit) { for (const auto &edge : small) emit(edge); });
	check_graph(graph, { {{1, {17}}}, {{2, {19}}}, {{0, {31}}} });
	graph.build(0, {});
	check_graph(graph, {});
	graph.build(n, edges.size(), [&](auto emit) { for (const auto &edge : edges) emit(edge); });
	check_graph(graph, expected);
	copied = graph;
	check_graph(copied, expected);
}

void throwing_weights() {
	static_assert(!is_trivial_v<TrackedWeight>);
	static_assert(!is_nothrow_copy_constructible_v<TrackedWeight>);
	assert(TrackedWeight::live == 0);
	for (bool single : {false, true}) {
		using Graph = DirectedCSR<TrackedWeight>;
		vector<Graph::Edge> edges;
		const array<pair<int, int>, 5> general = { pair{2, 2}, pair{0, 1}, pair{0, 0}, pair{2, 1}, pair{0, 1} };
		const array<pair<int, int>, 3> direct = { pair{3, 1}, pair{0, 2}, pair{2, 3} };
		for (int i = 0; i < (single ? 3 : 5); ++i) {
			auto [from, to] = single ? direct[i] : general[i];
			edges.push_back({from, to, TrackedWeight(i + 7)});
		}
		vector<vector<pair<int, TrackedWeight>>> expected(4);
		for (const auto &edge : edges) expected[edge.from].emplace_back(edge.to, edge.weight);
		Graph graph(4, edges);
		check_graph(graph, expected);
		int before = TrackedWeight::live;
		bool failed = false;
		TrackedWeight::copies = 1;
		try { Graph copied(graph); }
		catch (const runtime_error &) { failed = true; }
		TrackedWeight::copies = -1;
		assert(failed && TrackedWeight::live == before);
		check_graph(graph, expected);
		for (bool producer : {false, true}) {
			graph.build(4, edges);
			before = TrackedWeight::live - int(graph.edge_count());
			failed = false;
			try {
				if (producer) graph.build(4, edges.size(), [&](auto emit) {
					for (const auto &edge : edges) emit(edge);
					TrackedWeight::copies = 1;
				});
				else {
					TrackedWeight::copies = 1;
					graph.build(4, edges);
				}
			} catch (const runtime_error &) { failed = true; }
			TrackedWeight::copies = -1;
			assert(failed && TrackedWeight::live == before);
			check_graph(graph, vector<vector<pair<int, TrackedWeight>>>(4));
			graph.build(4, edges);
			check_graph(graph, expected);
		}
	}
	assert(TrackedWeight::live == 0);
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
	int producer_calls = 0;
	Graph produced(4, edges.size(), [&](auto emit) {
		++producer_calls;
		for (auto edge : edges) emit(edge);
	});
	assert(producer_calls == 1);
	check_graph(produced, expected);
	produced.build(0, 0, [&](auto) { ++producer_calls; });
	assert(producer_calls == 2);
	check_graph(produced, {});
	bool short_input = false, long_input = false;
	try { Graph invalid(1, 1, [](auto) {}); }
	catch (const length_error &) { short_input = true; }
	try { Graph invalid(1, 0, [](auto emit) { emit(Graph::Edge{0, 0, 0}); }); }
	catch (const length_error &) { long_input = true; }
	assert(short_input && long_input);
	produced = graph;
	try { produced.build(4, 2, [](auto emit) { emit(Graph::Edge{2, 0, 1}); }); }
	catch (const length_error &) {}
	check_graph(produced, vector<vector<pair<int, int>>>(4));
	produced = graph;
	try {
		produced.build(4, 1, [](auto emit) {
			emit(Graph::Edge{2, 0, 1});
			emit(Graph::Edge{2, 0, 1});
		});
	} catch (const length_error &) {}
	check_graph(produced, vector<vector<pair<int, int>>>(4));
	produced = graph;
	bool callback_failed = false;
	try {
		produced.build(4, 2, [](auto emit) {
			emit(Graph::Edge{2, 0, 1});
			throw runtime_error("producer");
		});
	} catch (const runtime_error &) { callback_failed = true; }
	assert(callback_failed);
	check_graph(produced, vector<vector<pair<int, int>>>(4));
	produced.build(4, edges);
	check_graph(produced, expected);
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
	text.build(2, 2, [](auto emit) {
		emit(DirectedCSR<string>::Edge{1, 0, "two"});
		emit(DirectedCSR<string>::Edge{0, 1, "one"});
	});
	check_graph(text, {{{1, "one"}}, {{0, "two"}}});
	for (int n = 0; n <= 40; ++n) {
		for (int gap = -1; gap < n; ++gap) {
			vector<Graph::Edge> single_edges;
			vector<vector<pair<int, int>>> single_expected(n);
			for (int v = 0; v < n; ++v) if (v != gap) {
				int to = (v * 7 + 3) % n;
				single_edges.push_back({v, to, v - 5});
				single_expected[v].emplace_back(to, v - 5);
			}
			reverse(single_edges.begin(), single_edges.end());
			graph.build(n, single_edges);
			assert(directed_csr_detail::Access::has_single_edge_layout(graph));
			check_graph(graph, single_expected);
			produced.build(n, single_edges.size(), [&](auto emit) { for (const auto &edge : single_edges) emit(edge); });
			assert(directed_csr_detail::Access::has_single_edge_layout(produced));
			check_graph(produced, single_expected);
			copied = graph;
			Graph moved_graph = std::move(copied);
			assert(!directed_csr_detail::Access::has_single_edge_layout(copied));
			check_graph(moved_graph, single_expected);
			copied.build(4, edges);
			assert(!directed_csr_detail::Access::has_single_edge_layout(copied));
		}
	}
	vector<Graph::Edge> unique_edges = {{0, 63, 1}, {0, 64, 2}, {1, 127, 3}, {129, 0, 4}, {65, 65, 5}};
	vector<vector<pair<int, int>>> unique_expected(130);
	for (const auto &edge : unique_edges) unique_expected[edge.from].emplace_back(edge.to, edge.weight);
	graph.build(130, unique_edges);
	assert(directed_csr_detail::Access::unique_incoming(graph));
	check_graph(graph, unique_expected);
	unique_edges.push_back({2, 64, -7});
	unique_expected[2].emplace_back(64, -7);
	produced.build(130, unique_edges.size(), [&](auto emit) { for (const auto &edge : unique_edges) emit(edge); });
	assert(!directed_csr_detail::Access::unique_incoming(produced));
	check_graph(produced, unique_expected);
	copied = produced;
	assert(!directed_csr_detail::Access::unique_incoming(copied));
	Graph moved_duplicate = std::move(copied);
	check_graph(moved_duplicate, unique_expected);
	check_graph(copied, {});
	try {
		produced.build(130, 3, [](auto emit) {
			emit(Graph::Edge{0, 64, 0});
			emit(Graph::Edge{1, 64, 0});
			throw runtime_error("producer");
		});
	} catch (const runtime_error &) {}
	check_graph(produced, vector<vector<pair<int, int>>>(130));
	produced.build(130, std::span<const Graph::Edge>(unique_edges).first(5));
	assert(directed_csr_detail::Access::unique_incoming(produced));
	mt19937_64 random(982731);
	random_graphs<int>(random);
	random_graphs<uint32_t>(random);
	random_graphs<uint64_t>(random);
	random_graphs<double>(random);
	sparse_graphs<int>();
	sparse_graphs<string>();
	aligned_weights();
	throwing_weights();
}
