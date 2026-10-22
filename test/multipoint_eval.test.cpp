#define PROBLEM "https://judge.yosupo.jp/problem/multipoint_evaluation"
#include "../fps/multipoint_eval.hpp"
#include "fixtures/fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0; input.read(n); input.read(m);
	auto f = read_fps(input, n), xs = read_fps(input, m);
	write_fps(output, multipoint_eval(f, xs));
}
