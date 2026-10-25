#define PROBLEM "https://judge.yosupo.jp/problem/static_range_lis_query"
#include "../data_structure/range_lis_query.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<int> values(n);
	for (auto &value : values) in.read(value);
	RangeLISQuery<> tree(values);
	while (q--) {
		int l = 0, r = 0;
		in.read(l), in.read(r);
		out.println(tree.query(l, r));
	}
}
