#include "tree/rerooting.hpp"
#include <cstdlib>
#include <memory>
#include <numeric>
#include <random>
#include <string>

void require(bool ok) {
	if (!ok) std::abort();
}

using Pair = std::pair<long long, int>;

struct Vertex {
	long long sum = 0;
	int count = 0;
};

struct NoDefault {
	int value;
	explicit NoDefault(int value) : value(value) {}
};

struct Poison {
	int value = -19937;
};

namespace article {
std::vector<long long> weights;
Pair merge(Pair x, Pair y) { return {x.first + y.first, x.second + y.second}; }
Pair e() { return {0, 0}; }
Vertex put_vertex(Pair x, int v) { return {x.first + weights[v], x.second + 1}; }
Pair put_edge(Vertex x, int idx) { return {x.sum + idx * x.count, x.count}; }
using Tree = RerootingDP<Pair, Vertex, merge, e, put_edge, put_vertex>;

void check() {
	Tree empty;
	require(empty.empty() && empty.size() == 0 && empty.edge_count() == 0);
	require(empty.build().empty() && empty.reroot().empty());
	{
		constexpr auto merge = [](NoDefault a, NoDefault b) { return NoDefault(a.value + b.value); };
		constexpr auto e = [] { return NoDefault(0); };
		constexpr auto put_edge = [](NoDefault x, int) { return x; };
		constexpr auto put_vertex = [](NoDefault x, int) { return NoDefault(x.value + 1); };
		RerootingDP<NoDefault, NoDefault, merge, e, put_edge, put_vertex> tree(3);
		tree.add_edge(0, 1, 7, 11);
		for (int root : {0, 2, 1}) {
			auto sub = tree.build(root);
			require(sub[2].value == 1 && sub[root].value == (root == 2 ? 1 : 2));
			auto answer = tree.reroot();
			require(answer[0].value == 2 && answer[1].value == 2 && answer[2].value == 1);
			auto copy = tree;
			require(copy.reroot()[0].value == 2 && copy.subtree(1, 0).value == 1);
		}
	}
	std::mt19937 rng(1702);
	for (int n = 1; n <= 25; ++n) for (int trial = 0; trial < 8; ++trial) {
		weights.resize(n);
		for (auto& x : weights) x = int(rng() % 31) - 15;
		Tree tree(n);
		std::vector<std::vector<std::pair<int, int>>> graph(n);
		for (int v = 1; v < n; ++v) if (trial % 2 || rng() % 3) {
			int u = rng() % v;
			int to = v;
			int idx = int(rng() % 41) - 20, xdi = int(rng() % 41) - 20;
			if (rng() & 1) std::swap(u, to);
			require(tree.add_edge(u, to, idx, xdi) == tree.edge_count() - 1);
			graph[u].emplace_back(to, idx), graph[to].emplace_back(u, xdi);
		}
		auto naive = [&](auto&& self, int v, int parent) -> Vertex {
			Pair x = e();
			for (auto [to, idx] : graph[v]) if (to != parent)
				x = merge(x, put_edge(self(self, to, v), idx));
			return put_vertex(x, v);
		};
		for (int root : {0, n - 1}) {
			auto sub = tree.build(root);
			std::vector<int> parent(n, -2), order;
			auto visit = [&](int s) {
				if (parent[s] != -2) return;
				parent[s] = -1;
				order.push_back(s);
				for (std::size_t i = order.size() - 1; i < order.size(); ++i)
					for (auto [to, idx] : graph[order[i]]) if (to != parent[order[i]])
						parent[to] = order[i], order.push_back(to);
			};
			visit(root);
			for (int v = 0; v < n; ++v) visit(v);
			for (int v = 0; v < n; ++v) {
				auto expected = naive(naive, v, parent[v]);
				require(sub[v].sum == expected.sum && sub[v].count == expected.count);
			}
			for (int repeat = 0; repeat < 2; ++repeat) {
				auto answer = tree.reroot();
				for (int v = 0; v < n; ++v) {
					auto expected = naive(naive, v, -1);
					require(answer[v].sum == expected.sum && answer[v].count == expected.count);
				}
				for (int u = 0; u < n; ++u) {
					std::vector<int> parents(n, -2), vertices{u};
					parents[u] = -1;
					for (std::size_t i = 0; i < vertices.size(); ++i)
						for (auto [to, idx] : graph[vertices[i]]) if (to != parents[vertices[i]])
							parents[to] = vertices[i], vertices.push_back(to);
					for (int v : vertices) {
						auto expected = naive(naive, v, parents[v]);
						const auto& actual = tree.subtree(u, v);
						require(actual.sum == expected.sum && actual.count == expected.count);
					}
				}
			}
			require(sub[root].sum == naive(naive, root, -1).sum);
		}
	}
	{
		constexpr auto merge = [](std::string a, std::string b) { return a + b; };
		constexpr auto e = [] { return std::string{}; };
		constexpr auto put_edge = [](std::string x, int idx) { return '[' + std::to_string(idx) + x + ']'; };
		constexpr auto put_vertex = [](std::string x, int v) { return '(' + std::to_string(v) + x + ')'; };
		RerootingDP<std::string, std::string, merge, e, put_edge, put_vertex> tree(5);
		std::vector<std::vector<std::pair<int, int>>> graph(5);
		for (auto [u, v] : {std::pair{2, 1}, {0, 2}, {2, 3}}) {
			tree.add_edge(u, v, 10 + v, 20 + u);
			graph[u].emplace_back(v, 10 + v), graph[v].emplace_back(u, 20 + u);
		}
		auto naive = [&](auto&& self, int v, int parent) -> std::string {
			std::string x;
			for (auto [to, idx] : graph[v]) if (to != parent)
				x += put_edge(self(self, to, v), idx);
			return put_vertex(x, v);
		};
		for (int root = 0; root < 5; ++root) {
			auto sub = tree.build(root);
			require(sub[root] == naive(naive, root, -1));
			auto answer = tree.reroot();
			for (int v = 0; v < 5; ++v) require(answer[v] == naive(naive, v, -1));
			for (int u = 0; u < 5; ++u) {
				std::vector<int> parents(5, -2), vertices{u};
				parents[u] = -1;
				for (std::size_t i = 0; i < vertices.size(); ++i)
					for (auto [to, idx] : graph[vertices[i]]) if (to != parents[vertices[i]])
						parents[to] = vertices[i], vertices.push_back(to);
				for (int v : vertices) require(tree.subtree(u, v) == naive(naive, v, parents[v]));
			}
		}
	}
	weights.assign(3, 0);
	Tree growing(3);
	growing.add_edge(0, 1, 2, 7);
	growing.build();
	require(growing.reroot()[0].sum == 2 && growing.reroot()[1].sum == 7);
	growing.add_edge(1, 2, 5, 11);
	growing.build(2);
	auto copy = growing;
	require(copy.reroot()[2].sum == 29);
	require(copy.subtree(2, 1).sum == 7 && copy.subtree(2, 1).count == 2);
	for (bool star : {false, true}) {
		const int n = 300000;
		weights.assign(n, 0);
		Tree tree(n);
		for (int v = 1; v < n; ++v) tree.add_edge(star ? 0 : v - 1, v, 1, 1);
		tree.build(n / 2);
		auto answer = tree.reroot();
		for (int v = 0; v < n; ++v) {
			long long expected = star ? (v ? 2LL * n - 3 : n - 1) :
				1LL * v * (v + 1) / 2 + 1LL * (n - 1 - v) * (n - v) / 2;
			require(answer[v].sum == expected && answer[v].count == n);
			for (int u : {0, n - 1}) {
				const auto& sub = tree.subtree(u, v);
				long long sum;
				int count;
				if (u == v) sum = expected, count = n;
				else if (star) sum = v ? 0 : n - 2, count = v ? 1 : n - 1;
				else {
					count = u == 0 ? n - v : v + 1;
					sum = 1LL * count * (count - 1) / 2;
				}
				require(sub.sum == sum && sub.count == count);
			}
		}
	}
}
}

int main() {
	article::check();
	std::mt19937 rng(973401);
	Rerooting<int> empty;
	require(empty.empty() && empty.size() == 0 && empty.edge_count() == 0);
	empty.build(0, std::plus<int>{}, [](int x, int) { return x + 1; }, [](int x, int, int, int) { return x; });
	require(empty.values().empty());
	{
		Rerooting<Poison> tree(17);
		for (int v = 1; v < 12; ++v) tree.add_edge(v - 1, v);
		for (int root : {0, 8, 16, 0}) {
			tree.build(Poison{0}, [](Poison a, Poison b) { return Poison{a.value + b.value}; },
				[](Poison a, int) { return Poison{a.value + 1}; }, [](Poison a, int, int, int) { return a; }, root);
			for (int v = 0; v < 17; ++v) require(tree[v].value == (v < 12 ? 12 : 1));
			for (int id = 0; id < tree.edge_count(); ++id)
				require(tree.edge_dp(id).value == id + 1 && tree.edge_dp(id, true).value == 11 - id);
		}
		Rerooting<int, NoDefault> different(3);
		different.add_edge(0, 1);
		for (int repeat = 0; repeat < 2; ++repeat) {
			different.build(NoDefault(0), [](NoDefault a, NoDefault b) { return NoDefault(a.value + b.value); },
				[](NoDefault a, int) { return a.value + 1; }, [](int a, int, int, int) { return NoDefault(a); });
			require(different[0] == 2 && different[1] == 2 && different[2] == 1);
		}
	}
	for (int n = 1; n <= 35; ++n) for (int trial = 0; trial < 12; ++trial) {
		Rerooting<Vertex, Pair> tree(n);
		std::vector<std::pair<int, int>> edges;
		for (int v = 1; v < n; ++v) if (trial % 3 || rng() % 3) {
			int p = rng() % v;
			if (rng() & 1) edges.emplace_back(p, v);
			else edges.emplace_back(v, p);
		}
		std::shuffle(edges.begin(), edges.end(), rng);
		std::vector<std::vector<std::pair<int, int>>> graph(n);
		std::vector<Pair> weights(edges.size());
		std::vector<long long> a(n);
		for (auto &x : a) x = int(rng() % 101) - 50;
		for (int id = 0; id < static_cast<int>(edges.size()); ++id) {
			auto [u, v] = edges[id];
			require(tree.add_edge(u, v) == id && tree.edge(id) == edges[id]);
			graph[u].emplace_back(v, id), graph[v].emplace_back(u, id);
			weights[id] = {int(rng() % 21) - 10, int(rng() % 21) - 10};
		}
		auto merge = [](Pair x, Pair y) { return Pair{x.first + y.first, x.second + y.second}; };
		auto vertex = [&](Pair x, int v) { return Vertex{x.first + a[v], x.second + 1}; };
		auto lift = [&](Vertex x, int from, int, int id) {
			long long w = from == edges[id].first ? weights[id].first : weights[id].second;
			return Pair{x.sum + w * x.count, x.count};
		};
		auto naive = [&](auto&& self, int v, int banned) -> Vertex {
			Pair x{0, 0};
			for (auto [to, id] : graph[v]) if (id != banned) x = merge(x, lift(self(self, to, id), to, v, id));
			return vertex(x, v);
		};
		for (int root : {0, n - 1, int(rng() % n)}) {
			const auto &result = tree.build(Pair{0, 0}, merge, vertex, lift, root);
			require(&result == &tree.values() && static_cast<int>(result.size()) == n);
			for (int v = 0; v < n; ++v) {
				auto expected = naive(naive, v, -1);
				require(tree[v].sum == expected.sum && tree[v].count == expected.count);
			}
			for (int id = 0; id < tree.edge_count(); ++id) {
				auto [u, v] = tree.edge(id);
				require(tree.edge_dp(id) == lift(naive(naive, u, id), u, v, id));
				require(tree.edge_dp(id, true) == lift(naive(naive, v, id), v, u, id));
			}
		}
		for (auto &x : a) x += 31;
		tree.build(Pair{0, 0}, merge, vertex, lift);
		for (int v = 0; v < n; ++v) require(tree[v].sum == naive(naive, v, -1).sum);
		if (n <= 9) {
			Rerooting<std::string> strings(n);
			for (auto [u, v] : edges) strings.add_edge(u, v);
			auto combine = [](const auto &x, const auto &y) { return x + y; };
			auto add = [](const auto &x, int v) { return '(' + std::to_string(v) + ':' + x + ')'; };
			auto pass = [](const auto &x, int from, int to, int id) {
				return '[' + std::to_string(from) + '>' + std::to_string(to) + ':' + std::to_string(id) + x + ']';
			};
			auto expected = [&](auto&& self, int v, int banned) -> std::string {
				std::string x;
				for (auto [to, id] : graph[v]) if (id != banned) x += pass(self(self, to, id), to, v, id);
				return add(x, v);
			};
			for (int root = 0; root < n; ++root) {
				strings.build(std::string{}, combine, add, pass, root);
				for (int v = 0; v < n; ++v) require(strings[v] == expected(expected, v, -1));
				for (int id = 0; id < strings.edge_count(); ++id) {
					auto [u, v] = strings.edge(id);
					require(strings.edge_dp(id) == pass(expected(expected, u, id), u, v, id));
					require(strings.edge_dp(id, true) == pass(expected(expected, v, id), v, u, id));
				}
			}
		}
	}
	for (int n = 1; n <= 9; ++n) for (int trial = 0; trial < 8; ++trial) {
		Rerooting<unsigned long long> tree(n);
		std::vector<unsigned> adjacent(n);
		for (int v = 1; v < n; ++v) {
			int p = rng() % v;
			tree.add_edge(p, v);
			adjacent[p] |= 1U << v, adjacent[v] |= 1U << p;
		}
		std::vector<int> counts(n);
		for (unsigned mask = 1; mask < (1U << n); ++mask) {
			unsigned reached = mask & -mask, last = 0;
			while (last != reached) {
				last = reached;
				for (int v = 0; v < n; ++v) if ((reached >> v) & 1) reached |= adjacent[v] & mask;
			}
			if (reached == mask) for (int v = 0; v < n; ++v) counts[v] += (mask >> v) & 1;
		}
		for (unsigned long long mod : {1, 2, 6, 12, 1000000007}) {
			tree.build(1 % mod, [=](auto x, auto y) { return x * y % mod; },
				[](auto x, int) { return x; }, [=](auto x, int, int, int) { return (x + 1) % mod; });
			for (int v = 0; v < n; ++v) require(tree[v] == counts[v] % mod);
		}
	}
	{
		Rerooting<NoDefault> tree(5);
		tree.add_edge(2, 0), tree.add_edge(3, 1);
		int vertex_calls = 0, edge_calls = 0;
		auto vertex = [counter = std::make_unique<int>(0), &vertex_calls](NoDefault x, int) {
			++*counter, ++vertex_calls;
			return NoDefault(x.value + 1);
		};
		auto lift = [&](NoDefault x, int, int, int) { ++edge_calls; return x; };
		auto merge = [](NoDefault x, NoDefault y) { return NoDefault(x.value + y.value); };
		tree.build(NoDefault(0), merge, std::move(vertex), lift, 4);
		require(vertex_calls == 9 && edge_calls == 4 && tree[4].value == 1 && tree[0].value == 2);
		tree.add_edge(0, 4);
		tree.build(NoDefault(0), merge, vertex, lift, 3);
		require(tree[0].value == 3 && tree[2].value == 3 && tree[1].value == 2);
		auto copy = tree;
		require(copy[4].value == 3 && copy.edge_dp(2).value == 2);
	}
	for (bool star : {false, true}) {
		const int n = 300000;
		Rerooting<Pair> tree(n);
		for (int v = 1; v < n; ++v) tree.add_edge(star ? 0 : v - 1, v);
		tree.build(Pair{0, 0}, [](Pair x, Pair y) { return Pair{x.first + y.first, x.second + y.second}; },
			[](Pair x, int) { return Pair{x.first, x.second + 1}; },
			[](Pair x, int, int, int) { return Pair{x.first + x.second, x.second}; }, n / 2);
		for (int v = 0; v < n; ++v) {
			long long expected = star ? (v ? 2LL * n - 3 : n - 1) :
				1LL * v * (v + 1) / 2 + 1LL * (n - 1 - v) * (n - v) / 2;
			require(tree[v].first == expected && tree[v].second == n);
		}
	}
}
