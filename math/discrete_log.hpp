#ifndef DISCRETE_LOG_HPP
#define DISCRETE_LOG_HPP

#include "../components/barrett32.hpp"
#include "factorize.hpp"
#include <bit>
#include <cmath>
#include <numeric>
#include <optional>
#include <vector>

namespace discrete_log_detail {
inline std::uint32_t inverse(std::uint32_t x, std::uint32_t mod) {
	std::int64_t u = 0, v = 1, r = mod, s = x;
	while (s) {
		std::int64_t q = r / s, t = r - q * s;
		r = s, s = t, t = u - q * v, u = v, v = t;
	}
	return std::uint32_t(u < 0 ? u + mod : u);
}

inline std::optional<std::uint32_t> search(std::uint32_t a, std::uint32_t b, const Barrett32 &m, std::uint32_t order) {
	std::uint32_t value = 1;
	for (std::uint32_t j = 0; j < 32; ++j, value = m.mul(value, a)) {
		if (value == b) return j;
		if (j && value == 1) return std::nullopt;
	}
	std::uint32_t step = std::sqrt(double(order));
	step += std::uint64_t(step) * step < order;
	std::uint32_t capacity = std::bit_ceil(2 * step), mask = capacity - 1;
	int shift = 64 - std::countr_zero(capacity);
	std::vector<std::uint64_t> table(capacity);
	auto hash = [&](std::uint32_t x) { return std::uint32_t((x * 11995408973635179863ULL) >> shift); };
	value = 1;
	for (std::uint32_t j = 0; j < step; ++j, value = m.mul(value, a)) {
		if (value == b) return j;
		if (j && value == 1) return std::nullopt;
		auto slot = hash(value);
		while (table[slot]) slot = (slot + 1) & mask;
		table[slot] = (std::uint64_t(value) << 32) | j;
	}
	std::uint32_t stride = inverse(value, m.mod), target = b;
	for (std::uint64_t i = step; i < order; i += step) {
		target = m.mul(target, stride);
		auto slot = hash(target);
		while (table[slot] && std::uint32_t(table[slot] >> 32) != target) slot = (slot + 1) & mask;
		if (table[slot]) {
			std::uint64_t answer = i + std::uint32_t(table[slot]);
			if (answer < order) return std::uint32_t(answer);
			break;
		}
	}
	return std::nullopt;
}
}

inline std::optional<std::uint32_t> discrete_log(std::uint64_t a, std::uint64_t b, std::uint32_t mod) {
	assert(mod > 0);
	a %= mod, b %= mod;
	std::uint32_t offset = 0, start = 1 % mod;
	for (;;) {
		if (b == start) return offset;
		std::uint32_t g = std::gcd(std::uint32_t(a), mod);
		if (g == 1) break;
		if (b % g) return std::nullopt;
		b /= g, mod /= g, ++offset;
		start = std::uint64_t(start) * (a / g) % mod;
	}
	if (std::gcd(std::uint32_t(b), mod) != 1) return std::nullopt;
	Barrett32 m(mod);
	a %= mod;
	b = m.mul(b, discrete_log_detail::inverse(start, mod));
	std::uint32_t value = 1;
	for (std::uint32_t j = 0; j < 32; ++j, value = m.mul(value, a)) {
		if (value == b) return offset + j;
		if (j && value == 1) return std::nullopt;
	}
	std::uint32_t order = mod;
	for (auto [p, e] : factorize_count(mod)) order -= order / p;
	auto factors = factorize_count(order);
	for (auto &[p, e] : factors) while (e && m.pow(a, order / p) == 1) order /= p, --e;
	if (m.pow(b, order) != 1) return std::nullopt;
	std::uint64_t answer = 0, period = 1;
	std::uint32_t inverse_a = m.pow(a, order - 1);
	for (auto [p, e] : factors) if (e) {
		std::uint32_t base = m.pow(a, order / p), target = b, residue = 0, power = 1, exponent = order;
		for (int i = 0; i < e; ++i) {
			exponent /= p;
			auto digit = discrete_log_detail::search(base, m.pow(target, exponent), m, p);
			if (!digit) return std::nullopt;
			std::uint32_t term = *digit * power;
			residue += term, power *= p;
			target = m.mul(target, m.pow(inverse_a, term));
		}
		std::uint32_t delta = (std::uint64_t(residue) + power - answer % power) % power;
		delta = std::uint64_t(delta) * discrete_log_detail::inverse(period % power, power) % power;
		answer += period * delta, period *= power;
	}
	return std::uint32_t(answer + offset);
}

#endif
