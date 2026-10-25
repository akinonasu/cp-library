#ifndef MULTIPLICATIVE_DIVISORS_HPP
#define MULTIPLICATIVE_DIVISORS_HPP

#include "factorize.hpp"
#include <functional>
#include <type_traits>

template <class PrimePower>
auto multiplicative_divisors(const std::vector<std::pair<std::uint64_t, int>> &factors, PrimePower prime_power, bool sorted = true) {
	using U = std::uint64_t;
	using T = std::decay_t<std::invoke_result_t<PrimePower &, U, int, U>>;
	std::size_t count = 1;
	for (auto [p, e] : factors) {
		assert(p >= 2 && e > 0);
		count *= std::size_t(e) + 1;
	}
	std::vector<std::pair<U, T>> result;
	result.reserve(count);
	result.emplace_back(1, T(1));
	for (auto [p, e] : factors) {
		std::size_t size = result.size();
		U power = 1;
		for (int i = 1; i <= e; ++i) {
			assert(power <= std::numeric_limits<U>::max() / p);
			power *= p;
			T value = std::invoke(prime_power, p, i, power);
			for (std::size_t j = 0; j < size; ++j) {
				assert(result[j].first <= std::numeric_limits<U>::max() / power);
				result.emplace_back(result[j].first * power, result[j].second * value);
			}
		}
	}
	if (sorted) std::sort(result.begin(), result.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
	return result;
}

template <class PrimePower>
auto multiplicative_divisors(std::uint64_t n, PrimePower prime_power, bool sorted = true) {
	return multiplicative_divisors(factorize_count(n), std::move(prime_power), sorted);
}

#endif
