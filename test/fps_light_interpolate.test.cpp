#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_interpolation"
#include "fps/fps_light_algorithms.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	std::vector<mint> xs(n), ys(n);
	for (auto &x : xs) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	for (auto &x : ys) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	for (auto x : light_fps_algorithms::interpolate(xs, ys)) {
		out.write(x.val()); out.write(' ');
	}
	out.write('\n');
}
