#define PROBLEM "https://judge.yosupo.jp/problem/tree_diameter"
#include "tree/tree_diameter.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	TreeDiameter<> tree(n);
	read_unsigned_records<3>(input, n - 1, [&](unsigned u, unsigned v, unsigned weight) { tree.add_edge(u, v, weight); });
	auto result = tree.solve();
	output.write(result.distance), output.write(' '), output.println(result.path.size());
	write_unsigned_records<1>(output, result.path.size(), [&](std::size_t i) { return std::array{unsigned(result.path[i])}; });
}
