#include "math/bigint.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cassert>
#include <compare>
#include <cstdint>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using Oracle = boost::multiprecision::cpp_int;

Oracle parse_decimal(std::string_view text) {
	std::size_t first = !text.empty() && (text[0] == '+' || text[0] == '-');
	Oracle result = 0;
	for (std::size_t i = first; i < text.size(); ++i) result = result * 10 + (text[i] - '0');
	return !text.empty() && text[0] == '-' ? -result : result;
}

void equal(const BigInt &actual, const Oracle &expected) {
	assert(actual.to_string() == expected.str());
	assert(actual.sign() == (expected > 0 ? 1 : expected < 0 ? -1 : 0));
	assert(bool(actual) == (expected != 0));
}

void check(const Oracle &a, const Oracle &b, bool alias = false) {
	BigInt x(a.str()), y(b.str());
	equal(x, a);
	equal(y, b);
	assert((x == y) == (a == b));
	assert((x != y) == (a != b));
	assert((x < y) == (a < b));
	assert((x <= y) == (a <= b));
	assert((x > y) == (a > b));
	assert((x >= y) == (a >= b));
	assert((x <=> y) == (a < b ? std::strong_ordering::less : a > b ? std::strong_ordering::greater : std::strong_ordering::equal));
	equal(+x, a);
	equal(-x, -a);
	equal(abs(x), a < 0 ? -a : a);
	equal(x + y, a + b);
	equal(x - y, a - b);
	equal(x * y, a * b);
	equal(y + x, a + b);
	BigInt modified = x;
	equal(modified += y, a + b);
	modified = x;
	equal(modified -= y, a - b);
	modified = x;
	equal(modified *= y, a * b);
	if (b != 0) {
		auto [q, r] = divmod(x, y);
		equal(q, a / b);
		equal(r, a % b);
		equal(q * y + r, a);
		assert(!r || r.sign() == x.sign());
		assert(abs(r) < abs(y));
		equal(x / y, a / b);
		equal(x % y, a % b);
		modified = x;
		equal(modified /= y, a / b);
		modified = x;
		equal(modified %= y, a % b);
	}
	if (alias) {
		modified = x;
		equal(modified += modified, a + a);
		modified = x;
		equal(modified -= modified, 0);
		modified = x;
		equal(modified *= modified, a * a);
		if (a != 0) {
			modified = x;
			equal(modified /= modified, 1);
			modified = x;
			equal(modified %= modified, 0);
		}
	}
	equal(x, a);
	equal(y, b);
}

std::string digits(std::mt19937_64 &random, int length) {
	std::string result(length, '0');
	if (!length) return "0";
	result[0] = char('1' + random() % 9);
	for (int i = 1; i < length; ++i) result[i] = char('0' + random() % 10);
	return result;
}

Oracle magnitude_value(const bigint_detail::Magnitude &digits) {
	assert(digits.empty() || digits.back());
	Oracle result = 0;
	for (std::size_t i = digits.size(); i--;) {
		assert(digits[i] < bigint_detail::base);
		result *= bigint_detail::base;
		result += digits[i];
	}
	return result;
}

void check_magnitudes(const bigint_detail::Magnitude &a, const bigint_detail::Magnitude &b) {
	Oracle x = magnitude_value(a), y = magnitude_value(b);
	auto sum = a, original_b = b;
	bigint_detail::add(sum, b);
	assert(magnitude_value(sum) == x + y && b == original_b);
	auto difference = x >= y ? a : b;
	bigint_detail::subtract(difference, x >= y ? b : a);
	Oracle expected = x - y;
	if (expected < 0) expected = -expected;
	assert(magnitude_value(difference) == expected);
	auto alias = a;
	bigint_detail::add(alias, alias);
	assert(magnitude_value(alias) == x + x);
	bigint_detail::subtract(alias, alias);
	assert(alias.empty());
}

void check_carry_blocks() {
	using bigint_detail::Magnitude;
	using bigint_detail::Word;
	constexpr Word base = bigint_detail::base;
	std::mt19937_64 random(748901283);
	check_magnitudes({}, {});
	check_magnitudes({base - 1}, {});
	for (int n : {31, 32, 33, 63, 64, 65, 95, 96, 97, 127, 128, 129}) {
		Magnitude a(n, base - 1), b(n);
		a.back() = base - 2;
		b.back() = 1;
		check_magnitudes(a, b);
		check_magnitudes(Magnitude(n, base - 1), Magnitude(n, base - 1));
		for (int position : {0, 7, 8, 15, 16, 23, 24, 30, 31, 32, 33, 62, 63, 64, 65, 94, 95, 96, 97, 126, 127, 128}) {
			if (position >= n) continue;
			++b[position];
			check_magnitudes(a, b);
			--b[position];
			Magnitude minuend(n + 1, 2), subtrahend(n, 2);
			minuend.back() = 1;
			minuend[position] = 1;
			check_magnitudes(minuend, subtrahend);
		}
		for (int trial = 0; trial < 48; ++trial) {
			Magnitude x(n), y(n), u(n + 1), v(n);
			for (int i = 0; i < n; ++i) {
				unsigned state = trial < 3 ? unsigned(trial) : unsigned(random() % 12);
				x[i] = state ? base - 1 : 0;
				y[i] = state == 2 ? 1 : 0;
				u[i] = state == 2 ? 1 : state == 0 ? 3 : 2;
				v[i] = 2;
			}
			x.back() = base - 2;
			y.back() = 1;
			u.back() = 1;
			check_magnitudes(x, y);
			check_magnitudes(u, v);
		}
	}
}

int main() {
	check_carry_blocks();
	equal(BigInt(), 0);
	for (std::string_view text : {"0", "-0", "+0", "00000", "-00000", "+00001", "000123456789012345678901234567890", "-999999999999999999999999999999"})
		equal(BigInt(text), parse_decimal(text));
	for (const std::string &text : std::vector<std::string>{"", "+", "-", " 1", "1 ", "1x", "0x10", "--1", "+-1", "1_000", std::string("12\0" "3", 4)}) {
		bool caught = false;
		try { BigInt value(text); }
		catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
	for (std::int64_t value : {std::numeric_limits<std::int64_t>::min(), std::int64_t(-1000000001), std::int64_t(-1), std::int64_t(0), std::int64_t(1), std::numeric_limits<std::int64_t>::max()})
		equal(BigInt(value), Oracle(value));
	for (std::uint64_t value : {std::uint64_t(0), std::uint64_t(1000000000), std::numeric_limits<std::uint64_t>::max()})
		equal(BigInt(value), Oracle(value));
	check(parse_decimal("6999999993000000008"), parse_decimal("999999999000000001"));
	check(parse_decimal("6999999993000000007"), parse_decimal("999999999000000001"));
	__uint128_t unsigned_max = ~__uint128_t(0);
	__int128_t signed_max = __int128_t(unsigned_max >> 1), signed_min = -signed_max - 1;
	equal(BigInt(unsigned_max), parse_decimal("340282366920938463463374607431768211455"));
	equal(BigInt(signed_max), parse_decimal("170141183460469231731687303715884105727"));
	equal(BigInt(signed_min), parse_decimal("-170141183460469231731687303715884105728"));
	for (int a = -16; a <= 16; ++a) for (int b = -16; b <= 16; ++b) check(Oracle(a), Oracle(b), true);
	for (int sign : {-1, 1}) for (int limbs : {1, 2, 3, 7, 31, 64, 127, 128, 129}) {
		Oracle power = parse_decimal("1" + std::string(9 * limbs, '0'));
		for (int delta : {-1, 0, 1}) check(sign * (power + delta), Oracle(sign * 999999999), true);
	}
	for (int a : {-1, 0, 1}) {
		BigInt value(a);
		equal(value++, a);
		equal(value, a + 1);
		equal(++value, a + 2);
		equal(value--, a + 2);
		equal(--value, a);
		equal(value + 3, a + 3);
		equal(3 + value, a + 3);
		equal(3 - value, 3 - a);
		equal(3 * value, 3 * a);
	}
	for (int operation = 0; operation < 5; ++operation) {
		BigInt value(-17), original = value;
		bool caught = false;
		try {
			if (operation == 0) value /= 0;
			if (operation == 1) value %= 0;
			if (operation == 2) static_cast<void>(value / 0);
			if (operation == 3) static_cast<void>(value % 0);
			if (operation == 4) static_cast<void>(divmod(value, BigInt(0)));
		} catch (const std::domain_error &) { caught = true; }
		assert(caught && value == original);
	}
	{
		BigInt original("-123456789012345678901234567890"), copy(original);
		BigInt moved(std::move(copy));
		assert(moved == original && copy == 0);
		copy = original;
		BigInt target(99);
		target = std::move(copy);
		assert(target == original && copy == 0);
		BigInt *same = &target;
		target = std::move(*same);
		assert(target == original);
		target = *same;
		assert(target == original);
		std::ostringstream out;
		out << original << ' ' << BigInt(0) << ' ' << BigInt(999999999);
		assert(out.str() == "-123456789012345678901234567890 0 999999999");
		std::istringstream in("  -000123 +000456 0");
		BigInt a, b, c;
		in >> a >> b >> c;
		assert(!in.fail() && a == -123 && b == 456 && c == 0);
		for (std::string text : {"", "abc", "12x", "+", "--7"}) {
			BigInt value(91);
			std::istringstream invalid(text);
			invalid >> value;
			assert(invalid.fail() && value == 91);
		}
	}
	std::mt19937_64 random(238798532);
	for (int trial = 0; trial < 400; ++trial) {
		Oracle a = parse_decimal(digits(random, random() % 150));
		Oracle b = parse_decimal(digits(random, random() % 150));
		if (random() & 1) a = -a;
		if (random() & 1) b = -b;
		check(a, b, true);
	}
	for (int trial = 0; trial < 45; ++trial) {
		int a_length = 200 + random() % 2200, b_length = 100 + random() % 1300;
		Oracle a = parse_decimal(digits(random, a_length));
		Oracle b = parse_decimal(digits(random, b_length));
		if (trial & 1) a = -a;
		if (trial & 2) b = -b;
		check(a, b);
	}
	for (int limbs : {63, 64, 65, 127, 128, 129, 255, 256, 257}) {
		Oracle a = parse_decimal(digits(random, 9 * limbs));
		Oracle b = parse_decimal(digits(random, 9 * limbs - 1));
		check(a, b, true);
		check(a * b + 7, -b);
	}
	for (std::string leading : {"1", "499999999", "500000000", "999999999"}) {
		Oracle divisor = parse_decimal(leading + std::string(9 * 129, '0') + "1");
		Oracle quotient = parse_decimal(std::string(9 * 131, '9'));
		check(divisor * quotient, divisor);
		check(-divisor * quotient - divisor + 1, divisor);
	}
	for (int length : {4096, 10000}) {
		Oracle a = parse_decimal(digits(random, length));
		Oracle b = parse_decimal(digits(random, length / 2));
		check(a, -b, true);
		check(-a * b + 999999999, b);
		Oracle nines = parse_decimal(std::string(length, '9'));
		check(nines, parse_decimal("1" + std::string(length / 2, '0') + "1"));
	}
}
