#include "math/geometric_sum.hpp"
#include "test/fixtures/recurrence_types.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>

using W = boost::multiprecision::cpp_int;

static_assert(geometric_sum(3, 4) == 40);
static_assert(geometric_sum(2, 3, 4) == 80);
static_assert(geometric_sum(-2, 4) == -5);
static_assert(geometric_sum(0, 0) == 0 && geometric_sum(0, 4) == 1);
static_assert(geometric_sum(std::numeric_limits<std::int64_t>::min(), 1) == 1);
static_assert(geometric_sum(__uint128_t(1), std::numeric_limits<__uint128_t>::max()) == std::numeric_limits<__uint128_t>::max());

template <int Mod>
void modular() {
	using T = RecurrenceModint<Mod>;
	for (int a = -20; a <= 20; ++a) {
		T power = 1, sum = 0;
		for (int n = 0; n <= 100; ++n) {
			assert(geometric_sum(T(a), n) == sum);
			assert(geometric_sum(T(7), T(a), n) == T(7) * sum);
			sum += power;
			power *= T(a);
		}
	}
	std::mt19937_64 random(53711);
	for (int trial = 0; trial < 1000; ++trial) {
		T a = random() % Mod;
		std::uint64_t n = random() >> 1, m = random() >> 1;
		T power = 1, base = a;
		for (auto k = n; k; k >>= 1, base *= base) if (k & 1) power *= base;
		assert(geometric_sum(a, n + m) == geometric_sum(a, n) + power * geometric_sum(a, m));
	}
}

template <class T>
void integer(T ratio, int n) {
	W sum = 0, power = 1;
	for (int i = 0; i < n; ++i) sum += power, power *= ratio;
	bool overflow = false;
	T answer = 0;
	try { answer = geometric_sum(ratio, n); } catch (const std::overflow_error &) { overflow = true; }
	bool fits = sum >= W(std::numeric_limits<T>::min()) && sum <= W(std::numeric_limits<T>::max());
	assert(overflow == !fits);
	if (fits) assert(W(answer) == sum);
}

int main() {
	for (int ratio = -12; ratio <= 12; ++ratio) for (int n = 0; n <= 80; ++n) {
		integer(ratio, n);
		integer(std::int64_t(ratio), n);
		if (ratio >= 0) integer(std::uint64_t(ratio), n);
	}
	modular<998244353>();
	modular<1000000007>();
	modular<12>();
	modular<1>();
	assert(geometric_sum(W(3), 200) == (boost::multiprecision::pow(W(3), 200) - 1) / 2);
	assert(geometric_sum(std::int64_t(1), std::uint64_t(std::numeric_limits<std::int64_t>::max())) == std::numeric_limits<std::int64_t>::max());
	assert(geometric_sum(std::uint64_t(1), std::numeric_limits<std::uint64_t>::max()) == std::numeric_limits<std::uint64_t>::max());
	assert(geometric_sum(std::numeric_limits<std::int64_t>::max(), 0) == 0);
	assert(geometric_sum(std::numeric_limits<std::int64_t>::max(), 1) == 1);
	bool caught = false;
	try { (void)geometric_sum(1, -1); } catch (const std::invalid_argument &) { caught = true; }
	assert(caught);
	caught = false;
	try { (void)geometric_sum(std::int64_t(1), std::numeric_limits<std::uint64_t>::max()); } catch (const std::overflow_error &) { caught = true; }
	assert(caught);
	caught = false;
	try { (void)geometric_sum(std::numeric_limits<int>::max(), 2, 2); } catch (const std::overflow_error &) { caught = true; }
	assert(caught);
}
