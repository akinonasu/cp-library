#ifndef CP_FPS_TAYLOR_SHIFT_HPP
#define CP_FPS_TAYLOR_SHIFT_HPP

#include "fps.hpp"
#include "../components/mint_factorials.hpp"
#include "../components/fps_taylor_blocked.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> taylor_shift(FormalPowerSeries<T> f, std::type_identity_t<T> c) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, mint>)
		return O::export_work(taylor_shift<mint>(O::import_work(f), mint::raw(c.val())));
	else {
		const int n = static_cast<int>(f.size());
		if (n <= 1 || c == T(0)) return f;
		assert(unsigned(n) < unsigned(T::mod()));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if (n >= 256 && n <= (1 << 19) && fps_detail::NTT::vectorized()) {
			fps_detail::taylor_detail::shift(f, c);
			return f;
		}
#endif
		F a(n), b(n), weights;
		mint_detail::factorials(O::words(a), O::words(b), n);
		fps_detail::NTT::product(O::words(a), O::words(a), O::words(f), n);
		std::reverse(a.begin(), a.end());
		weights = b;
		O::geometric_scale(b, T(1), c);
		F prod = F::multiply(a, b);
		std::reverse(prod.begin(), prod.begin() + n);
		fps_detail::NTT::product(O::words(f), O::words(prod), O::words(weights), n);
		return f;
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
