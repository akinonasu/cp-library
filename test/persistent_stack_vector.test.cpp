#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ITP2_1_A"
#include "data_structure/persistent_stack.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	PersistentStack<int> stack;
	stack.reserve(q);
	auto root = stack.initial_root();
	while (q--) {
		int type = 0, x = 0;
		input.read(type);
		if (type == 0) { input.read(x); root = stack.push(root, x); }
		else if (type == 1) { input.read(x); output.println(stack.get(root, stack.size(root) - 1 - x)); }
		else root = stack.pop(root);
	}
}
