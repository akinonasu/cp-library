#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_convex"
#include "convolution/min_plus_convolution.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	std::vector<std::int64_t> a(n), b(m);
	for (auto &x : a) input.read(x);
	for (auto &x : b) input.read(x);
	const auto result = min_plus_convolution_convex(a, b);
	for (std::size_t i = 0; i < result.size(); ++i) {
		if (i) output.write(' ');
		output.write(result[i]);
	}
	output.write('\n');
}
