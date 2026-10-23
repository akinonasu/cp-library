#define PROBLEM "https://judge.yosupo.jp/problem/unionfind_with_potential"
#include "../data_structure/union_find/weighted_union_find.hpp"
#include "../math/mint.hpp"
#include "../components/fast_io.hpp"
#include <tuple>

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	WeightedUnionFind<mint> uf(n);
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int t = 0, u = 0, v = 0;
		input.read(t), input.read(u), input.read(v);
		if (t == 0) {
			unsigned x = 0;
			input.read(x);
			output.println(uf.merge(v, u, mint::raw(x)));
		} else if (uf.same(u, v)) output.println(uf.diff(v, u).val());
		else output.println(-1);
	}
}
