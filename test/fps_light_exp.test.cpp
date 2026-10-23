#define PROBLEM "https://judge.yosupo.jp/problem/exp_of_formal_power_series"
#include "fixtures/fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	write_light_fps(output, read_light_fps(input, n).exp());
}
