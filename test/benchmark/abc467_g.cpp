#ifndef POINT_WM_HEADER
#define POINT_WM_HEADER "data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp"
#endif
#include POINT_WM_HEADER
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	std::vector<int> a(n);
	for (auto &x : a) input.read(x);
	struct Query { int p, x, l, r; std::int64_t target; };
	std::vector<Query> queries(q);
	std::vector<std::pair<int, int>> candidates;
	candidates.reserve(q);
	for (auto &z : queries) {
		input.read(z.p); input.read(z.x); input.read(z.l); input.read(z.r); input.read(z.target);
		--z.p, --z.l;
		candidates.emplace_back(z.p, z.x);
	}
	PointSetWaveletMatrix<int> wm(a, candidates);
	for (auto z : queries) {
		[[maybe_unused]] bool ok = wm.set(z.p, z.x);
		assert(ok);
		output.println(wm.min_count_largest(z.l, z.r, z.target));
	}
}
