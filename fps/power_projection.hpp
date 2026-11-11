#ifndef CP_FPS_POWER_PROJECTION_HPP
#define CP_FPS_POWER_PROJECTION_HPP

#include "fps.hpp"
#include "../components/fps_projection_blocked.hpp"
#include "../components/fps_composition_generic.hpp"
#include <numeric>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#ifndef CP_FPS_PROJECT_API
#define CP_FPS_PROJECT_API 1
template<class T>
FormalPowerSeries<T> power_projection(const FormalPowerSeries<T> &f, const FormalPowerSeries<T> &w, int deg = -1) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	if (deg < 0) deg = static_cast<int>(w.size());
	assert(deg >= 0 && deg <= (1 << 22) && w.size() <= (1U << 21));
	if (deg == 0) return {};
	if (deg == 1) return F{w.empty() ? T(0) : w[0]};
	if constexpr (!std::is_same_v<T, mint>) {
		typename O::W inner(std::min(f.size(), w.size()));
		for (int i = 0; i < static_cast<int>(inner.size()); ++i) inner[i] = mint::raw(f[i].val());
		return O::export_work(power_projection<mint>(inner, O::import_work(w), deg));
	} else {
		int n = static_cast<int>(w.size());
		while (n && w[n - 1] == T(0)) --n;
		if (n == 0) return F(deg);
		int len = std::min<int>(n, f.size()), first = 1;
		while (first < len && f[first] == T(0)) ++first;
		T c = f.empty() ? T(0) : f[0];
		if (first >= len) {
			F res(deg);
			T value = w[0];
			for (auto &x : res) x = value, value *= c;
			return res;
		}
		if (first > 1) {
			int stride = first;
			for (int i = first + 1; i < len && stride > 1; ++i)
				if (f[i] != T(0)) stride = std::gcd(stride, i);
			if (stride > 1) {
				F compressed((len - 1) / stride + 1), cw((n - 1) / stride + 1);
				for (int i = 0; i < static_cast<int>(compressed.size()); ++i) compressed[i] = f[i * stride];
				for (int i = 0; i < static_cast<int>(cw.size()); ++i) cw[i] = w[i * stride];
				return power_projection(compressed, cw, deg);
			}
		}
		int count = std::min(deg, (n - 1) / first + 1);
		F res;
		int last = len - 1;
		while (f[last] == T(0)) --last;
		if (first == last) {
			res.resize(count);
			T value = 1;
			for (int i = 0; i < count; ++i) res[i] = w[i * first] * value, value *= f[first];
		} else if (n <= 32 || count <= 8) {
			res.resize(count);
			F h(f.begin(), f.begin() + last + 1), power{T(1)};
			h[0] = 0;
			for (int i = 0; i < count; ++i) {
				T value = 0;
				for (int j = 0; j < static_cast<int>(power.size()); ++j) value += power[j] * w[j];
				res[i] = value;
				if (i + 1 < count) {
					power = F::multiply(power, h);
					if (static_cast<int>(power.size()) > n) power.resize(n);
				}
			}
		} else
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if (n <= (1 << 21) && fps_detail::NTT::vectorized()) {
			fps_detail::composition_poly::native_weighted_projection(f.data(), len, w.data(), res, n, count);
		} else
#endif
		{
			F inner(f.begin(), f.begin() + last + 1);
			inner[0] = 0;
			res = fps_detail::composition_fallback::power_projection(inner, F(w.begin(), w.begin() + n), count);
		}
		if (c == T(0)) {
			res.resize(deg);
			return res;
		}
		F b(deg);
		T fact = 1;
		for (int i = 1; i < deg; ++i) fact *= i;
		T invfact = fact.inv();
		for (int i = deg - 1; i >= 0; --i) {
			b[i] = invfact;
			if (i < count) res[i] *= invfact;
			invfact *= i;
		}
		O::geometric_scale(b, T(1), c);
		res = F::multiply(res, b);
		res.resize(deg);
		fact = 1;
		for (int i = 0; i < deg; ++i) res[i] *= fact, fact *= i + 1;
		return res;
	}
}

template<class T>
FormalPowerSeries<T> power_projection(const FormalPowerSeries<T> &f, int n, int deg = -1) {
	assert(n >= 0 && n < (1 << 21));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if constexpr (std::is_same_v<T, mint>) {
		if (n >= 64 && f.size() > 2 && f[0] == T(0) && f[1] != T(0) && f[2] != T(0) &&
			fps_detail::NTT::vectorized()) {
			const int count = deg < 0 ? n + 1 : deg;
			assert(count >= 0 && count <= (1 << 22));
			if (count == 0) return {};
			FormalPowerSeries<T> res;
			fps_detail::composition_poly::native_projection(f.data(), f.size(), res, n + 1);
			res.resize(count);
			return res;
		}
	}
#endif
	FormalPowerSeries<T> w(n + 1);
	w[n] = 1;
	return power_projection(f, w, deg);
}

#endif

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
