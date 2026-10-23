#define PROBLEM "https://judge.yosupo.jp/problem/pow_of_formal_power_series_sparse"
#include "fixtures/fps_bulk_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, k = 0;
	long long exponent = 0;
	input.read(n), input.read(k), input.read(exponent);
	fps a(n);
	for (int i = 0; i < k; ++i) {
		int degree = 0;
		unsigned value = 0;
		input.read(degree), input.read(value);
		a[degree] = mint::raw(value);
	}
	write_fps(output, a.pow(exponent));
}
