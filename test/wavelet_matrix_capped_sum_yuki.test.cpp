#define PROBLEM "https://yukicoder.me/problems/no/2065"
#include "../data_structure/wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<int> a(n);
	for (auto &x : a) in.read(x);
	const WaveletMatrix<int, std::int64_t> wm(a, true);
	while (q--) {
		int l = 0, r = 0, x = 0;
		in.read(l); in.read(r); in.read(x);
		--l;
		int count = wm.range_freq(l, r, x);
		out.println(wm.range_sum(l, r, x) + std::int64_t(x) * (r - l - count));
	}
}
