#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include "../data_structure/wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<int> a(n);
	for (auto &x : a) in.read(x);
	const WaveletMatrix<int> wm(a);
	while (q--) {
		int l = 0, r = 0, k = 0;
		in.read(l); in.read(r); in.read(k);
		out.println(k < (r - l) / 2 ? wm.kth_smallest(l, r, k) : wm.kth_largest(l, r, r - l - 1 - k));
	}
}
