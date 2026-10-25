#include "math/generalized_floor_sum.hpp"
#include "math/mint.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>

using U = std::uint64_t;
using I = __int128_t;

template <int D, class T, class A, class B>
auto check(U n, U m, A a, B b) {
	auto actual = generalized_floor_sum_moments<D, T>(n, m, a, b);
	generalized_floor_sum_detail::Table<D, T> expected{};
	for (U i = 0; i < n; ++i) {
		I value = I(a) * i + I(b), y = value / I(m);
		if (value % I(m) < 0) --y;
		std::array<T, D + 1> xp{}, yp{};
		xp[0] = yp[0] = T(1);
		for (int j = 1; j <= D; ++j) xp[j] = xp[j - 1] * T(i), yp[j] = yp[j - 1] * T(y);
		for (int p = 0; p <= D; ++p)
			for (int q = 0; p + q <= D; ++q) expected[p][q] += xp[p] * yp[q];
	}
	assert(actual == expected);
	return expected;
}

I inversions(U n, U m, U a, U b) {
	auto x = generalized_floor_sum_moments<2>(n, m, a, b);
	auto y = generalized_floor_sum_moments<2>(n, m, a, 0);
	return 2 * x[1][1] - I(n - 1) * x[0][1] + y[1][1] - I(n) * y[0][1];
}

int main() {
	for (U n = 0; n <= 12; ++n)
		for (U m = 1; m <= 12; ++m)
			for (U a = 0; a <= 15; ++a)
				for (U b = 0; b <= 15; ++b) {
					auto x = check<2, I>(n, m, a, b);
					assert((generalized_floor_sum<0, 1>(n, m, a, b) == x[0][1]));
					assert((generalized_floor_sum<1, 1>(n, m, a, b) == x[1][1]));
					assert((generalized_floor_sum<0, 2>(n, m, a, b) == x[0][2]));
					assert((generalized_floor_sum<2, 0>(n, m, a, b) == x[2][0]));
					assert((generalized_floor_sum_detail::euclidean<2, I>(n, m, a, b) == x));
				}
	for (U n = 1; n <= 15; ++n)
		for (U m = 1; m <= 15; ++m)
			for (U a = 0; a < m; ++a)
				for (U b = 0; b < m; ++b) {
					I expected = 0;
					for (U j = 0; j < n; ++j)
						for (U i = 0; i < j; ++i) expected += (a * i + b) % m > (a * j + b) % m;
					assert(inversions(n, m, a, b) == expected);
				}
	std::mt19937_64 rng(142895234);
	for (U n = 0; n <= 8; ++n)
		for (U m = 1; m <= 8; ++m)
			for (int a = -8; a <= 8; ++a)
				for (int b = -8; b <= 8; ++b) {
					auto x = check<2, I>(n, m, a, b);
					assert((generalized_floor_sum<0, 1>(n, m, a, b) == x[0][1]));
					assert((generalized_floor_sum<1, 1>(n, m, a, b) == x[1][1]));
					assert((generalized_floor_sum<0, 2>(n, m, a, b) == x[0][2]));
					assert((generalized_floor_sum<2, 0>(n, m, a, b) == x[2][0]));
				}
	for (int t = 0; t < 200; ++t) {
		U n = rng() % 25, m = rng() % 100 + 1;
		std::int64_t a = std::int64_t(rng() % 401) - 200, b = std::int64_t(rng() % 401) - 200;
		auto integer = check<6, I>(n, m, a, b);
		auto modular = check<8, mint>(n, m, a, b);
		check<0, I>(n, m, a, b);
		check<1, I>(n, m, a, b);
		assert((generalized_floor_sum<3, 3>(n, m, a, b) == integer[3][3]));
		assert((generalized_floor_sum<4, 4, mint>(n, m, a, b) == modular[4][4]));
	}
	constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
	constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
	constexpr auto unsigned_maximum = std::numeric_limits<U>::max();
	for (U m : {U(1), U(2), U(97), U(1) << 63, unsigned_maximum}) {
		check<4, mint>(7, m, minimum, unsigned_maximum);
		check<4, mint>(7, m, unsigned_maximum, minimum);
		check<1, I>(7, m, minimum, unsigned_maximum);
		check<1, I>(7, m, unsigned_maximum, minimum);
		check<4, mint>(7, m, minimum, maximum);
		check<4, mint>(7, m, maximum, minimum);
		check<4, mint>(7, m, maximum, maximum);
	}
	auto singleton = generalized_floor_sum_moments<1, std::int64_t>(1, 1, minimum, minimum);
	assert(singleton[0][1] == minimum);
	assert((generalized_floor_sum<0, 1, std::int64_t>(1, 1, minimum, minimum) == minimum));
	assert((generalized_floor_sum<2, 0>(3, 1, minimum, minimum) == 5));
	auto zero = generalized_floor_sum_moments<20>(0, 1, minimum, minimum);
	assert((zero == generalized_floor_sum_detail::Table<20, I>{}));
	auto one = generalized_floor_sum_moments<20>(1, 1, minimum, 0);
	assert(one[0][0] == 1);
	one[0][0] = 0;
	assert((one == generalized_floor_sum_detail::Table<20, I>{}));
	for (int t = 0; t < 2000; ++t) {
		U n = rng() % 50, m = rng() % 100 + 1, a = rng() % 200, b = rng() % 200;
		check<6, I>(n, m, a, b);
		auto moments = check<8, mint>(n, m, a, b);
		assert((generalized_floor_sum<0, 8, mint>(n, m, a, b) == moments[0][8]));
		assert((generalized_floor_sum<1, 7, mint>(n, m, a, b) == moments[1][7]));
		assert((generalized_floor_sum<4, 4, mint>(n, m, a, b) == moments[4][4]));
		assert((generalized_floor_sum<8, 0, mint>(n, m, a, b) == moments[8][0]));
		check<2, mint>(n, m, a, b);
		check<1, mint>(n, m, a, b);
		check<0, I>(n, m, a, b);
		assert((generalized_floor_sum<3, 0>(n, m, a, b) == generalized_floor_sum_moments<3>(n, m, a, b)[3][0]));
		m = rng() | (U(1) << 63);
		a = rng();
		b = rng();
		check<4, mint>(n, m, a, b);
		check<2, mint>(n, m, a, b);
		check<1, mint>(n, m, a, b);
		m = rng() % 20 + 1;
		check<6, mint>(n, m, a, b);
		check<2, mint>(n, m, a, b);
		check<1, mint>(n, m, a, b);
	}
	for (U n : {U(0), U(1), U(1000000000), std::numeric_limits<U>::max()}) {
		auto x = generalized_floor_sum_moments<8, mint>(n, 1, 1, 0);
		auto negative = generalized_floor_sum_moments<8, mint>(n, 1, -1, 0);
		auto constant = generalized_floor_sum_moments<8, mint>(n, unsigned_maximum, 0, -1);
		for (int p = 0; p <= 8; ++p)
			for (int q = 0; p + q <= 8; ++q) {
				assert(x[p][q] == x[p + q][0]);
				assert(negative[p][q] == (q & 1 ? -x[p][q] : x[p][q]));
				assert(constant[p][q] == (q & 1 ? -x[p][0] : x[p][0]));
			}
		for (int d = 0; d < 2; ++d) {
			auto v = generalized_floor_sum_moments<2, mint>(n, 12200160415121876738ULL, 7540113804746346429ULL, d ? ~U(0) : U(0));
			auto w = generalized_floor_sum_detail::euclidean<2, mint>(n, 12200160415121876738ULL, 7540113804746346429ULL, d ? ~U(0) : U(0));
			assert(v == w);
		}
	}
	assert(inversions(1000000, 15, 3, 9) == 200000200000LL);
}
