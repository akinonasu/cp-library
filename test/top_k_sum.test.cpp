#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"
#include "../data_structure/top_k_sum.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n);
	for (int &x : a) input.read(x);
	TopKSum<int> values(a);
	while (q--) {
		int type = 0;
		input.read(type);
		if (type == 0) { int x = 0; input.read(x); values.insert(x); }
		else {
			auto x = type == 1 ? values.kth_smallest(0) : values.kth_largest(0);
			output.println(x);
			values.erase(static_cast<int>(x));
		}
	}
}
