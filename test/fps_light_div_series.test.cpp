#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_formal_power_series"
#include "fixtures/fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	auto denominator = read_light_fps(input, n);
	fps numerator(n);
	numerator[0] = 1;
	write_light_fps(output, numerator / denominator);
}
