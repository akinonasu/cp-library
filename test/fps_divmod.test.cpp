#define PROBLEM "https://judge.yosupo.jp/problem/division_of_polynomials"
#include "fixtures/fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0; input.read(n); input.read(m);
	auto f = read_fps(input, n), g = read_fps(input, m);
	auto [q, r] = f.divmod(g);
	output.write(q.size()); output.write(' '); output.println(r.size());
	write_fps(output, q); write_fps(output, r);
}
