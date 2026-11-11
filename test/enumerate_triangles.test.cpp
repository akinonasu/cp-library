#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_triangles"
#include "graph/enumerate_triangles.hpp"
#include "components/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	std::vector<std::uint32_t> weight(n);
	for (auto &value : weight) input.read(value);
	std::vector<std::pair<int, int>> edges(m);
	for (auto &[u, v] : edges) input.read(u), input.read(v);
	constexpr std::uint32_t mod = 998244353;
	std::uint32_t answer = 0;
	enumerate_triangles(n, edges, [&](int u, int v, int w) {
		std::uint32_t value = std::uint64_t(weight[u]) * weight[v] % mod * weight[w] % mod;
		answer += value;
		if (answer >= mod) answer -= mod;
	});
	output.write(answer), output.write('\n');
}
