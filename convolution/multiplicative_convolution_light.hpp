#ifndef MULTIPLICATIVE_CONVOLUTION_LIGHT_HPP
#define MULTIPLICATIVE_CONVOLUTION_LIGHT_HPP

#include "../components/modular_power_sequence.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>

#include "../math/number_theory/is_prime.hpp"
#include "convolution_light.hpp"

namespace light_convolution {
template <class T>
std::vector<T> multiplicative_convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	assert(a.size() == b.size() && a.size() <= std::numeric_limits<std::uint32_t>::max() && is_prime(a.size()));
	std::uint32_t p = a.size();
	std::vector<std::uint32_t> factors;
	std::uint32_t remaining = p - 1;
	for (std::uint32_t d = 2; std::uint64_t(d) * d <= remaining; ++d) {
		if (remaining % d) continue;
		factors.push_back(d);
		do { remaining /= d; } while (remaining % d == 0);
	}
	if (remaining > 1) factors.push_back(remaining);
	auto power = [=](std::uint64_t x, std::uint32_t k) {
		std::uint64_t result = 1;
		for (; k; k >>= 1, x = x * x % p)
			if (k & 1) result = result * x % p;
		return result;
	};
	std::uint32_t root = 1;
	while (std::any_of(factors.begin(), factors.end(), [&](auto q) { return power(root, (p - 1) / q) == 1; })) ++root;
	std::vector<T> x(p - 1), y(&a == &b ? 0 : p - 1), result(p, T(0));
	auto powers = modular_power_sequence_detail::generate(p - 1, root, p);
	for (std::uint32_t i = 0; i < p - 1; ++i) {
		x[i] = a[powers[i]];
		if (&a != &b) y[i] = b[powers[i]];
	}

	auto product = &a == &b ? light_convolution::convolution(x, x) : light_convolution::convolution(x, y);
	for (std::size_t i = p - 1; i < product.size(); ++i) product[i - (p - 1)] += product[i];
	for (std::uint32_t i = 0; i < p - 1; ++i) result[powers[i]] = product[i];
	T sum_a(0), sum_b(0);
	for (std::size_t i = 1; i < a.size(); ++i) sum_a += a[i];
	for (const T &v : b) sum_b += v;
	result[0] = a[0] * sum_b;
	result[0] += sum_a * b[0];
	return result;
}

}

#endif
