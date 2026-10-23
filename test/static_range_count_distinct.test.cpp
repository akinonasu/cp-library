#define PROBLEM "https://judge.yosupo.jp/problem/static_range_count_distinct"
#include "data_structure/wavelet_matrix/wavelet_matrix.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n), previous(n), last(n, -1);
	for (auto &x : a) input.read(x);
	auto values = a;
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	for (int i = 0; i < n; ++i) {
		const auto j = std::lower_bound(values.begin(), values.end(), a[i]) - values.begin();
		previous[i] = last[j];
		last[j] = i;
	}
	WaveletMatrix matrix(previous);
	while (q--) {
		int l = 0, r = 0;
		input.read(l), input.read(r);
		output.println(matrix.range_freq(l, r, l));
	}
}
