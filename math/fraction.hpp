#ifndef FRACTION_HPP
#define FRACTION_HPP

#include <cassert>
#include <compare>
#include <concepts>
#include <cstdint>
#include <istream>
#include <limits>
#include <numeric>
#include <ostream>
#include <type_traits>

template <std::signed_integral Int = std::int64_t>
requires (sizeof(Int) <= sizeof(std::int64_t))
class Fraction {
	using U = std::make_unsigned_t<Int>;
	using W = std::conditional_t<(sizeof(Int) <= 4), std::int64_t, __int128_t>;
	Int num = 0, den = 1;
	static constexpr U magnitude(Int x) { return x < 0 ? U(0) - U(x) : U(x); }
	static constexpr U gcd(U a, U b) {
		if (a == 1 || b == 1) return 1;
		return std::gcd(a, b);
	}
	static constexpr Fraction reduced(W n, W d) {
		assert(std::numeric_limits<Int>::min() <= n && n <= std::numeric_limits<Int>::max());
		assert(0 < d && d <= std::numeric_limits<Int>::max());
		Fraction result;
		result.num = static_cast<Int>(n);
		result.den = static_cast<Int>(d);
		return result;
	}
	static constexpr Int divide(Int n, U d) {
		if (d > U(std::numeric_limits<Int>::max())) return -1;
		return n / static_cast<Int>(d);
	}
	constexpr bool assign(Int n, Int d) {
		if (!d) return false;
		if (!n) { num = 0, den = 1; return true; }
		U a = magnitude(n), b = magnitude(d), g = gcd(a, b);
		a /= g, b /= g;
		bool negative = (n < 0) != (d < 0);
		if (b > U(std::numeric_limits<Int>::max())
			|| a > U(std::numeric_limits<Int>::max()) + U(negative)) return false;
		num = negative ? Int(-static_cast<Int>(a - 1) - 1) : static_cast<Int>(a);
		den = static_cast<Int>(b);
		return true;
	}
	template <bool subtract>
	constexpr Fraction add(Fraction b) const {
		if (!b.num) return *this;
		if (!num) {
			if constexpr (subtract) return -b;
			else return b;
		}
		U g = gcd(U(den), U(b.den));
		Int left = den / static_cast<Int>(g), right = b.den / static_cast<Int>(g);
		W n;
		if constexpr (subtract) n = W(num) * right - W(b.num) * left;
		else n = W(num) * right + W(b.num) * left;
		if (!n) return {};
		U h = 1;
		if (g > 1) {
			U remainder;
			if (std::numeric_limits<Int>::min() <= n && n <= std::numeric_limits<Int>::max())
				remainder = magnitude(static_cast<Int>(n)) % g;
			else {
				W r = n % W(g);
				remainder = static_cast<U>(r < 0 ? -r : r);
			}
			h = gcd(remainder, g);
		}
		if (h > 1) n /= W(h);
		return reduced(n, W(left) * (b.den / static_cast<Int>(h)));
	}

  public:
	constexpr Fraction() = default;
	constexpr Fraction(Int n) : num(n) {}
	constexpr Fraction(Int n, Int d) {
		bool valid = assign(n, d);
		assert(valid);
		(void)valid;
	}
	constexpr Int numerator() const { return num; }
	constexpr Int denominator() const { return den; }
	constexpr Fraction operator+() const { return *this; }
	constexpr Fraction operator-() const { return reduced(-W(num), den); }
	constexpr bool operator==(const Fraction &) const = default;
	constexpr std::strong_ordering operator<=>(Fraction b) const { return W(num) * b.den <=> W(b.num) * den; }
	friend constexpr Fraction operator+(Fraction a, Fraction b) { return a.template add<false>(b); }
	friend constexpr Fraction operator-(Fraction a, Fraction b) { return a.template add<true>(b); }
	friend constexpr Fraction operator*(Fraction a, Fraction b) {
		if (!a.num || !b.num) return {};
		U g = gcd(magnitude(a.num), U(b.den)), h = gcd(magnitude(b.num), U(a.den));
		return reduced(W(divide(a.num, g)) * divide(b.num, h),
			W(a.den / static_cast<Int>(h)) * (b.den / static_cast<Int>(g)));
	}
	friend constexpr Fraction operator/(Fraction a, Fraction b) {
		assert(b.num != 0);
		if (!a.num) return {};
		U g = gcd(magnitude(a.num), magnitude(b.num)), h = gcd(U(a.den), U(b.den));
		W n = W(divide(a.num, g)) * (b.den / static_cast<Int>(h));
		W d = W(a.den / static_cast<Int>(h)) * divide(b.num, g);
		if (d < 0) n = -n, d = -d;
		return reduced(n, d);
	}
	constexpr Fraction &operator+=(Fraction b) { return *this = *this + b; }
	constexpr Fraction &operator-=(Fraction b) { return *this = *this - b; }
	constexpr Fraction &operator*=(Fraction b) { return *this = *this * b; }
	constexpr Fraction &operator/=(Fraction b) { return *this = *this / b; }
	constexpr Fraction &operator++() { return *this += 1; }
	constexpr Fraction &operator--() { return *this -= 1; }
	constexpr Fraction operator++(int) { Fraction old = *this; ++*this; return old; }
	constexpr Fraction operator--(int) { Fraction old = *this; --*this; return old; }
	constexpr explicit operator bool() const { return num != 0; }
	constexpr explicit operator Int() const { return num / den; }
	template <std::floating_point F>
	constexpr explicit operator F() const { return F(num) / F(den); }
	constexpr Int floor() const { return num / den - (num % den < 0); }
	constexpr Int ceil() const { return num / den + (num % den > 0); }
	constexpr Fraction inv() const {
		assert(num != 0);
		return num < 0 ? reduced(-W(den), -W(num)) : reduced(den, num);
	}
	constexpr Fraction pow(std::uint64_t k) const {
		Fraction result(1), base = *this;
		while (k) {
			if (k & 1) result *= base;
			k >>= 1;
			if (k) base *= base;
		}
		return result;
	}
	friend constexpr Fraction abs(Fraction x) { return x.num < 0 ? -x : x; }
	friend constexpr Int floor(Fraction x) { return x.floor(); }
	friend constexpr Int ceil(Fraction x) { return x.ceil(); }
	friend constexpr Fraction pow(Fraction x, std::uint64_t k) { return x.pow(k); }
	friend std::ostream &operator<<(std::ostream &os, Fraction x) {
		return os << std::int64_t(x.num) << '/' << std::int64_t(x.den);
	}
	friend std::istream &operator>>(std::istream &is, Fraction &x) {
		std::int64_t n, d = 1;
		if (!(is >> n)) return is;
		if (!is.eof() && is.peek() == '/') {
			is.get();
			if (!(is >> d)) return is;
		}
		Fraction value;
		if (!d || n < std::numeric_limits<Int>::min() || n > std::numeric_limits<Int>::max()
			|| d < std::numeric_limits<Int>::min() || d > std::numeric_limits<Int>::max()
			|| !value.assign(static_cast<Int>(n), static_cast<Int>(d))) {
			is.setstate(std::ios::failbit);
			return is;
		}
		x = value;
		return is;
	}
};

using fraction = Fraction<>;

#endif
