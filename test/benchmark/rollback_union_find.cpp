#ifndef ROLLBACK_HEADER
#define ROLLBACK_HEADER "data_structure/rollback_union_find.hpp"
#endif
#include ROLLBACK_HEADER
#include "components/fast_io.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
	FastInput input;
	int n, q;
	input.read(n), input.read(q);
	struct Operation { int type, x, y; };
	std::vector<Operation> operations(q);
	for (auto &op : operations) input.read(op.type), input.read(op.x), input.read(op.y);
	auto start = std::chrono::steady_clock::now();
	RollbackUnionFind uf(n);
	uf.reserve(q);
	std::uint64_t checksum = 0;
	for (auto [type, x, y] : operations) {
		if (type == 0) checksum = checksum * 101 + uf.merge(x, y);
		else if (type == 1) checksum = checksum * 101 + uf.same(x, y);
		else if (type == 2) uf.rollback(x);
		else uf.undo();
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration<double, std::milli>(end - start).count() << ' '
		<< checksum << ' ' << uf.snapshot() << ' ' << uf.component_count() << '\n';
}
