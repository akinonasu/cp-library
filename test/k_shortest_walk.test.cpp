#define PROBLEM "https://judge.yosupo.jp/problem/k_shortest_walk"
#include "../graph/k_shortest_walk.hpp"
#include "../components/directed_csr.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0, s = 0, t = 0, k = 0;
	in.read(n), in.read(m), in.read(s), in.read(t), in.read(k);
	using Graph = DirectedCSR<std::uint32_t>;
	Graph graph(n, m, [&](auto emit) {
		for (int i = 0; i < m; ++i) {
			Graph::Edge edge{};
			in.read(edge.from), in.read(edge.to), in.read(edge.weight);
			emit(edge);
		}
	});
	const auto answer = k_shortest_walk(graph, s, t, k);
	for (int i = 0; i < k; ++i) out.println(i < int(answer.size()) ? answer[i] : -1);
}
