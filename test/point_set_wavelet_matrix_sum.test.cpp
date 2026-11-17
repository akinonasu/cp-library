#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"
#include "../data_structure/point_set_wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<std::int64_t> a(n);
	for (auto &x : a) in.read(x);
	auto current = a;
	struct Query { int type, p; std::int64_t x; };
	std::vector<Query> queries(q);
	std::vector<std::pair<int, std::int64_t>> candidates;
	for (auto &z : queries) {
		in.read(z.type); in.read(z.p); in.read(z.x);
		if (z.type == 0) candidates.emplace_back(z.p, current[z.p] += z.x);
	}
	PointSetWaveletMatrix<std::int64_t, std::int64_t> wm(a, candidates);
	for (auto z : queries) {
		if (z.type == 0) wm.set(z.p, wm[z.p] + z.x);
		else out.println(wm.range_sum(z.p, int(z.x)));
	}
}
