#define PROBLEM "https://judge.yosupo.jp/problem/range_add_range_min"
#include "../data_structure/advanced/kinetic_tournament.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<std::int64_t> values(n), slopes(n);
	for (auto &value : values) in.read(value);
	KineticTournament<std::int64_t, false> tree(slopes, values);
	for (int i = 0; i < q; ++i) {
		int type = 0, l = 0, r = 0;
		in.read(type); in.read(l); in.read(r);
		if (!type) { std::int64_t value = 0; in.read(value); tree.add(l, r, value); }
		else out.println(tree.prod(l, r)->value);
	}
}
