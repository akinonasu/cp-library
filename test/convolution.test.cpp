#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"
#include "../convolution/convolution.hpp"
#include "fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0; input.read(n); input.read(m);
	auto a = read_fps(input, n), b = read_fps(input, m);
	write_fps(output, convolution(a, b));
}
