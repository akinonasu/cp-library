#define PROBLEM "https://judge.yosupo.jp/problem/set_xor_min"
#include "data_structure/xor_set.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	XorSet<> set;
	read_unsigned_records<2>(input, q, [&](std::uint32_t type, std::uint32_t x) {
		if (type == 0) set.insert(x);
		else if (type == 1) set.erase(x);
		else output.println(set.min_xor(x));
	});
}
