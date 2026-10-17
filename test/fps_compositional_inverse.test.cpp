#define PROBLEM "https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series"
#include "fps/compositional_inverse.hpp"
#include "fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	write_fps(output, compositional_inverse(read_fps(input, n)));
}
