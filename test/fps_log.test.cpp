#define PROBLEM "https://judge.yosupo.jp/problem/log_of_formal_power_series"
#include "../fps/fps.hpp"
#include "fixtures/fps_bulk_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	write_fps(output, read_fps(input, n).log(n));
}
