#ifndef CP_COMPONENTS_FPS_COMPOSITION_STAGES_HPP
#define CP_COMPONENTS_FPS_COMPOSITION_STAGES_HPP

#include "polynomial_multiply.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

namespace fps_detail::composition_fallback {
template<class F> void trim(F &f) {
	using T = typename F::value_type;
	while (!f.empty() && f.back() == T(0)) f.pop_back();
}

template<class F> F multiply(const F &a, const F &b) {
	return polynomial_detail::Products<F>::multiply(a, b);
}

template<class F> struct CompositionStage {
	int width, before, after, parity;
	F even, odd;
};

template<class F>
std::vector<CompositionStage<F>> composition_stages(const F &f, int n, int count) {
	using T = typename F::value_type;
	std::vector<CompositionStage<F>> stages;
	F q(2 * n);
	q[0] = 1;
	for (int i = 1; i < std::min(n, int(f.size())); ++i) q[n + i] = -f[i];
	int columns = 2, before = 1;
	while (n > 1) {
		const int next = (n + 1) / 2, parity = (n - 1) & 1;
		CompositionStage<F> stage{n, before, std::min(count, before + columns - 1), parity, {}, {}};
		stage.even.resize(n * columns), stage.odd.resize(n * columns);
		for (int y = 0; y < columns; ++y) for (int x = 0; x < n; ++x)
			(x & 1 ? stage.odd : stage.even)[y * n + x / 2] = q[y * n + x];
		trim(stage.even), trim(stage.odd);
		F r = multiply(stage.even, stage.even), s = multiply(stage.odd, stage.odd);
		const int height = std::min(count, 2 * columns - 1);
		q.assign(next * height, T(0));
		for (int y = 0; y < height; ++y) for (int x = 0; x < next; ++x) {
			int i = y * n + x;
			q[y * next + x] = (i < int(r.size()) ? r[i] : T(0)) - (i && i - 1 < int(s.size()) ? s[i - 1] : T(0));
		}
		trim(q);
		columns = std::max(1, (int(q.size()) + next - 1) / next);
		q.resize(next * columns);
		before = stage.after;
		stages.push_back(std::move(stage));
		n = next;
	}
	return stages;
}
}

#endif
