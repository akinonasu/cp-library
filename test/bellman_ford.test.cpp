#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_1_B"
#include "../graph/bellman_ford.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0, source = 0;
	in.read(n); in.read(m); in.read(source);
	DirectedCSR<int> g(n, m, [&](auto emit) {
		for (int i = 0; i < m; ++i) {
			int v = 0, to = 0, w = 0;
			in.read(v); in.read(to); in.read(w);
			emit(DirectedCSR<int>::Edge{v, to, w});
		}
	});
	auto result = bellman_ford(g, source);
	if (result.has_negative_cycle()) out.println("NEGATIVE CYCLE");
	else for (int v = 0; v < n; ++v) {
		if (result.reachable(v)) out.println(result.distance[v]);
		else out.println("INF");
	}
}
