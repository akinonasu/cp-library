#ifndef CP_FPS_COMPOSE_HPP
#define CP_FPS_COMPOSE_HPP

#include "fps.hpp"
#include "../components/fps_composition.hpp"
#include <numeric>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> compose(const FormalPowerSeries<T> &outer, const FormalPowerSeries<T> &inner, int deg = -1) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	if (deg < 0) deg = static_cast<int>(outer.size());
	assert(deg >= 0 && (inner.empty() || inner[0] == T(0)));
	if (deg == 0) return {};
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if constexpr (std::is_same_v<T, mint>) {
		if (deg >= 64 && deg <= (1 << 21) && outer.size() > 8 && inner.size() > 2 &&
			outer[std::min(outer.size(), std::size_t(deg)) - 1] != T(0) &&
			inner[1] != T(0) && inner[2] != T(0) && fps_detail::NTT::vectorized()) {
			F result;
			fps_detail::composition_poly::native_compose(outer.data(), outer.size(), inner.data(), inner.size(), result, deg);
			return result;
		}
	}
#endif
	F f = outer.pre(deg), g = inner.pre(deg);
	O::trim(f);
	O::trim(g);
	if (f.size() <= 1 || g.size() <= 1) {
		F result(deg);
		if (!f.empty()) result[0] = f[0];
		return result;
	}
	int first = 1;
	while (g[first] == T(0)) ++first;
	if (static_cast<int>(f.size()) > (deg - 1) / first + 1) {
		f.resize((deg - 1) / first + 1);
		O::trim(f);
		if (f.size() <= 1) {
			F result(deg);
			if (!f.empty()) result[0] = f[0];
			return result;
		}
	}
	if (first + 1 == static_cast<int>(g.size())) {
		F result(deg);
		T power = 1;
		for (int i = 0; i < static_cast<int>(f.size()) && (long long)i * first < deg; ++i)
			result[i * first] = f[i] * power, power *= g[first];
		return result;
	}
	if (first > 1) {
		int stride = first;
		for (int i = first + 1; i < static_cast<int>(g.size()) && stride > 1; ++i)
			if (g[i] != T(0)) stride = std::gcd(stride, i);
		if (stride > 1) {
			F compressed((g.size() - 1) / stride + 1);
			for (int i = 1; i < static_cast<int>(compressed.size()); ++i) compressed[i] = g[i * stride];
			compressed = compose(f, compressed, (deg - 1) / stride + 1);
			F result(deg);
			for (int i = 0; i < static_cast<int>(compressed.size()); ++i) result[i * stride] = compressed[i];
			return result;
		}
	}
	if (deg <= 32 || f.size() <= 8) {
		F result;
		for (int i = (static_cast<int>(f.size())) - 1; i >= 0; --i) {
			result = F::multiply(result, g).pre(deg);
			result += f[i];
		}
		result.resize(deg);
		return result;
	}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if constexpr (std::is_same_v<T, mint>) {
		if (fps_detail::NTT::vectorized() && deg <= (1 << 21)) {
			F result;
			fps_detail::composition_poly::native_compose(f.data(), f.size(), g.data(), g.size(), result, deg);
			return result;
		}
	}
#endif
	int size = 1;
	while (size < deg) size <<= 1;
	assert(size <= (1 << 22));
	fps_detail::NTT plan(size);
	std::vector<unsigned int> p(2 * size), q(size);
	for (int i = 0; i < static_cast<int>(f.size()); ++i) {
		if constexpr (std::is_same_v<T, mint>) p[i] = f[i].v;
		else p[i] = plan.in(f[i].val());
	}
	for (int i = 0; i < static_cast<int>(g.size()); ++i) {
		if constexpr (std::is_same_v<T, mint>) q[i] = g[i].v;
		else q[i] = plan.in(g[i].val());
	}
	unsigned int a = plan.one, b = plan.one;
	plan.compose(p.data(), q.data(), 1, size, a, b);
	F result(deg);
	for (int i = 0; i < deg; ++i) {
		if constexpr (std::is_same_v<T, mint>) result[i] = T::word(p[i]);
		else result[i] = T::raw(plan.out(p[i]));
	}
	return result;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
