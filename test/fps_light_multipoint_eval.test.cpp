#define PROBLEM "https://judge.yosupo.jp/problem/multipoint_evaluation"
#include "fps/fps_light_algorithms.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n), in.read(m);
	light_fps f(n);
	std::vector<mint> xs(m);
	for (auto &x : f) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	for (auto &x : xs) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	for (auto x : light_fps_algorithms::multipoint_eval(f, xs)) {
		out.write(x.val()); out.write(' ');
	}
	out.write('\n');
}
