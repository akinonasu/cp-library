#define PROBLEM "https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series"
#include "fps/compositional_inverse_light.hpp"
#include "fixtures/fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	write_light_fps(output, light_fps_algorithms::compositional_inverse(read_light_fps(input, n)));
}
