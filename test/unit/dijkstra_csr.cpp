#include "graph/shortest_path/dijkstra.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;

template<class W>
void check_csr(mt19937_64 &random) {
	using Graph = DirectedCSR<W>;
	using D = typename Graph::distance_type;
	for (int trial = 0; trial < 200; ++trial) {
		int n = 1 + int(random() % 16), m = int(random() % (n * n + 1));
		vector<typename Graph::Edge> edges;
		vector<vector<pair<int, D>>> expected(n);
		for (int i = 0; i < m; ++i) {
			int from = int(random() % n), to = int(random() % n);
			W weight;
			if constexpr (is_floating_point_v<W>) weight = W(random() % 1000) / 4;
			else weight = trial % 3 == 0 ? W(random() % 2) : W(random() % 1000000001);
			edges.push_back({from, to, weight});
			expected[from].emplace_back(to, D(weight));
		}
		Graph graph(n, edges);
		for (int source = 0; source < n; ++source) {
			auto distances = dijkstra(expected, source);
			assert(dijkstra(graph, source) == distances);
			for (int target = 0; target < n; ++target) {
				auto [distance, path] = dijkstra_path(graph, source, target);
				assert(distance == distances[target]);
				if (distance == D(-1)) {
					assert(path.empty());
					continue;
				}
				assert(!path.empty() && path.front() == source && path.back() == target && path.size() <= size_t(n));
				D sum = 0;
				for (size_t i = 1; i < path.size(); ++i) {
					D weight = shortest_path_detail::infinity<D>();
					for (const auto &[to, value] : graph[path[i - 1]])
						if (to == path[i]) weight = min(weight, D(value));
					assert(weight < shortest_path_detail::infinity<D>());
					sum += weight;
				}
				assert(sum == distance);
			}
		}
	}
}

int main() {
	using Graph = DirectedCSR<uint32_t>;
	static_assert(is_same_v<decltype(dijkstra(declval<const Graph &>(), 0)), vector<int64_t>>);
	static_assert(is_same_v<decltype(dijkstra_path(declval<const Graph &>(), 0, 0)), pair<int64_t, vector<int>>>);
	vector<Graph::Edge> edges;
	for (int v = 0; v < 7; ++v) edges.push_back({v, v + 1, 1000000000});
	Graph line(9, edges);
	assert((dijkstra(line, 0) == vector<int64_t>{0, 1000000000, 2000000000, 3000000000, 4000000000,
		5000000000, 6000000000, 7000000000, -1}));
	assert((dijkstra_path(line, 0, 7) == pair<int64_t, vector<int>>{7000000000, {0, 1, 2, 3, 4, 5, 6, 7}}));
	assert((dijkstra_path(line, 0, 8) == pair<int64_t, vector<int>>{-1, {}}));
	assert((dijkstra_path(line, 3, 3) == pair<int64_t, vector<int>>{0, {3}}));
	vector<Graph::Edge> narrow_edges{{0, 1, 4000000000U}, {0, 2, 30}, {2, 1, 20}};
	Graph narrow(3, narrow_edges);
	assert((dijkstra(narrow, 0, 100) == vector<int>{0, 50, 30}));
	assert((dijkstra_path(narrow, 0, 1, 100) == pair<int, vector<int>>{50, {0, 2, 1}}));
	using WideGraph = DirectedCSR<uint64_t>;
	const uint64_t maximum = numeric_limits<uint64_t>::max(), high = uint64_t(1) << 63;
	vector<WideGraph::Edge> huge_edges = {{0, 1, high}, {1, 2, maximum - high - 1}, {2, 3, 1}};
	WideGraph huge(4, huge_edges);
	assert((dijkstra(huge, 0, maximum) == vector<uint64_t>{0, high, maximum - 1, maximum}));
	assert((dijkstra_path(huge, 0, 2, maximum) == pair<uint64_t, vector<int>>{maximum - 1, {0, 1, 2}}));
	mt19937_64 random(531768);
	check_csr<uint32_t>(random);
	check_csr<uint64_t>(random);
	check_csr<int64_t>(random);
	check_csr<double>(random);
}
