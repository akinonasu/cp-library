#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_interpolation"
#include "fps/interpolate.hpp"
#include "fixtures/fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	auto xs = read_fps(input, n), ys = read_fps(input, n);
	write_fps(output, interpolate(xs, ys));
}
