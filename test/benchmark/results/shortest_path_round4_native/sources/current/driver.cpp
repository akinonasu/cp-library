#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "components/fast_io_bulk.hpp"
#include "graph/dijkstra.hpp"

using Graph = DirectedCSR<std::uint32_t>;
using Clock = std::chrono::steady_clock;

double elapsed(Clock::time_point first, Clock::time_point last) {
	return std::chrono::duration<double, std::milli>(last - first).count();
}

int main() {
	auto start = Clock::now();
	FastInput input;
	FastOutput output;
	int n, m, source, target;
	input.read(n); input.read(m); input.read(source); input.read(target);
	Clock::time_point parsed;
	Graph graph(n, m, [&](auto emit) {
		read_unsigned_records<3>(input, m, [&](std::uint32_t from, std::uint32_t to, std::uint32_t weight) {
			emit(Graph::Edge{int(from), int(to), weight});
		});
		parsed = Clock::now();
	});
	auto built = Clock::now();
#ifdef SP_TOPOLOGICAL
	auto [distance, path] = dijkstra_path<true>(graph, source, target);
#else
	auto [distance, path] = dijkstra_path(graph, source, target);
#endif
	auto solved = Clock::now();
	if (distance == -1) output.println(-1);
	else {
		output.write(distance); output.write(' '); output.println(path.size() - 1);
		write_unsigned_records<2>(output, path.size() - 1, [&](std::size_t i) {
			return std::array{std::uint32_t(path[i]), std::uint32_t(path[i + 1])};
		});
	}
	output.flush();
	auto written = Clock::now();
	std::fprintf(stderr,
		"{\"parse_ms\":%.9f,\"build_ms\":%.9f,\"solve_ms\":%.9f,\"output_ms\":%.9f,\"distance\":%lld,\"path_edges\":%zu}\n",
		elapsed(start, parsed), elapsed(parsed, built), elapsed(built, solved), elapsed(solved, written),
		static_cast<long long>(distance), path.empty() ? 0 : path.size() - 1);
}
