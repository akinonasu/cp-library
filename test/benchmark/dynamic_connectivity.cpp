#include "variants.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using I = std::int64_t;
I add(I a, I b) { return a + b; }
I zero() { return 0; }
struct Query { int type, u, v; };
struct Input { std::vector<std::pair<int, int>> edges; std::vector<Query> queries; };

Input make_input(const std::string &kind, int n, int q) {
	std::mt19937_64 random(713);
	Input input;
	for (int i = 1; i < n; ++i) input.edges.push_back({kind == "star" ? 0 : kind == "query" ? int(random() % i) : i - 1, i});
	if (kind == "cycle") input.edges.push_back({n - 1, 0});
	if (kind == "dense") {
		for (int i = 0; i < n; ++i) {
			int u = random() % n, v = random() % n;
			if (u != v) input.edges.push_back({u, v});
		}
	}
	if (kind == "star") for (int i = 1; i + 1 < n; ++i) input.edges.push_back({i, i + 1});
	for (int i = 0; i < q;) {
		int u = random() % n, v = random() % n;
		if (kind == "query") {
			int type = random() % 10;
			input.queries.push_back({type < 6 ? 3 : type < 9 ? 2 : 4, u, type < 9 ? int(random() % 100) : v});
			++i;
		} else {
			auto [a, b] = kind == "star" ? input.edges[(i / 4) % (n - 1)] : input.edges[random() % input.edges.size()];
			for (auto query : {Query{1, a, b}, Query{3, u, 0}, Query{2, v, int(random() % 100)}, Query{0, a, b}}) {
				if (i++ < q) input.queries.push_back(query);
			}
		}
	}
	return input;
}

template <template <class, auto, auto> class Graph, bool Aggregate>
void run(const Input &input, int n, const std::string &name, const std::string &kind) {
	using G = std::conditional_t<Aggregate, Graph<I, add, zero>, Graph<void, nullptr, nullptr>>;
	std::vector<I> values(n, 1);
	std::uint64_t digest = 0;
	auto begin = std::chrono::steady_clock::now();
	G graph = [&] { if constexpr (Aggregate) return G(values); else return G(n); }();
	graph.reserve(static_cast<int>(input.edges.size()));
	for (auto [u, v] : input.edges) digest += graph.link(u, v);
	for (auto [type, u, v] : input.queries) {
		std::uint64_t answer = 0;
		if (type == 0) answer = graph.link(u, v);
		else if (type == 1) answer = graph.cut(u, v);
		else if (type == 2) {
			if constexpr (Aggregate) graph.set(u, graph.get(u) + v);
			else answer = graph.size(u);
		} else if (type == 3) {
			if constexpr (Aggregate) answer = graph.prod(u);
			else answer = graph.size(u);
		} else answer = graph.same(u, v);
		digest = digest * 1000000007 + answer;
	}
	digest += graph.component_count();
	double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
	std::cout << "{\"variant\":\"" << name << "\",\"kind\":\"" << kind << "\",\"aggregate\":" << Aggregate
		<< ",\"seconds\":" << seconds << ",\"digest\":\"" << digest << "\"}\n";
}

int main(int argc, char **argv) {
	if (argc != 7) return 1;
	std::string name = argv[1], kind = argv[2];
	int n = std::stoi(argv[3]), q = std::stoi(argv[4]);
	bool aggregate = std::stoi(argv[5]);
	dynamic_connectivity_benchmark_seed = std::stoull(argv[6]);
	auto input = make_input(kind, n, q);
	return dispatch(input, n, name, kind, aggregate);
}
