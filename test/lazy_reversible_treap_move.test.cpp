#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ITP2_4_B"
#include "data_structure/lazy_reversible_treap.hpp"
#include "fixtures/sequence_rotate_verify.hpp"

int main() {
	using Sequence = LazyReversibleTreap<RotationValue, rotation_op, rotation_e,
		RotationValue, rotation_mapping, rotation_op, rotation_e, true>;
	return verify_rotation<Sequence, true>();
}
