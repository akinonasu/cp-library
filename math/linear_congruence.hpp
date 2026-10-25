#ifndef LINEAR_CONGRUENCE_HPP
#define LINEAR_CONGRUENCE_HPP

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>

struct LinearCongruenceSolution {
	std::int64_t residue, period, count;
	bool operator==(const LinearCongruenceSolution &) const = default;
};

constexpr std::optional<LinearCongruenceSolution> linear_congruence(std::int64_t a, std::int64_t b, std::int64_t m) {
	if (m <= 0) throw std::invalid_argument("linear_congruence modulus");
	a %= m;
	b %= m;
	if (a < 0) a += m;
	if (b < 0) b += m;
	std::int64_t r = m, s = a, u = 0, v = 1;
	while (s) {
		std::int64_t q = r / s;
		r -= q * s;
		u -= q * v;
		std::swap(r, s);
		std::swap(u, v);
	}
	if (b % r) return std::nullopt;
	std::int64_t period = m / r;
	if (u < 0) u += period;
	std::int64_t residue = __int128_t(u) * (b / r) % period;
	return LinearCongruenceSolution{residue, period, r};
}

#endif
