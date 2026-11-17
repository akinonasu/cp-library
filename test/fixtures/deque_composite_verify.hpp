#ifndef DEQUE_COMPOSITE_VERIFY_HPP
#define DEQUE_COMPOSITE_VERIFY_HPP

#include "components/fast_io.hpp"
#include "math/mint.hpp"

struct DequeAffine { mint a, b; };
DequeAffine deque_affine_op(DequeAffine f, DequeAffine g) { return {g.a * f.a, g.a * f.b + g.b}; }
DequeAffine deque_affine_e() { return {1, 0}; }
DequeAffine deque_affine_mapping(int, DequeAffine value, int) { return value; }
int deque_affine_composition(int, int) { return 0; }
int deque_affine_id() { return 0; }

template<class Sequence>
int verify_deque_composite() {
	FastInput input;
	FastOutput output;
	Sequence sequence;
	int q = 0;
	input.read(q);
	sequence.reserve(q);
	while (q--) {
		int type = 0;
		input.read(type);
		if (type <= 1) {
			unsigned a = 0, b = 0;
			input.read(a), input.read(b);
			sequence.insert(type == 0 ? 0 : sequence.size(), {mint::raw(a), mint::raw(b)});
		} else if (type <= 3) sequence.erase(type == 2 ? 0 : sequence.size() - 1);
		else {
			unsigned x = 0;
			input.read(x);
			auto f = sequence.all_prod();
			output.println((f.a * mint::raw(x) + f.b).val());
		}
	}
	return 0;
}

#endif
