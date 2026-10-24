#define PROBLEM "https://judge.yosupo.jp/problem/cartesian_tree"
#include "tree/cartesian_tree.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<int> values(n);
	int position = 0;
	read_unsigned_records<1>(input, n, [&](unsigned value) { values[position++] = value; });
	auto tree = cartesian_tree(values);
	write_unsigned_records<1>(output, n, [&](std::size_t v) {
		return std::array{unsigned(tree.parent[v] == -1 ? int(v) : tree.parent[v])};
	});
}
