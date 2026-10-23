#ifndef CP_COMPONENTS_FPS_COMPOSITION_GENERIC_HPP
#define CP_COMPONENTS_FPS_COMPOSITION_GENERIC_HPP

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

template<class F>
F power_projection(const F &f, int n, int deg = -1) {
	assert(0 <= n && n < (1 << 21));
	F weights(n + 1);
	weights[n] = 1;
	return fps_detail::composition_fallback::power_projection(f, weights, deg);
}

template<class F>
F compositional_inverse(const F &f, int deg = -1) {
	using T = typename F::value_type;
	if (deg < 0) deg = int(f.size());
	assert(0 <= deg && deg <= (1 << 21) && (f.empty() || f[0] == T(0)));
	if (deg <= 1) return F(deg);
	assert(f.size() >= 2 && f[1] != T(0));
	F projected = fps_detail::composition_fallback::power_projection(f, deg - 1), h(deg - 1);
	std::vector<T> inverse(deg, T(1));
	for (int i = 2; i < deg; ++i) inverse[i] = -T(T::mod() / i) * inverse[T::mod() % i];
	for (int i = 1; i < deg; ++i) h[deg - 1 - i] = projected[i] * inverse[i];
	h /= h[0];
	h = (h.log() * -inverse[deg - 1]).exp() / f[1];
	h.insert(h.begin(), T(0));
	return h;
}
}

#endif
