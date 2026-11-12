#ifndef CP_COMPONENTS_FPS_PROJECTION_GENERIC_HPP
#define CP_COMPONENTS_FPS_PROJECTION_GENERIC_HPP

#include "fps_composition_stages.hpp"

namespace fps_detail::composition_fallback {
template<class F>
F power_projection(const F &f, const F &weights, int deg = -1) {
	using T = typename F::value_type;
	if (deg < 0) deg = int(weights.size());
	assert(0 <= deg && deg <= (1 << 22) && weights.size() <= (std::size_t(1) << 21));
	F result(deg);
	if (!deg || weights.empty()) return result;
	int n = int(weights.size());
	while (n && weights[n - 1] == T(0)) --n;
	if (!n) return result;
	const int count = std::min(deg, n);
	F value(weights.begin(), weights.begin() + n);
	std::reverse(value.begin(), value.end());
	for (const auto &stage : composition_stages(f, n, count)) {
		const int width = stage.width, next = (width + 1) / 2;
		F numerator(2 * width * stage.before), denominator(2 * std::max(stage.even.size(), stage.odd.size()));
		for (int y = 0; y < stage.before; ++y) for (int x = 0; x < width; ++x)
			numerator[2 * y * width + x] = value[y * width + x];
		for (int i = 0; i < int(stage.even.size()); ++i) denominator[2 * i] = stage.even[i];
		for (int i = 0; i < int(stage.odd.size()); ++i) denominator[2 * i + 1] = -stage.odd[i];
		trim(numerator), trim(denominator);
		F product = multiply(numerator, denominator);
		value.assign(next * stage.after, T(0));
		for (int y = 0; y < stage.after; ++y) for (int x = 0; x < next; ++x) {
			int i = 2 * y * width + 2 * x + stage.parity;
			if (i < int(product.size())) value[y * next + x] = product[i];
		}
	}
	std::copy(value.begin(), value.end(), result.begin());
	const T constant = f.empty() ? T(0) : f[0];
	if (constant == T(0)) return result;
	F factorial(deg, T(1)), inverse(deg, T(1)), b(deg);
	for (int i = 1; i < deg; ++i) factorial[i] = factorial[i - 1] * T(i);
	inverse.back() = factorial.back().inv();
	for (int i = deg - 1; i; --i) inverse[i - 1] = inverse[i] * T(i);
	T power = 1;
	for (int i = 0; i < deg; ++i) result[i] *= inverse[i], b[i] = power * inverse[i], power *= constant;
	result = multiply(result, b).pre(deg);
	for (int i = 0; i < deg; ++i) result[i] *= factorial[i];
	return result;
}

}

#endif
