#define PROBLEM "https://judge.yosupo.jp/problem/line_add_get_min"
#include "data_structure/li_chao_tree/dynamic_li_chao_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	DynamicLiChaoTree<> tree(-1000000000LL, 1000000001LL);
	tree.reserve(n + q);
	for (int i = 0; i < n; ++i) {
		std::int64_t a = 0, b = 0;
		input.read(a), input.read(b);
		tree.add_line(a, b);
	}
	while (q--) {
		int type = 0;
		std::int64_t a = 0, b = 0;
		input.read(type), input.read(a);
		if (type == 0) input.read(b), tree.add_line(a, b);
		else output.println(std::int64_t(*tree.query(a)));
	}
}
