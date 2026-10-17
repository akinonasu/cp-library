#ifndef CP_FPS_BOSTAN_MORI_HPP
#define CP_FPS_BOSTAN_MORI_HPP

#include "fps.hpp"

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
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) return T::raw(bostan_mori<fps_detail::Mint>(O::import_work(p), O::import_work(q), k).val());
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
	base powers(n), twists(n), a = p, b = q;
	if constexpr (std::is_same_v<T, fps_detail::Mint>) {
		auto &plan = O::transform(n);
		for (int i = 0; i < n; ++i) O::words(twists)[i] = plan.inverse_root_quotients[i] * (0u - T::mod());
	} else {
		const auto &roots = O::inverse_ntt_roots(2 * n);
		for (int i = 0; i < n; ++i) twists[i] = roots[2 * i];
	}
	T lead = q.back();
	std::fill(powers.begin(), powers.end(), T(1));
	O::geometric_scale(powers, inv, step);
	a.resize(2 * n);
	b.resize(2 * n);
	O::ntt(a);
	O::ntt(b);
	auto doubling = [&](base &v, T leading) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			Word *odd = O::words(v) + n;
			std::copy_n(O::words(v), n, odd);
			auto &plan = O::transform(std::max(1, n / 2));
			plan.template fft<true>(odd, n);
			odd[0] = fps_detail::NTT::sub(odd[0], (T(2 * n) * leading).v);
			fps_detail::NTT::product(odd, odd, O::words(powers), n);
			plan.template fft<false>(odd, n);
		} else {
			base odd = v;
			O::ntt_inverse(odd);
			odd[0] -= T(2 * n) * leading;
			O::pointwise(odd, odd, powers, n);
			O::ntt(odd);
			v.insert(v.end(), odd.begin(), odd.end());
		}
	};
	while (k) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (k & 1) fps_detail::NTT::reduce_pairs<true>(O::words(a), O::words(b), O::words(twists), n);
			else fps_detail::NTT::reduce_pairs<false>(O::words(a), O::words(b), O::words(twists), n);
		} else {
			for (int i = 0; i < n; ++i) {
				T u = a[2 * i] * b[2 * i + 1], v = a[2 * i + 1] * b[2 * i];
				a[i] = k & 1 ? (u - v) * twists[i] : u + v;
				b[i] = b[2 * i] * b[2 * i + 1];
			}
		}
		correction *= half;
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
			a.resize(n);
			b.resize(n);
		}
		lead = (d & 1 ? -lead : lead) * lead;
		k >>= 1;
		if (k < d) break;
		doubling(a, T(0));
		doubling(b, d == n ? lead : T(0));
	}
	a.resize(n);
	b.resize(n);
	if (k) {
		O::intt(a);
		O::intt(b);
		if (d == n) b[0] -= lead;
		return answer + correction * coefficient(a, b, k);
	}
	T numerator = 0, denominator = 0;
	for (int i = 0; i < n; ++i) numerator += a[i], denominator += b[i];
	if (d == n) denominator -= T(n) * lead;
	return answer + correction * numerator / denominator;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
