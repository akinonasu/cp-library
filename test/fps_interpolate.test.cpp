#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_interpolation"
#include "fps/interpolate.hpp"
#include "fixtures/fps_small_io.hpp"

int main() {
	FpsInput input;
	FpsOutput output;
	int n = 0; input.read(n);
	auto xs = read_series<fps>(input, n), ys = read_series<fps>(input, n);
	write_series(output, interpolate(xs, ys));
}
