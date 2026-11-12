#ifndef CP_COMPONENTS_FPS_COMPOSE_GENERIC_HPP
#define CP_COMPONENTS_FPS_COMPOSE_GENERIC_HPP

#include "fps_composition_stages.hpp"

namespace fps_detail::composition_fallback {
template<class F>
F composition_correlation(const F &a, const F &b) {
	if (b.empty()) return {};
	return multiply(a, F(b.rbegin(), b.rend()));
}
template<class F>
F compose(const F &outer, const F &inner, int deg = -1) {
	using T = typename F::value_type;
	if (deg < 0) deg = int(outer.size());
	assert(0 <= deg && deg <= (1 << 22) && (inner.empty() || inner[0] == T(0)));
	F result(deg);
	if (!deg || outer.empty()) return result;
	const int count = std::min(deg, int(outer.size()));
	if (count == 1) { result[0] = outer[0]; return result; }
	auto stages = composition_stages(inner, deg, count);
	F value(count);
	std::copy_n(outer.begin(), count, value.begin());
	if (!stages.empty()) value.resize(stages.back().after);
	for (int level = int(stages.size()) - 1; level >= 0; --level) {
		const auto &stage = stages[level];
		const int n = stage.width, next = (n + 1) / 2;
		F weights(n * stage.after);
		for (int y = 0; y < stage.after; ++y) for (int x = 0; x < next; ++x)
			weights[y * n + x] = value[y * next + x];
		trim(weights);
		F even = composition_correlation(weights, stage.even);
		F odd = composition_correlation(weights, stage.odd);
		auto get = [](const F &a, int i) { return 0 <= i && i < int(a.size()) ? a[i] : T(0); };
		value.assign(n * stage.before, T(0));
		for (int y = 0; y < stage.before; ++y) for (int x = 0; x < n; ++x) {
			int i = y * n + x / 2;
			if ((x & 1) == stage.parity)
				value[y * n + x] = get(even, i + int(stage.even.size()) - 1);
			else value[y * n + x] = -get(odd, i + int(stage.odd.size()) - 1 + (stage.parity == 0));
		}
	}
	std::reverse_copy(value.begin(), value.begin() + deg, result.begin());
	return result;
}
}

#endif
