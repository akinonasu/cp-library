#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/taylor_shift.hpp"
#include <cassert>
#include <random>

using M = mint;
using F = FormalPowerSeries<M>;

int main() {
	std::mt19937 random(93721);
	for (auto [n, m] : std::vector<std::pair<int, int>>{{2049, 2048}, {4097, 1025}, {33, 131073}, {131073, 33}, {4096, 4097}}) {
		F f(n);
		std::vector<M> xs(m);
		for (M &v : f) v = M::word(M::raw(random() % M::mod()).v % M::mod() + M::mod());
		for (int i = 0; i < m; ++i) xs[i] = i % 4 == 0 ? M(0) : i % 4 == 1 ? M(-1) : M(random());
		const auto values = multipoint_eval(f, xs);
		assert(values.size() == xs.size());
		for (int i = 0; i < m; i += std::max(1, m / 67)) assert(values[i] == f.eval(xs[i]));
		assert(values.back() == f.eval(xs.back()));
		assert(multipoint_eval(F(n), xs) == std::vector<M>(m));
	}
	for (int n : {64, 65, 127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025, 2049, 4097}) {
		F f(n);
		std::vector<M> xs(n);
		for (int i = 0; i < n; ++i) f[i] = random(), xs[i] = i * 7;
		const auto ys = multipoint_eval(f, xs);
		assert(interpolate(xs, ys) == f);
	}
	for (int n : {255, 256, 257, 511, 512, 513, 1023, 1024, 1025, 4097, 65536, 524288}) {
		F f(n);
		for (M &v : f) v = random();
		for (M c : {M(1), M(-1), M(17)}) {
			const auto shifted = taylor_shift(f, c);
			assert(shifted.size() == f.size());
			for (M x : {M(0), M(1), M(29)}) assert(shifted.eval(x) == f.eval(x + c));
			assert(taylor_shift(shifted, -c) == f);
		}
	}
}
