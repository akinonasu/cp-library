#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_taylor_shift"
#include "fps/taylor_shift.hpp"
#include "fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; unsigned c = 0; input.read(n); input.read(c);
	write_fps(output, taylor_shift(read_fps(input, n), mint::raw(c)));
}
