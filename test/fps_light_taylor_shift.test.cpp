#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_taylor_shift"
#include "fps/fps_light_algorithms.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	unsigned c = 0;
	in.read(n), in.read(c);
	light_fps f(n);
	for (auto &x : f) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	for (auto x : light_fps_algorithms::taylor_shift(std::move(f), mint::raw(c))) {
		out.write(x.val()); out.write(' ');
	}
	out.write('\n');
}
