#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"
#include "fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0; input.read(n); input.read(m);
	auto a = read_light_fps(input, n), b = read_light_fps(input, m);
	write_light_fps(output, a * b);
}
