#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ITP2_4_B"
#include "data_structure/persistent_deque.hpp"
#include "components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n);
	std::vector<int> values(n);
	for (int &value : values) input.read(value);
	PersistentDeque<int> sequence;
	auto root = sequence.build(values);
	input.read(q);
	while (q--) {
		int l = 0, m = 0, r = 0;
		input.read(l), input.read(m), input.read(r);
		auto [prefix, rest] = sequence.split(root, l);
		auto [left, after] = sequence.split(rest, m - l);
		auto [right, suffix] = sequence.split(after, r - m);
		root = sequence.concat(prefix, sequence.concat(right, sequence.concat(left, suffix)));
	}
	bool first = true;
	for (int value : sequence.to_vector(root)) {
		if (!first) output.write(' ');
		first = false;
		output.write(value);
	}
	output.write('\n');
}
