#ifndef CP_FPS_TAYLOR_SHIFT_HPP
#define CP_FPS_TAYLOR_SHIFT_HPP

#include "fps.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> taylor_shift(FormalPowerSeries<T> f, std::type_identity_t<T> c) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	using Word = typename fps_detail::Operations<T>::Word;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>)
		return O::export_work(taylor_shift<fps_detail::Mint>(O::import_work(f), fps_detail::Mint::raw(c.val())));
	const int n = static_cast<int>(f.size());
	if (n <= 1 || c == T(0)) return f;
	assert(unsigned(n) < unsigned(T::mod()));
	if constexpr (std::is_same_v<T, fps_detail::Mint>) {
		F a(n), b(n), weights;
		Word fact = fps_detail::NTT::one;
		for (int i = 0; i < n; ++i) {
			O::words(a)[i] = fact;
			fact = (unsigned long long)fact * (i + 1) % T::mod();
		}
		const Word last = T::word(O::words(a)[n - 1]).inv().v;
		Word invfact = last;
		for (int i = n - 1; i >= 0; --i) {
			O::words(b)[i] = invfact;
			invfact = (unsigned long long)invfact * i % T::mod();
		}
		fps_detail::NTT::product(O::words(a), O::words(a), O::words(f), n);
		std::reverse(a.begin(), a.end());
		weights = b;
		O::geometric_scale(b, T(1), c);
		F prod = F::multiply(a, b);
		std::reverse(prod.begin(), prod.begin() + n);
		fps_detail::NTT::product(O::words(f), O::words(prod), O::words(weights), n);
		return f;
	}
	const auto &inv = O::inverses(n);
	F a(n), b(n);
	T fact = 1, invfact = 1, cpow = 1;
	for (int i = 0; i < n; ++i) {
		a[n - 1 - i] = f[i] * fact;
		b[i] = cpow * invfact;
		fact *= i + 1;
		invfact *= inv[i + 1];
		cpow *= c;
	}
	F prod = F::multiply(a, b);
	invfact = 1;
	for (int i = 0; i < n; ++i) {
		f[i] = prod[n - 1 - i] * invfact;
		invfact *= inv[i + 1];
	}
	return f;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
