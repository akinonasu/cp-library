#ifndef SEQUENCE_ROTATE_VERIFY_HPP
#define SEQUENCE_ROTATE_VERIFY_HPP

#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

using RotationValue = std::int64_t;
RotationValue rotation_op(RotationValue a, RotationValue b) { return a + b; }
RotationValue rotation_e() { return 0; }
RotationValue rotation_mapping(RotationValue f, RotationValue x, int n) { return x + f * n; }

template<class Sequence, bool Move>
int verify_rotation() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n);
	std::vector<RotationValue> values(n);
	for (auto &value : values) input.read(value);
	Sequence sequence(values);
	input.read(q);
	while (q--) {
		int l = 0, m = 0, r = 0;
		input.read(l), input.read(m), input.read(r);
		if constexpr (Move) sequence.move(l, m, r - (m - l));
		else sequence.rotate(l, m, r);
	}
	bool first = true;
	for (auto value : sequence.to_vector()) {
		if (!first) output.write(' ');
		first = false;
		output.write(value);
	}
	output.write('\n');
	return 0;
}

#endif
