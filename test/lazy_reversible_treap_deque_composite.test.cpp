#define PROBLEM "https://judge.yosupo.jp/problem/deque_operate_all_composite"
#include "data_structure/lazy_reversible_treap.hpp"
#include "fixtures/deque_composite_verify.hpp"

int main() {
	using Sequence = LazyReversibleTreap<DequeAffine, deque_affine_op, deque_affine_e,
		int, deque_affine_mapping, deque_affine_composition, deque_affine_id>;
	return verify_deque_composite<Sequence>();
}
