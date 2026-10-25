#ifndef MULTIPLICATIVE_CONVOLUTION_HPP
#define MULTIPLICATIVE_CONVOLUTION_HPP

#include "../components/modular_power_sequence.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>

#include "../math/factorize.hpp"
#include "../components/convolution_dispatch.hpp"

template <class T>
std::vector<T> multiplicative_convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	assert(a.size() == b.size() && a.size() <= std::numeric_limits<std::uint32_t>::max() && is_prime(a.size()));
	std::uint32_t p = a.size();
	auto factors = factorize(p - 1);
	factors.erase(std::unique(factors.begin(), factors.end()), factors.end());
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

	auto product = &a == &b ? convolution(x, x) : convolution(x, y);
	for (std::size_t i = p - 1; i < product.size(); ++i) product[i - (p - 1)] += product[i];
	for (std::uint32_t i = 0; i < p - 1; ++i) result[powers[i]] = product[i];
	T sum_a(0), sum_b(0);
	for (std::size_t i = 1; i < a.size(); ++i) sum_a += a[i];
	for (const T &v : b) sum_b += v;
	result[0] = a[0] * sum_b;
	result[0] += sum_a * b[0];
	return result;
}

#endif
