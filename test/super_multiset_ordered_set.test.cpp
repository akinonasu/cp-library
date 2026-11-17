#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include "data_structure/super_multiset.hpp"
#include "components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> values(n);
	for (int &x : values) input.read(x);
	SuperMultiset<int> set(values);
	while (q--) {
		int type = 0, x = 0;
		input.read(type), input.read(x);
		if (type == 0) { if (!set.contains(x)) set.insert(x); }
		else if (type == 1) set.erase_all(x);
		else if (type == 2) output.println(set.size() < x ? -1 : set.kth_smallest(x - 1));
		else if (type == 3) output.println(set.size() - set.count_greater(x));
		else if (type == 4) output.println(set.prev(x).value_or(-1));
		else output.println(set.next(x).value_or(-1));
	}
}
