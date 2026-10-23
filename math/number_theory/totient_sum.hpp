#ifndef TOTIENT_SUM_HPP
#define TOTIENT_SUM_HPP

#include "prime_sieve.hpp"
#include "kth_root.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace totient_detail {
template<class T, class I> T value(I x) {
	if constexpr (requires { std::uint64_t(T::mod()); }) {
		auto modulus = std::uint64_t(T::mod());
		if constexpr (sizeof(I) <= 8) {
			if constexpr (!std::is_signed_v<I>) return T(std::uint64_t(x) % modulus);
			else if (modulus <= std::uint64_t(INT64_MAX)) return T(std::int64_t(x) % std::int64_t(modulus));
		}
		auto residue = __int128_t(x) % __int128_t(modulus);
		return T(std::uint64_t(residue < 0 ? residue + modulus : residue));
	} else return T(x);
}

template<class T, class Count> T compute(std::uint64_t n) {
	using U = std::uint64_t;
	const U root = kth_root(n, 2);
	auto primes = prime_sieve(root + 1);
	primes.push_back(root + 1);
	const auto k = std::upper_bound(primes.begin(), primes.end(), kth_root(root, 3)) - primes.begin();
	const U cutoff = primes[k], m = cutoff * root;
	std::vector<Count> small(root + 2), large(root + 2), mu(root + 2), upper_mu(root + 2);
	std::vector<double> reciprocals(root + 1);
	for (U i = 1; i <= root; ++i) reciprocals[i] = 1.0 / double(i);
	auto divide = [&](U x, U d) -> U {
		if (n <= (U(1) << 53) && std::numeric_limits<double>::is_iec559
			&& std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits >= 53) {
			U q = double(x) * reciprocals[d];
			while (q * d > x) --q;
			while ((q + 1) * d <= x) ++q;
			return q;
		}
		return x / d;
	};
	for (U i = 1; i <= root; ++i) small[i] = i, large[i] = divide(n, i);
	for (std::size_t j = 0; j < std::size_t(k); ++j) {
		U p = primes[j], t = root / p, quotient = n / p;
		for (U i = 1; i <= t; ++i) large[i] -= large[i * p];
		for (U i = t + 1; i <= root; ++i) large[i] -= small[divide(quotient, i)];
		U end = root;
		for (U x = t; x; --x) {
			auto delta = small[x];
			for (U stop = x * p; end >= stop; --end) small[end] -= delta;
		}
	}
	mu[1] = 2;
	for (std::size_t j = k; j + 1 < primes.size() && primes[j] * primes[j] <= m; ++j) {
		U p = primes[j], square = p * p, t = root / p, quotient = n / p, bound = m / p;
		std::size_t next = j + 1;
		for (; primes[next] <= t; ++next) mu[p * primes[next]] += 2;
		for (; primes[next] <= bound; ++next) upper_mu[divide(quotient, primes[next])] += 2;
		t /= p, quotient /= p, bound /= p;
		if (square <= root) ++mu[square];
		else ++upper_mu[quotient];
		for (next = k; primes[next] <= t; ++next) ++mu[square * primes[next]];
		for (; primes[next] <= bound; ++next) ++upper_mu[divide(quotient, primes[next])];
	}
	for (U i = 1; i <= root; ++i) mu[i] += mu[i - 1];
	upper_mu[root] += mu[root];
	for (U i = root; i; --i) upper_mu[i - 1] += upper_mu[i];
	for (U i = 1; i <= root; ++i) mu[i] -= small[i], upper_mu[i] -= large[i];
	struct Rough { U value; int mu; };
	std::vector<Rough> roughs;
	for (U i = 2; i <= root; ++i) if (small[i] != small[i - 1]) roughs.push_back({i, int(mu[i] - mu[i - 1])});
	roughs.push_back({root + 1, 0});
	for (U i = n / m; i; --i) {
		U x = divide(n, i), bound = kth_root(x, 2);
		std::size_t j = 0;
		Count sum = Count(1) - large[i] + small[bound] * mu[bound];
		for (; roughs[j].value <= root / i; ++j) {
			U t = i * roughs[j].value;
			sum -= upper_mu[t] + large[t] * roughs[j].mu;
		}
		for (; roughs[j].value <= bound; ++j) {
			U t = divide(x, roughs[j].value);
			sum -= mu[t] + small[t] * roughs[j].mu;
		}
		upper_mu[i] = sum;
	}
	for (std::size_t j = k; j--;) {
		U p = primes[j], t = root / p, quotient = n / p;
		for (U i = 1; i <= t; ++i) upper_mu[i] -= upper_mu[i * p];
		for (U i = t + 1; i <= root; ++i) upper_mu[i] -= mu[divide(quotient, i)];
		U end = root;
		for (U x = t; x; --x) {
			auto delta = mu[x];
			for (U stop = x * p; end >= stop; --end) mu[end] -= delta;
		}
	}
	auto triangular = [](U x) {
		return x & 1 ? value<T>(x) * value<T>(x / 2 + 1) : value<T>(x / 2) * (value<T>(x) + T(1));
	};
	T answer(0);
	for (U i = 1; i <= root; ++i)
		answer += value<T>(i) * value<T>(upper_mu[i]) + value<T>(mu[i] - mu[i - 1]) * triangular(divide(n, i));
	return answer - triangular(root) * value<T>(mu[root]);
}
}

template<class T = std::uint64_t> T totient_sum(std::uint64_t n) {
	constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;
	using A = std::conditional_t<integer, __uint128_t, T>;
	A answer(0);
	if (n < 64) {
		std::vector<unsigned> phi(n + 1);
		for (unsigned i = 0; i <= n; ++i) phi[i] = i;
		for (unsigned p = 2; p <= n; ++p) if (phi[p] == p)
			for (unsigned i = p; i <= n; i += p) phi[i] -= phi[i] / p;
		for (unsigned i = 1; i <= n; ++i) answer += A(phi[i]);
	} else if (n <= (std::uint64_t(1) << 40)) answer = totient_detail::compute<A, std::int64_t>(n);
	else answer = totient_detail::compute<A, __int128_t>(n);
	if constexpr (integer) { assert(answer <= __uint128_t(std::numeric_limits<T>::max())); }
	return T(answer);
}

#endif
