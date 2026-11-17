#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include "../data_structure/dynamic_wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<std::uint32_t> initial(n);
	for (auto &x : initial) in.read(x);
	DynamicWaveletMatrix<std::uint32_t, 30> wm(initial);
	while (q--) {
		int type = 0;
		std::uint32_t x = 0;
		in.read(type); in.read(x);
		if (type == 0 && wm.select(x, 0) < 0) wm.push_back(x);
		if (type == 1) {
			int p = wm.select(x, 0);
			if (p >= 0) wm.erase(p);
		}
		if (type == 2) out.println(std::uint32_t(wm.size()) < x ? std::int64_t(-1) : wm.kth_largest(0, wm.size(), wm.size() - int(x)));
		if (type == 3) out.println(wm.range_freq(0, wm.size(), x + 1));
		if (type == 4) {
			auto result = wm.prev_value(0, wm.size(), x + 1);
			out.println(result ? std::int64_t(*result) : -1);
		}
		if (type == 5) {
			auto result = wm.next_value(0, wm.size(), x);
			out.println(result ? std::int64_t(*result) : -1);
		}
	}
}
