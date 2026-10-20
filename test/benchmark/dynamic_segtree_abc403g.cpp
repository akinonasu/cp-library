#include "data_structure/segment_tree/dynamic_segtree.hpp"
#include "components/fast_io.hpp"
#include <bit>
#include <cstdint>

#ifdef DYNAMIC_SEGTREE_PACKED
using Value = std::uint64_t;
Value join(Value a, Value b) {
	Value value = b & ~Value(1);
	if (a & 1) value = -value;
	return (a & ~Value(1)) + value + ((a ^ b) & 1);
}
Value unit() { return 0; }
Value element(std::int64_t x) { return (Value(x) << 1) | 1; }
std::int64_t alternating(Value x) { return std::bit_cast<std::int64_t>(x) >> 1; }
#else
struct Value { std::int64_t sum; unsigned parity; };
Value join(Value a, Value b) { return {a.sum + (a.parity ? -b.sum : b.sum), a.parity ^ b.parity}; }
Value unit() { return {0, 0}; }
Value element(std::int64_t x) { return {x, 1}; }
std::int64_t alternating(Value x) { return x.sum; }
#endif

int main() {
	FastInput input;
	FastOutput output;
	int q;
	input.read(q);
	DynamicSegtree<Value, join, unit> tree(1000000001);
	tree.reserve(q);
	std::int64_t total = 0, answer = 0;
	while (q--) {
		unsigned y;
		input.read(y);
		std::int64_t x = (y + answer) % 1000000000 + 1;
		total += x;
		tree.multiply(x, element(x));
		answer = (total + alternating(tree.all_prod())) / 2;
		output.write(answer);
		output.write('\n');
	}
}
