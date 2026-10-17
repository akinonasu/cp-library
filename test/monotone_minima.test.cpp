#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_arbitrary"
#include "components/monotone_minima.hpp"
#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	std::vector<std::int64_t> a(n), b(m);
	for (auto &x : a) input.read(x);
	for (auto &x : b) input.read(x);
	auto best = monotone_minima(n + m - 1, m, [&](int row, int x, int y) {
		if (row < y) return false;
		if (row - x >= n) return true;
		return a[row - y] + b[y] < a[row - x] + b[x];
	});
	for (int i = 0; i < n + m - 1; ++i) {
		if (i) output.write(' ');
		output.write(a[i - best[i]] + b[best[i]]);
	}
	output.write('\n');
}
