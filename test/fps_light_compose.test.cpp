#define PROBLEM "https://judge.yosupo.jp/problem/composition_of_formal_power_series"
#include "fps/compose_light.hpp"
#include "fixtures/fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	auto f = read_light_fps(input, n), g = read_light_fps(input, n);
	write_light_fps(output, light_fps_algorithms::compose(f, g));
}
