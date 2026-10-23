#ifndef CP_FPS_BOSTAN_MORI_HPP
#define CP_FPS_BOSTAN_MORI_HPP

#include "fps.hpp"
#include "../components/fps_bostan.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
T bostan_mori(FormalPowerSeries<T> p, FormalPowerSeries<T> q, long long k) {
	using F = FormalPowerSeries<T>;
	using base = std::vector<T>;
	using O = fps_detail::Operations<T>;
	using Word = typename fps_detail::Operations<T>::Word;
	if constexpr (!std::is_same_v<T, mint>) {
		return T::raw(bostan_mori<mint>(O::import_work(p), O::import_work(q), k).val());
	} else {
		assert(k >= 0 && !q.empty() && q[0] != T(0));
		O::trim(p);
		O::trim(q);
		if (p.empty()) return 0;
		auto coefficient = [](const base &p, const base &q, int k) {
			F inverse(q.begin(), q.begin() + std::min<int>(k + 1, q.size()));
			inverse = inverse.inv(k + 1);
			return O::dot(p.data(), inverse.data() + k, std::min<int>(k + 1, p.size()));
		};
		if (k < 32) return coefficient(p, q, k);
		T answer = 0;
		if (p.size() >= q.size()) {
			auto [quotient, remainder] = p.divmod(q);
			if (k < (long long)quotient.size()) answer = quotient[k];
			p = std::move(remainder);
		}
		if (p.empty()) return answer;
		if (q.size() == 2) return answer + p[0] / q[0] * (-q[1] / q[0]).pow(k);
		const int d = static_cast<int>(q.size()) - 1;
		if (k < d) return answer + coefficient(p, q, k);
		int n = 1;
		while (n < d) n <<= 1;
		assert(n <= (1 << 22));
		const T half = T(2).inv(), inv = T(n).inv();
		T correction = 1;
		const T step = T(3).pow((T::mod() - 1) / (2 * n));
		base powers(n), twists(n);
		auto storage = fps_detail::allocate_fps_words(4 * std::size_t(n));
		Word *a = storage.get(), *b = a + 2 * n;
		std::copy_n(O::words(p), p.size(), a);
		std::fill(a + p.size(), a + 2 * n, 0u);
		std::copy_n(O::words(q), q.size(), b);
		std::fill(b + q.size(), b + 2 * n, 0u);
		auto &plan = O::transform(n);
		for (int i = 0; i < n; ++i) O::words(twists)[i] = plan.inverse_root_quotients[i] * (0u - T::mod());
		T lead = q.back();
		std::fill(powers.begin(), powers.end(), T(1));
		O::geometric_scale(powers, inv, step);
		plan.template fft<false>(a, 2 * n);
		plan.template fft<false>(b, 2 * n);
		auto doubling = [&](Word *v, T leading) {
			Word *odd = v + n;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
			if (n >= 8 && fps_detail::NTT::vectorized()) {
				fps_detail::bostan_detail::inverse_twisted(plan, v, odd, O::words(powers), n);
				odd[0] = fps_detail::NTT::sub(odd[0], (T(2) * leading).v);
			} else
#endif
			{
				std::copy_n(v, n, odd);
				plan.template fft<true>(odd, n);
				odd[0] = fps_detail::NTT::sub(odd[0], (T(2 * n) * leading).v);
				fps_detail::NTT::product(odd, odd, O::words(powers), n);
			}
			plan.template fft<false>(odd, n);
		};
		while (k) {
			if (k & 1) fps_detail::NTT::reduce_pairs<true>(a, b, O::words(twists), n);
			else fps_detail::NTT::reduce_pairs<false>(a, b, O::words(twists), n);
			correction *= half;
			lead = (d & 1 ? -lead : lead) * lead;
			k >>= 1;
			if (k < d) break;
			doubling(a, T(0));
			doubling(b, d == n ? lead : T(0));
		}
		if (k) {
			base aa(n), bb(n);
			std::copy_n(a, n, O::words(aa));
			std::copy_n(b, n, O::words(bb));
			O::intt(aa);
			O::intt(bb);
			if (d == n) bb[0] -= lead;
			return answer + correction * coefficient(aa, bb, k);
		}
		T numerator = 0, denominator = 0;
		for (int i = 0; i < n; ++i) numerator += T::word(a[i]), denominator += T::word(b[i]);
		if (d == n) denominator -= T(n) * lead;
		return answer + correction * numerator / denominator;
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
