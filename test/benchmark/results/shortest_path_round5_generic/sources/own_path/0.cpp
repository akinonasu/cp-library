#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#include "1.hpp"
#include "8.hpp"
#include <cstdint>
#include <array>
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
		write_unsigned_records<2>(out, path.size() - 1, [&](std::size_t i) {
			return std::array{std::uint32_t(path[i]), std::uint32_t(path[i + 1])};
		});
	}
}
