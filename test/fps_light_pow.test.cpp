#define PROBLEM "https://judge.yosupo.jp/problem/pow_of_formal_power_series"
#include "fixtures/fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; long long exponent = 0; input.read(n); input.read(exponent);
	write_light_fps(output, read_light_fps(input, n).pow(exponent));
}
