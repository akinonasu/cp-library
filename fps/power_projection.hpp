#ifndef CP_FPS_POWER_PROJECTION_HPP
#define CP_FPS_POWER_PROJECTION_HPP

#include "fps.hpp"
#include <numeric>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> power_projection(const FormalPowerSeries<T> &f, const FormalPowerSeries<T> &weights, int deg = -1) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	if (deg < 0) deg = static_cast<int>(weights.size());
	assert(deg >= 0 && deg <= (1 << 22) && weights.size() <= (1U << 21));
	if (deg == 0) return {};
	if (deg == 1) return F{weights.empty() ? T(0) : weights[0]};
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
		typename O::W inner(std::min(f.size(), weights.size()));
		for (int i = 0; i < static_cast<int>(inner.size()); ++i) inner[i] = fps_detail::Mint::raw(f[i].val());
		return O::export_work(power_projection<fps_detail::Mint>(inner, O::import_work(weights), deg));
	} else {
		int n = static_cast<int>(weights.size());
		while (n && weights[n - 1] == T(0)) --n;
		if (n == 0) return F(deg);
		int length = std::min<int>(n, f.size()), first = 1;
		while (first < length && f[first] == T(0)) ++first;
		T c = f.empty() ? T(0) : f[0];
		if (first >= length) {
			F result(deg);
			T value = weights[0];
			for (auto &x : result) x = value, value *= c;
			return result;
		}
		if (first > 1) {
			int stride = first;
			for (int i = first + 1; i < length && stride > 1; ++i)
				if (f[i] != T(0)) stride = std::gcd(stride, i);
			if (stride > 1) {
				F compressed((length - 1) / stride + 1), w((n - 1) / stride + 1);
				for (int i = 0; i < static_cast<int>(compressed.size()); ++i) compressed[i] = f[i * stride];
				for (int i = 0; i < static_cast<int>(w.size()); ++i) w[i] = weights[i * stride];
				return power_projection(compressed, w, deg);
			}
		}
		int count = std::min(deg, (n - 1) / first + 1);
		F result(count);
		int last = length - 1;
		while (f[last] == T(0)) --last;
		if (first == last) {
			T value = 1;
			for (int i = 0; i < count; ++i) result[i] = weights[i * first] * value, value *= f[first];
		} else if (n <= 32 || count <= 8) {
			F h(f.begin(), f.begin() + last + 1), power{T(1)};
			h[0] = 0;
			for (int i = 0; i < count; ++i) {
				T value = 0;
				for (int j = 0; j < static_cast<int>(power.size()); ++j) value += power[j] * weights[j];
				result[i] = value;
				if (i + 1 < count) {
					power = F::multiply(power, h);
					if (static_cast<int>(power.size()) > n) power.resize(n);
				}
			}
		} else {
			int size = 1;
			while (size < n) size <<= 1;
			assert(size <= (1 << 21));
			fps_detail::NTT t(size);
			fps_detail::NTT::CompositionWords storage(static_cast<unsigned int *>(::operator new[](8 * size * sizeof(unsigned int), std::align_val_t(64))));
			auto *p = storage.get(), *q = p + 4 * size;
			std::copy_n(O::words(weights), n, p);
			std::fill_n(p + n, size - n, 0u);
			q[0] = 0;
			for (int i = 1; i <= last; ++i) q[i] = (-f[i]).v;
			std::fill_n(q + last + 1, 2 * size - last - 1, 0u);
			bool unit = n == size && weights[n - 1] == T(1) &&
				std::all_of(weights.begin(), weights.begin() + n - 1, [](T x) { return x == T(0); });
			auto factor = t.reversion_projection(p, q, size, unit ? nullptr : p);
			t.scale(p, count, t.out(factor));
			for (int i = 0; i < count; ++i) result[i] = T::word(p[i]);
		}
		if (c == T(0)) {
			result.resize(deg);
			return result;
		}
		F b(deg);
		T fact = 1;
		for (int i = 1; i < deg; ++i) fact *= i;
		T invfact = fact.inv();
		for (int i = deg - 1; i >= 0; --i) {
			b[i] = invfact;
			if (i < count) result[i] *= invfact;
			invfact *= i;
		}
		O::geometric_scale(b, T(1), c);
		result = F::multiply(result, b);
		result.resize(deg);
		fact = 1;
		for (int i = 0; i < deg; ++i) result[i] *= fact, fact *= i + 1;
		return result;
	}
}

template<class T>
FormalPowerSeries<T> power_projection(const FormalPowerSeries<T> &f, int n, int deg = -1) {
	assert(n >= 0 && n < (1 << 21));
	FormalPowerSeries<T> weights(n + 1);
	weights[n] = 1;
	return power_projection(f, weights, deg);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
