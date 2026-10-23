#define PROBLEM "https://judge.yosupo.jp/problem/multipoint_evaluation"
#include "../fps/multipoint_eval.hpp"
#include "fixtures/fps_small_io.hpp"

int main() {
	FpsInput input;
	FpsOutput output;
	int n = 0, m = 0; input.read(n); input.read(m);
	auto f = read_series<fps>(input, n), xs = read_series<fps>(input, m);
	write_series(output, multipoint_eval(f, xs));
}
