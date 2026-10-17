#ifndef CP_FPS_COMPOSITIONAL_INVERSE_HPP
#define CP_FPS_COMPOSITIONAL_INVERSE_HPP

#include "fps.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> compositional_inverse(const FormalPowerSeries<T> &f, int deg = -1) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	if (deg < 0) deg = static_cast<int>(f.size());
	assert(deg >= 0 && (f.empty() || f[0] == T(0)));
	if (deg == 0) return {};
	if (deg == 1) return F(1);
	assert(f.size() >= 2 && f[1] != T(0));
	if constexpr (!std::is_same_v<T, fps_detail::Mint>)
		return O::export_work(compositional_inverse<fps_detail::Mint>(O::import_work(f), deg));
	else {
		T w = f[1].inv();
		int length = std::min<int>(deg, f.size());
		int first = 2;
		while (first < length && f[first] == T(0)) ++first;
		if (first == length) {
			F result(deg);
			result[1] = w;
			return result;
		}
		int size = 1;
		while (size < deg) size <<= 1;
		assert(size <= (1 << 21));

		const int n = deg - 1;
		auto &inv = O::inverses(size);
		F h(n);
		{
			fps_detail::NTT t(size);
			fps_detail::NTT::CompositionWords storage(static_cast<unsigned int *>(::operator new[](8 * size * sizeof(unsigned int), std::align_val_t(64))));
			auto *p = storage.get(), *q = p + 4 * size;
			std::copy_n(O::words(f), length, q);
			std::fill_n(q + length, 2 * size - length, 0u);
			t.geometric(q, length, T(-1).v, w.v);
			auto factor = t.reversion_projection(p, q, size);
			t.product(p + size - n, p + size - n, O::words(inv) + size - n, n);
			for (int i = 0; i < n; ++i) h[i] = T::word(p[size - 1 - i]);
			O::scale_words(h, T(size - 1) * T::word(factor));
		}
		h = h.log(n);
		O::scale_words(h, -inv[size - 1]);
		h = h.exp(n);
		O::scale_words(h, w);
		F result(deg);
		std::copy(h.begin(), h.end(), result.begin() + 1);
		return result;
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
