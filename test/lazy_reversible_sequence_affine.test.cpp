#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum"
#include "data_structure/lazy_reversible_sequence.hpp"
#include "components/fast_io.hpp"
#include "math/mint.hpp"
#include <vector>

struct Affine { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(Affine f, mint value, int n) { return f.a * value + f.b * n; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<mint> values(n);
	for (auto &value : values) { unsigned x = 0; input.read(x); value = mint::raw(x); }
	LazyReversibleSequence<mint, op, e, Affine, mapping, composition, id, true> sequence(values);
	sequence.reserve(n + q);
	while (q--) {
		int type = 0, l = 0;
		input.read(type), input.read(l);
		if (type == 0) { unsigned x = 0; input.read(x); sequence.insert(l, mint::raw(x)); }
		else if (type == 1) sequence.erase(l);
		else {
			int r = 0;
			input.read(r);
			if (type == 2) sequence.reverse(l, r);
			else if (type == 3) {
				unsigned a = 0, b = 0;
				input.read(a), input.read(b);
				sequence.apply(l, r, {mint::raw(a), mint::raw(b)});
			} else output.println(sequence.prod(l, r).val());
		}
	}
}
