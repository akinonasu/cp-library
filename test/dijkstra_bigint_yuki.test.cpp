#define PROBLEM "https://yukicoder.me/problems/no/3669"
#include "graph/dijkstra.hpp"
#include "math/bigint.hpp"
#include "components/fast_io.hpp"
#include <numeric>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	struct Edge { int u, v, a, b; };
	std::vector<Edge> edges(m);
	int max_b = 1;
	for (auto& e : edges) input.read(e.u), input.read(e.v), input.read(e.a), input.read(e.b), max_b = std::max(max_b, e.b);
	BigInt denominator = 1;
	for (int b = 2; b <= max_b; ++b) denominator *= b / std::gcd(b, int(denominator % BigInt(b)));
	std::vector<std::vector<std::pair<int, BigInt>>> graph(n);
	std::vector<BigInt> unit(max_b + 1);
	for (int b = 1; b <= max_b; ++b) unit[b] = denominator / BigInt(b);
	for (auto e : edges) {
		auto weight = unit[e.b] * BigInt(e.a);
		graph[e.u - 1].emplace_back(e.v - 1, weight);
		graph[e.v - 1].emplace_back(e.u - 1, std::move(weight));
	}
	auto distance = dijkstra(graph, 0, denominator * BigInt(300LL * n + 1));
	for (int v = 1; v < n; ++v) {
		BigInt a = distance[v], b = denominator;
		while (b) { auto r = a % b; a = std::move(b), b = std::move(r); }
		output.write(distance[v] / a), output.write(' '), output.println(denominator / a);
	}
}
