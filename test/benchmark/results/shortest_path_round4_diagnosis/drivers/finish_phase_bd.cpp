#include <array>
#include <sys/resource.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "components/fast_io.hpp"
#include "graph/dijkstra.hpp"
#include "components/fast_io_bulk.hpp"

using Graph = DirectedCSR<std::uint32_t>;
using Clock = std::chrono::steady_clock;

double elapsed(Clock::time_point first, Clock::time_point last) {
	return std::chrono::duration<double, std::milli>(last - first).count();
}

int main() {
	rusage before{}, parsed_usage{}, built_usage{};
	getrusage(RUSAGE_SELF, &before);
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
		getrusage(RUSAGE_SELF, &parsed_usage);
		parsed = Clock::now();
	});
	auto built = Clock::now();
	getrusage(RUSAGE_SELF, &built_usage);
#ifdef SP_TOPOLOGICAL
	auto [distance, path] = dijkstra_path<true>(graph, source, target);
#else
	auto [distance, path] = dijkstra_path(graph, source, target);
#endif
	auto solved = Clock::now();
	if (distance == -1) output.println(-1);
	else {
		output.write(distance); output.write(' '); output.println(path.size() - 1);
#ifdef SP_BASELINE
		for (std::size_t i = 1; i < path.size(); ++i) {
			output.write(std::uint32_t(path[i - 1])); output.write(' '); output.println(std::uint32_t(path[i]));
		}
#else
		write_unsigned_records<2>(output, path.size() - 1, [&](std::size_t i) {
			return std::array{std::uint32_t(path[i]), std::uint32_t(path[i + 1])};
		});
#endif
	}
	output.flush();
	auto written = Clock::now();
	std::fprintf(stderr,
		"{\"parse_ms\":%.9f,\"build_ms\":%.9f,\"solve_ms\":%.9f,\"output_ms\":%.9f,\"distance\":%lld,\"path_edges\":%zu}\n",
		elapsed(start, parsed), elapsed(parsed, built), elapsed(built, solved), elapsed(solved, written),
		static_cast<long long>(distance), path.empty() ? 0 : path.size() - 1);

	std::fprintf(stderr, "{\"offset_addr\":%llu,\"arcs_addr\":%llu,\"edges_addr\":%llu,\"parse_minor\":%ld,\"build_minor\":%ld,\"build_major\":%ld}\n", static_cast<unsigned long long>(reproduce::offset), static_cast<unsigned long long>(reproduce::arcs), static_cast<unsigned long long>(reproduce::edges), parsed_usage.ru_minflt-before.ru_minflt, built_usage.ru_minflt-parsed_usage.ru_minflt, built_usage.ru_majflt-parsed_usage.ru_majflt);

	std::fprintf(stderr, "{\"prefix_ms\":%.9f,\"alloc_ms\":%.9f,\"scatter_ms\":%.9f,\"prefix_fault\":%ld,\"alloc_fault\":%ld,\"scatter_fault\":%ld}\n", elapsed(reproduce::t0,reproduce::t1),elapsed(reproduce::t1,reproduce::t2),elapsed(reproduce::t2,reproduce::t3), reproduce::r1.ru_minflt-reproduce::r0.ru_minflt,reproduce::r2.ru_minflt-reproduce::r1.ru_minflt,reproduce::r3.ru_minflt-reproduce::r2.ru_minflt);
}
