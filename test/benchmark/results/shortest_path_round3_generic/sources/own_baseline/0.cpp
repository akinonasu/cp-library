#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#include "1.hpp"
#include "6.hpp"
#include <cstdint>
#include <utility>
#include <vector>

#ifndef DIJKSTRA_USE_TOPOLOGICAL
#define DIJKSTRA_USE_TOPOLOGICAL false
#endif

int main() {
	FastInput in;
	FastOutput out;
	int n, m, s, t;
	in.read(n), in.read(m), in.read(s), in.read(t);
	using Graph = DirectedCSR<std::uint32_t>;
	Graph g(n, m, [&](auto emit) {
		read_unsigned_records<3>(in, m, [&](std::uint32_t from, std::uint32_t to, std::uint32_t weight) {
			emit(Graph::Edge{int(from), int(to), weight});
		});
	});
	auto [distance, path] = dijkstra_path<DIJKSTRA_USE_TOPOLOGICAL>(g, s, t);
	if (distance == -1) out.println(-1);
	else {
		out.write(distance), out.write(' '), out.println(path.size() - 1);
		for (std::size_t i = 1; i < path.size(); ++i)
			out.write(std::uint32_t(path[i - 1])), out.write(' '), out.println(std::uint32_t(path[i]));
	}
}
