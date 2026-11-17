#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_frequency"
#include "../data_structure/point_set_wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <utility>
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<int> a(n);
	for (auto &x : a) in.read(x);
	struct Query { int type, l, r, x; };
	std::vector<Query> queries(q);
	std::vector<std::pair<int, int>> candidates;
	for (auto &z : queries) {
		in.read(z.type); in.read(z.l);
		if (z.type == 0) in.read(z.x), candidates.emplace_back(z.l, z.x);
		else in.read(z.r), in.read(z.x);
	}
	PointSetWaveletMatrix<int> wm(a, candidates);
	for (auto z : queries) {
		if (z.type == 0) wm.set(z.l, z.x);
		else out.println(z.l ? wm.count(z.l, z.r, z.x) : wm.rank(z.x, z.r));
	}
}
