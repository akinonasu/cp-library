#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_frequency"
#include "../data_structure/dynamic_wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<std::uint32_t> a(n);
	for (auto &x : a) in.read(x);
	DynamicWaveletMatrix<std::uint32_t, 30> wm(a);
	while (q--) {
		int type = 0, l = 0, r = 0;
		std::uint32_t x = 0;
		in.read(type); in.read(l);
		if (type == 0) in.read(x), wm.set(l, x);
		else {
			in.read(r); in.read(x);
			out.println(l ? wm.count(l, r, x) : wm.rank(x, r));
		}
	}
}
