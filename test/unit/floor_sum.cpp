#include "math/floor_sum/floor_sum.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

using U = std::uint64_t;
using I = __int128_t;

static_assert(std::is_same_v<decltype(floor_sum(1, 1, 0, 0)), std::int64_t>);
static_assert(floor_sum(4, 10, 6, 3) == 3);
static_assert(floor_sum(6, 6, -2, 9) == 1);
static_assert(floor_sum(0, 1, -1, -1) == 0);
static_assert(floor_sum(1, 2, 0, -1) == -1);
static_assert(floor_sum<__int128_t>(10000000000ULL, 1, 1, 0) == I(10000000000ULL) * 9999999999ULL / 2);

template <class A, class B>
void check(U n, U m, A a, B b) {
	I expected = 0;
	for (U i = 0; i < n; ++i) {
		I value = I(a) * I(i) + I(b), q = value / I(m);
		expected += q - (value % I(m) < 0);
	}
	assert(floor_sum<I>(n, m, a, b) == expected);
	if (expected >= std::numeric_limits<std::int64_t>::min() && expected <= std::numeric_limits<std::int64_t>::max())
		assert(floor_sum(n, m, a, b) == expected);
	if (expected >= 0 && expected <= std::numeric_limits<U>::max()) assert(floor_sum<U>(n, m, a, b) == expected);
}

int main() {
	for (U m = 1; m <= 18; ++m)
		for (int a = -20; a <= 20; ++a)
			for (int b = -20; b <= 20; ++b) {
				I expected = 0;
				for (U n = 0; n <= 20; ++n) {
					assert(floor_sum(n, m, a, b) == expected);
					assert(floor_sum<I>(n, m, a, b) == expected);
					I value = I(a) * n + b;
					expected += value / I(m) - (value % I(m) < 0);
				}
			}
	std::mt19937_64 random(904317);
	for (int t = 0; t < 20000; ++t) {
		U n = random() % 65, m = random() | 1, a = random(), b = random();
		check(n, m, a, b);
		check(n, m, std::int64_t(a), b);
		check(n, m, a, std::int64_t(b));
		check(n, m, std::int64_t(a), std::int64_t(b));
		check(n, random() % 1000 + 1, std::int64_t(a), std::int64_t(b));
	}
	constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
	constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
	constexpr auto umax = std::numeric_limits<U>::max();
	for (U m : std::array<U, 10>{1, 2, 3, 97, (U(1) << 32) - 1, U(1) << 32, (U(1) << 32) + 1, U(1) << 63, umax - 1, umax})
		for (U n : {U(0), U(1), U(2), U(3), U(17)})
			for (U a : std::array<U, 6>{0, 1, m / 2, m - 1, m, umax})
				for (U b : std::array<U, 6>{0, 1, m / 2, m - 1, m, umax}) {
					check(n, m, a, b);
					check(n, m, a, minimum);
					check(n, m, minimum, b);
					check(n, m, maximum, minimum);
				}
}
