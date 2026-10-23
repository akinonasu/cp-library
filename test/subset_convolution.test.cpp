#define PROBLEM "https://judge.yosupo.jp/problem/subset_convolution"
#include "../convolution/subset_convolution.hpp"
#include "../math/mint.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int exponent = 0;
	input.read(exponent);
	int n = 1 << exponent;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	for (auto &x : b) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	for (auto x : subset_convolution(a, b)) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}
