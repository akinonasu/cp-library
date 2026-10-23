#define PROBLEM "https://judge.yosupo.jp/problem/rectangle_sum"
#include "../data_structure/offline_rectangle_sum.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	OfflineRectangleSum<int, int, std::int64_t> solver(n, q);
	for (int i = 0; i < n; ++i) {
		int x = 0, y = 0, w = 0;
		input.read(x); input.read(y); input.read(w);
		solver.add_point(x, y, w);
	}
	for (int i = 0; i < q; ++i) {
		int l = 0, d = 0, r = 0, u = 0;
		input.read(l); input.read(d); input.read(r); input.read(u);
		solver.add_query(l, d, r, u);
	}
	for (auto answer : solver.solve()) output.println(answer);
}
