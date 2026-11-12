#define PROBLEM "https://judge.yosupo.jp/problem/scc"
#include "graph/scc.hpp"
#include "components/fast_io_bulk.hpp"
#include "components/fast_io_output_values.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n), in.read(m);
	SCCGraph graph(n);
	graph.reserve(m);
	read_unsigned_records<2>(in, m, [&](unsigned from, unsigned to) { graph.add_edge(from, to); });
	auto groups = graph.decomposition();
	out.println(groups.size());
	if (groups.size() == 1) {
		auto group = groups[0];
		out.write(group.size());
		out.write(' ');
		write_unsigned_values<unsigned>(out, group.size(), [&](std::size_t i) { return unsigned(group[i]); });
	} else for (int component = 0; component < groups.size();) {
		auto group = groups[component];
		if (group.size() == 1) {
			int end = component + 1;
			while (end < groups.size() && groups[end].size() == 1) ++end;
			write_unsigned_records<2>(out, end - component, [&](std::size_t i) {
				return std::array{1U, unsigned(groups[component + int(i)][0])};
			});
			component = end;
		} else {
			out.write(group.size());
			for (int v : group) { out.write(' '); out.write(v); }
			out.write('\n');
			++component;
		}
	}
}
