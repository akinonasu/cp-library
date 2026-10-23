#define PROBLEM "https://judge.yosupo.jp/problem/composition_of_formal_power_series"
#include "fps/compose.hpp"
#include "fixtures/fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	auto f = read_fps(input, n), g = read_fps(input, n);
	write_fps(output, compose(f, g));
}
