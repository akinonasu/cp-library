#include "math/number_theory/multiplicative_prefix_sum.hpp"
#include "math/mint.hpp"
#include "test/benchmark/multiplicative_prefix_sum_types.hpp"
#include <array>
#include <cassert>
#include <random>
#include <utility>

struct Ring {
	long long value;
	explicit Ring(long long x) : value(x) {}
	Ring& operator+=(Ring b) { value += b.value; return *this; }
	Ring& operator-=(Ring b) { value -= b.value; return *this; }
	Ring& operator*=(Ring b) { value *= b.value; return *this; }
	friend Ring operator+(Ring a, Ring b) { return a += b; }
	friend Ring operator-(Ring a, Ring b) { return a -= b; }
	friend Ring operator*(Ring a, Ring b) { return a *= b; }
};

template<class T, class PrimePower>
void check(std::uint64_t n, std::array<int, 3> coefficients, PrimePower power) {
	MultiplicativePrefixSum<T> sieve(n);
	auto constant = sieve.prime_sum([](std::uint64_t x) { return T(x); });
	auto linear = sieve.prime_sum([](std::uint64_t x) { return T(x * (x + 1) / 2); });
	auto quadratic = sieve.prime_sum([](std::uint64_t x) { return T(x * (x + 1) * (2 * x + 1) / 6); });
	std::vector<T> prime = constant;
	for (std::size_t i = 0; i < sieve.size(); ++i)
		prime[i] = T(coefficients[0]) * constant[i] + T(coefficients[1]) * linear[i] + T(coefficients[2]) * quadratic[i];
	if (!coefficients[2]) assert(prime == sieve.prime_sum_linear(T(coefficients[0]), T(coefficients[1])));
	const auto saved = prime;
	const auto actual = sieve.prefix_sum(prime, power);
	std::vector<T> expected(n + 1, T(0));
	for (std::uint64_t i = 1; i <= n; ++i) {
		auto x = i;
		T value = T(1);
		for (std::uint64_t p = 2; p <= x / p; ++p) if (x % p == 0) {
			unsigned e = 0;
			std::uint64_t pe = 1;
			do { x /= p; ++e; pe *= p; } while (x % p == 0);
			value *= power(p, e, pe);
		}
		if (x > 1) value *= power(x, 1, x);
		expected[i] = expected[i - 1] + value;
	}
	for (auto x : sieve.quotients()) assert(sieve.get(actual, x) == expected[x]);
	assert(sieve.get(actual, 0) == T(0));
	assert(sieve.sum(prime, power) == expected[n]);
	assert(prime == saved);
	assert(sieve.sum(prime, power) == expected[n]);
}

template<class T>
void cases() {
	for (std::uint64_t n = 0; n <= 100; ++n) {
		check<T>(n, {-1, 1, 0}, [](std::uint64_t p, unsigned, std::uint64_t pe) { return T(pe - pe / p); });
		check<T>(n, {-1, 0, 0}, [](std::uint64_t, unsigned e, std::uint64_t) { return e == 1 ? T(-1) : T(0); });
		check<T>(n, {2, 0, 0}, [](std::uint64_t, unsigned e, std::uint64_t) { return T(e + 1); });
		check<T>(n, {0, 0, 0}, [](std::uint64_t, unsigned e, std::uint64_t) { return T(e % 2 == 0); });
	}
	std::mt19937_64 random(20261022);
	for (int trial = 0; trial < 100; ++trial) {
		const auto n = random() % 5001;
		std::array<int, 3> coefficients;
		for (auto& c : coefficients) c = int(random() % 7) - 3;
		auto power = [coefficients](std::uint64_t p, unsigned e, std::uint64_t pe) {
			std::uint64_t actual = 1;
			for (unsigned i = 0; i < e; ++i) actual *= p;
			assert(actual == pe);
			if (e == 1) return T(coefficients[0]) + T(coefficients[1]) * T(p) + T(coefficients[2]) * T(p * p);
			return T(static_cast<int>((p * 17 + e * 23) % 11) - 5);
		};
		check<T>(n, coefficients, power);
	}
	check<T>(3000, {1, 1, 0}, [](std::uint64_t p, unsigned e, std::uint64_t) {
		T sum = T(1), current = T(1);
		for (unsigned i = 0; i < e; ++i) { current *= T(p); sum += current; }
		return sum;
	});
	MultiplicativePrefixSum<T> character(10000);
	auto prefix = [](std::uint64_t x) { return T(x % 4 == 1 || x % 4 == 2); };
	auto prime = character.prime_sum(prefix);
	auto power = [](std::uint64_t p, unsigned e, std::uint64_t) { return T(p == 2 ? 0 : p % 4 == 1 || e % 2 == 0 ? 1 : -1); };
	auto all = character.prefix_sum(prime, power);
	for (auto x : character.quotients()) assert(character.get(all, x) == prefix(x));
	assert(character.sum(prime, power) == prefix(10000));
}

int main() {
	cases<std::int64_t>();
	cases<mint>();
	cases<MultiplicativeSumModint<469762049>>();
	cases<MultiplicativeSumModint<1000000007>>();
	cases<MultiplicativeSumModint<36>>();
	using Wide = __int128_t;
	const std::uint64_t n = 100000000;
	MultiplicativePrefixSum<Wide> sieve(n);
	auto count = sieve.prime_sum([](std::uint64_t x) { return Wide(x); });
	auto ones = [](std::uint64_t, unsigned, std::uint64_t) { return Wide(1); };
	assert(sieve.sum(count, ones) == Wide(n));
	auto identity = sieve.prime_sum([](std::uint64_t x) { return Wide(x) * (x + 1) / 2; });
	assert(sieve.sum(identity, [](std::uint64_t, unsigned, std::uint64_t pe) { return Wide(pe); }) == Wide(n) * (n + 1) / 2);
	Wide divisors = 0;
	std::uint64_t root = 0;
	for (std::uint64_t i = 1; i * i <= n; ++i) { divisors += 2 * (n / i); root = i; }
	divisors -= Wide(root) * root;
	for (auto& value : count) value *= 2;
	assert(sieve.sum(count, [](std::uint64_t, unsigned e, std::uint64_t) { return Wide(e + 1); }) == divisors);
	auto copy = sieve;
	auto moved = std::move(copy);
	assert(copy.max_n() == 0 && copy.size() == 0 && copy.sum({}, ones) == 0);
	copy = std::move(moved);
	assert(moved.max_n() == 0 && moved.size() == 0 && copy.quotients() == sieve.quotients());
	MultiplicativePrefixSum<Ring> custom(100);
	auto prime = custom.prime_sum_linear(Ring(-1), Ring(1));
	auto phi = [](std::uint64_t p, unsigned, std::uint64_t pe) { return Ring(pe - pe / p); };
	assert(custom.sum(prime, phi).value == 3044);
	assert(custom.get(custom.prefix_sum(prime, phi), 10).value == 32);
}
