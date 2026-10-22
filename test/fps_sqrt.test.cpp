#define PROBLEM "https://judge.yosupo.jp/problem/sqrt_of_formal_power_series"
#include "fixtures/fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	auto root = read_fps(input, n).sqrt(n);
	if (root.empty()) output.println(-1);
	else write_fps(output, root);
}
