#define PROBLEM "https://judge.yosupo.jp/problem/segment_add_get_min"
#include "data_structure/li_chao_tree/dynamic_li_chao_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	DynamicLiChaoTree<> tree(-1000000000LL, 1000000001LL);
	tree.reserve(n + q);
	auto add = [&](std::int64_t l) {
		std::int64_t r = 0, a = 0, b = 0;
		input.read(r), input.read(a), input.read(b);
		tree.add_segment(l, r, a, b);
	};
	for (int i = 0; i < n; ++i) {
		std::int64_t l = 0;
		input.read(l), add(l);
	}
	while (q--) {
		int type = 0;
		std::int64_t x = 0;
		input.read(type), input.read(x);
		if (type == 0) add(x);
		else {
			auto value = tree.query(x);
			if (value) output.println(std::int64_t(*value));
			else output.println("INFINITY");
		}
	}
}
