#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"
#include "data_structure/fenwick_tree.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<long long> a(n);
	std::size_t position = 0;
	read_unsigned_blocks(in, n, [&](const std::uint32_t *block, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) a[position++] = block[i];
	});
	FenwickTree<long long> tree(a);
	read_unsigned_records<3>(in, q, [&](std::uint32_t type, std::uint32_t l, std::uint32_t r) {
		if (type == 0) tree.add(l, r);
		else out.println(tree.sum(l, int(r)));
	});
}
