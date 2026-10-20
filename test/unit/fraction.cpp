#include "math/fraction.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

using I = __int128_t;
using U = __uint128_t;
using F = Fraction<>;

struct Reference {
	I n, d;
	Reference(I n, I d) : n(n), d(d) {
		assert(d);
		if (d < 0) this->n = -n, this->d = -d;
		U a = this->n < 0 ? U(0) - U(this->n) : U(this->n), b = this->d;
		while (b) { U r = a % b; a = b, b = r; }
		this->n /= I(a), this->d /= I(a);
	}
};

template <class T>
bool fits(Reference r) {
	return std::numeric_limits<T>::min() <= r.n && r.n <= std::numeric_limits<T>::max()
		&& r.d <= std::numeric_limits<T>::max();
}

template <class T>
void equal(Fraction<T> actual, Reference expected) {
	assert(actual.numerator() == expected.n && actual.denominator() == expected.d);
	assert(actual.denominator() > 0);
}

template <class T>
void pair(Fraction<T> a, Fraction<T> b) {
	I an = a.numerator(), ad = a.denominator(), bn = b.numerator(), bd = b.denominator();
	assert((a == b) == (an == bn && ad == bd));
	assert((a < b) == (an * bd < bn * ad));
	assert((a <= b) == (an * bd <= bn * ad));
	assert((a > b) == (an * bd > bn * ad));
	assert((a >= b) == (an * bd >= bn * ad));
	Reference sum(an * bd + bn * ad, ad * bd);
	Reference difference(an * bd - bn * ad, ad * bd);
	Reference product(an * bn, ad * bd);
	if (fits<T>(sum)) { equal(a + b, sum); auto c = a; c += b; equal(c, sum); }
	if (fits<T>(difference)) { equal(a - b, difference); auto c = a; c -= b; equal(c, difference); }
	if (fits<T>(product)) { equal(a * b, product); auto c = a; c *= b; equal(c, product); }
	if (bn) {
		Reference quotient(an * bd, ad * bn);
		if (fits<T>(quotient)) { equal(a / b, quotient); auto c = a; c /= b; equal(c, quotient); }
	}
}

static_assert(F(12, -18) == F(-2, 3));
static_assert(F(1, 6) + F(1, 3) == F(1, 2));
static_assert(F(-2, 3) * F(-9, 4) == F(3, 2));
static_assert(F(3, 2) / F(-9, 4) == F(-2, 3));
static_assert(2 + F(1, 3) == F(7, 3) && 2 - F(1, 3) == F(5, 3));
static_assert(2 * F(1, 3) == F(2, 3) && 2 / F(1, 3) == 6);
static_assert(F(-4, 3).floor() == -2 && F(-4, 3).ceil() == -1);
static_assert(floor(F(4, 3)) == 1 && ceil(F(4, 3)) == 2);
static_assert(F(2, 3).inv() == F(3, 2) && pow(F(-2, 3), 3) == F(-8, 27));
static_assert(F(-2, 3).inv() == F(-3, 2));
static_assert(1 < F(3, 2) && F(-1, 2) < 0 && 1 == F(5, 5));
static_assert(std::is_trivially_copyable_v<F> && sizeof(F) == 16);
static_assert(Fraction<int>(std::numeric_limits<int>::max(), 2) * 2 == std::numeric_limits<int>::max());
static_assert(Fraction<int>(std::numeric_limits<int>::max(), 2) + Fraction<int>(std::numeric_limits<int>::max(), 2)
	== std::numeric_limits<int>::max());

int main() {
	for (int n = -12; n <= 12; ++n)
		for (int d = -12; d <= 12; ++d) if (d) {
			equal(F(n, d), Reference(n, d));
			equal(Fraction<int>(n, d), Reference(n, d));
		}
	std::vector<Fraction<std::int8_t>> small;
	for (int n = -8; n <= 8; ++n)
		for (int d = 1; d <= 8; ++d) small.emplace_back(n, d);
	for (auto a : small) for (auto b : small) pair(a, b);
	std::mt19937_64 random(713);
	for (int i = 0; i < 20000; ++i) {
		int an = std::bit_cast<std::int32_t>(std::uint32_t(random()));
		int bn = std::bit_cast<std::int32_t>(std::uint32_t(random()));
		int ad = int(random() % std::numeric_limits<int>::max()) + 1;
		int bd = int(random() % std::numeric_limits<int>::max()) + 1;
		pair(Fraction<int>(an, ad), Fraction<int>(bn, bd));
	}
	for (int i = 0; i < 40000; ++i) {
		auto integer = [&] { return std::int64_t(random() % 2000000001) - 1000000000; };
		F a(integer(), random() % 1000000000 + 1), b(integer(), random() % 1000000000 + 1);
		pair(a, b);
		assert(a.floor() <= static_cast<long double>(a) && static_cast<long double>(a) <= a.ceil());
		auto whole = a; ++whole; assert(whole - a == 1); whole--; assert(whole == a);
		assert(a + 3 == 3 + a && a * 3 == 3 * a);
		if (a) assert(a * a.inv() == 1 && a / a == 1);
	}
	constexpr auto low = std::numeric_limits<std::int64_t>::min();
	constexpr auto high = std::numeric_limits<std::int64_t>::max();
	static_assert(F(low, low) == 1 && F(0, low) == 0);
	static_assert(F(low, 2).numerator() == low / 2 && F(low, 2).denominator() == 1);
	static_assert(F(2, low).numerator() == -1 && F(2, low).denominator() == -(low / 2));
	static_assert(F(high, 2) + F(high, 2) == high);
	static_assert(F(low, 3) / F(low, 3) == 1);
	static_assert(F(high).pow(1) == high && F(low).pow(1) == low);
	std::array<std::int64_t, 11> values{low, low + 1, -3037000500LL, -2, -1, 0, 1, 2, 3037000500LL, high - 1, high};
	std::vector<F> boundary;
	for (auto n : values) for (auto d : values) if (d && fits<std::int64_t>(Reference(n, d))) {
		equal(F(n, d), Reference(n, d));
		boundary.emplace_back(n, d);
	}
	for (auto a : boundary) for (auto b : boundary) pair(a, b);
	for (auto a : boundary) if (a && fits<std::int64_t>(Reference(a.denominator(), a.numerator())))
		equal(a.inv(), Reference(a.denominator(), a.numerator()));
	for (int i = 0; i < 30000; ++i) {
		auto integer = [&] { return std::bit_cast<std::int64_t>(random()); };
		std::int64_t n = integer(), d = std::int64_t(random() % high) + 1;
		F a(n, d), b(integer(), std::int64_t(random() % high) + 1);
		equal(a, Reference(n, d));
		pair(a, b);
		pair(a, a);
		if (n != low) pair(a, -a);
		if (n && n != low) pair(a, a.inv());
	}
	F self(3, 7); self += self; assert(self == F(6, 7));
	self -= self; assert(!self && self.denominator() == 1);
	self = F(-3, 7); self *= self; assert(self == F(9, 49));
	self /= self; assert(self == 1);
	assert(abs(F(-3, 7)) == F(3, 7) && +F(2, 3) == F(2, 3));
	assert(static_cast<int>(Fraction<int>(-7, 3)) == -2);
	assert(static_cast<double>(F(1, 2)) == 0.5 && static_cast<float>(F(1, 2)) == 0.5f);
	std::sort(boundary.begin(), boundary.end());
	for (std::size_t i = 1; i < boundary.size(); ++i) assert(!(boundary[i] < boundary[i - 1]));
	std::istringstream input("2/-6 7 -8/4 0/-3");
	F x; input >> x; assert(input && x == F(-1, 3));
	input >> x; assert(input && x == 7);
	input >> x; assert(input && x == -2);
	input >> x; assert(input && x == 0);
	std::ostringstream output; output << F(-2, 6) << ' ' << Fraction<std::int8_t>(2, 3);
	assert(output.str() == "-1/3 2/3");
	for (std::string text : {"", "abc", "1/", "1/0", "1/-9223372036854775808", "-9223372036854775808/-1"}) {
		std::istringstream bad(text); F old(5, 7); bad >> old; assert(bad.fail() && old == F(5, 7));
	}
	std::istringstream last("123"); last >> x; assert(last && last.eof() && x == 123);
	std::istringstream minimum("-9223372036854775808/-9223372036854775808");
	minimum >> x; assert(minimum && x == 1);
	Fraction<std::int8_t> tiny(3, 7);
	std::istringstream out_of_range("128/2"); out_of_range >> tiny;
	assert(out_of_range.fail() && tiny == Fraction<std::int8_t>(3, 7));
}
