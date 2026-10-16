#ifndef FRACTION_BASELINE_HPP
#define FRACTION_BASELINE_HPP

#include <cassert>
#include <compare>
#include <cstdint>
#include <limits>

class FractionBaseline {
	using W = __int128_t;
	using U = __uint128_t;
	std::int64_t num = 0, den = 1;
	FractionBaseline(W n, W d, bool) {
		if (d < 0) n = -n, d = -d;
		U a = n < 0 ? U(0) - U(n) : U(n), b = d;
		while (b) { U r = a % b; a = b, b = r; }
		n /= W(a), d /= W(a);
		assert(std::numeric_limits<std::int64_t>::min() <= n && n <= std::numeric_limits<std::int64_t>::max());
		assert(0 < d && d <= std::numeric_limits<std::int64_t>::max());
		num = n, den = d;
	}

  public:
	FractionBaseline(std::int64_t n = 0, std::int64_t d = 1) : FractionBaseline(n, d, true) {}
	std::int64_t numerator() const { return num; }
	std::int64_t denominator() const { return den; }
	friend FractionBaseline operator+(FractionBaseline a, FractionBaseline b) {
		return FractionBaseline(W(a.num) * b.den + W(b.num) * a.den, W(a.den) * b.den, true);
	}
	friend FractionBaseline operator-(FractionBaseline a, FractionBaseline b) {
		return FractionBaseline(W(a.num) * b.den - W(b.num) * a.den, W(a.den) * b.den, true);
	}
	friend FractionBaseline operator*(FractionBaseline a, FractionBaseline b) {
		return FractionBaseline(W(a.num) * b.num, W(a.den) * b.den, true);
	}
	friend FractionBaseline operator/(FractionBaseline a, FractionBaseline b) {
		return FractionBaseline(W(a.num) * b.den, W(a.den) * b.num, true);
	}
	bool operator==(const FractionBaseline &) const = default;
	std::strong_ordering operator<=>(FractionBaseline b) const { return W(num) * b.den <=> W(b.num) * den; }
};

#endif
