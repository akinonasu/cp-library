#ifndef LINEAR_RECURRENCE_GENERIC_HPP
#define LINEAR_RECURRENCE_GENERIC_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

template <class T, class InitialAllocator, class CoefficientAllocator, class Convolve>
T linear_recurrence(const std::vector<T, InitialAllocator> &initial, const std::vector<T, CoefficientAllocator> &coefficients,
		long long k, Convolve &&convolve) {
	assert(k >= 0 && initial.size() == coefficients.size() && initial.size() <= std::size_t(std::numeric_limits<int>::max()));
	const std::size_t d = initial.size();
	if (k < static_cast<long long>(d)) return initial[std::size_t(k) % d];
	if (!d) return T(0);
	auto multiply = [&](const std::vector<T> &a, const std::vector<T> &b) {
		std::vector<T> result = convolve(a, b);
		assert(result.size() == a.size() - 1 + b.size());
		return result;
	};
	std::vector<T> p(initial.begin(), initial.end()), q(d + 1, T(0));
	q[0] = T(1);
	for (std::size_t i = 0; i < d; ++i) q[i + 1] = T(0) - coefficients[i];
	p = multiply(p, q);
	p.erase(p.begin() + d, p.end());
	while (k) {
		std::vector<T> negative = q;
		for (std::size_t i = 1; i < negative.size(); i += 2) negative[i] = T(0) - negative[i];
		std::vector<T> numerator = multiply(p, negative), denominator = multiply(q, negative);
		p.clear();
		for (std::size_t i = std::size_t(k & 1); i < numerator.size(); i += 2) p.push_back(numerator[i]);
		q.clear();
		for (std::size_t i = 0; i < denominator.size(); i += 2) q.push_back(denominator[i]);
		k >>= 1;
	}
	return p[0];
}

#endif
