#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#include "data_structure/segtree.hpp"
#include "components/fast_io_bulk.hpp"
#include "math/mint.hpp"
#include <utility>
#include <vector>

using S = std::pair<mint, mint>;
S op(S f, S g) { return {g.first * f.first, g.first * f.second + g.second}; }
S e() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<S> values(n);
	std::size_t position = 0;
	read_unsigned_records<2>(input, n, [&](std::uint32_t a, std::uint32_t b) { values[position++] = {mint::raw(a), mint::raw(b)}; });
	Segtree<S, op, e> tree(values);
	read_unsigned_records<4>(input, q, [&](std::uint32_t type, std::uint32_t l, std::uint32_t a, std::uint32_t b) {
		if (type == 0) tree.set(l, {mint::raw(a), mint::raw(b)});
		else {
			auto [scale, shift] = tree.prod(l, a);
			output.println((scale * mint::raw(b) + shift).val());
		}
	});
}
