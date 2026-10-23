#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#include "../graph/shortest_path/bellman_ford.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0, source = 0, target = 0;
	in.read(n); in.read(m); in.read(source); in.read(target);
	DirectedCSR<int> g(n, m, [&](auto emit) {
		for (int i = 0; i < m; ++i) {
			int v = 0, to = 0, w = 0;
			in.read(v); in.read(to); in.read(w);
			emit(DirectedCSR<int>::Edge{v, to, w});
		}
	});
	auto result = bellman_ford(g, source);
	if (!result.reachable(target)) { out.println(-1); return 0; }
	auto path = result.path(target);
	out.write(result.distance[target]); out.write(' '); out.println(path.size() - 1);
	for (std::size_t i = 1; i < path.size(); ++i) { out.write(path[i - 1]); out.write(' '); out.println(path[i]); }
}
