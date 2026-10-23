#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include "../components/fast_io.hpp"
#include "../data_structure/top_k_sum.hpp"
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<int> values(n);
	for (int &x : values) in.read(x);
	SmallestKSum<int> set(values);
	while (q--) {
		int type = 0, x = 0;
		in.read(type); in.read(x);
		if (type == 0) { if (!set.contains(x)) set.insert(x); }
		if (type == 1) set.erase_all(x);
		if (type == 2) out.println(set.size() < x ? -1 : set.kth(x - 1));
		if (type == 3) out.println(set.order_of_key(x + 1));
		if (type == 4) {
			auto k = set.order_of_key(x + 1);
			out.println(k ? set.kth(k - 1) : -1);
		}
		if (type == 5) out.println(set.lower_bound(x).value_or(-1));
	}
}
