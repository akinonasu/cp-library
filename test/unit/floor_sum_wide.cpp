#include "math/floor_sum/floor_sum.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

using U = std::uint64_t;
using I = __int128_t;
using W = __uint128_t;
using Big = boost::multiprecision::cpp_int;

Big reference(Big n, Big m, Big a, Big b) {
	Big qa = a / m, qb = b / m;
	if (a % m < 0) --qa;
	if (b % m < 0) --qb;
	Big result = n * (n - 1) / 2 * qa + n * qb;
	a -= qa * m;
	b -= qb * m;
	for (;;) {
		if (a >= m) {
			result += n * (n - 1) / 2 * (a / m);
			a %= m;
		}
		if (b >= m) {
			result += n * (b / m);
			b %= m;
		}
		Big z = a * n + b;
		if (z < m) return result;
		n = z / m;
		b = z % m;
		std::swap(a, m);
	}
}

template <class A, class B>
void check(U n, U m, A a, B b) {
	Big expected = reference(Big(n), Big(m), Big(a), Big(b));
	if (expected >= std::numeric_limits<std::int64_t>::min() && expected <= std::numeric_limits<std::int64_t>::max())
		assert(Big(floor_sum(n, m, a, b)) == expected);
	if (expected >= -(Big(1) << 127) && expected < (Big(1) << 127)) assert(Big(floor_sum<I>(n, m, a, b)) == expected);
	if (expected >= 0 && expected < (Big(1) << 128)) assert(Big(floor_sum<W>(n, m, a, b)) == expected);
}

int main() {
	constexpr U maximum = std::numeric_limits<U>::max();
	constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
	std::array<U, 11> values{0, 1, 2, (U(1) << 32) - 1, U(1) << 32, (U(1) << 32) + 1,
		(U(1) << 63) - 1, U(1) << 63, (U(1) << 63) + 1, maximum - 1, maximum};
	for (U n : values)
		for (U m : values) if (m)
			for (U a : {U(0), U(1), m / 2, m - 1, m, maximum})
				for (U b : {U(0), U(1), m - 1, maximum}) {
					check(n, m, a, b);
					check(n, m, a, minimum);
					check(n, m, minimum, b);
				}
	assert(floor_sum(maximum, 1, 1, -std::numeric_limits<std::int64_t>::max()) == 0);
	assert(floor_sum<I>(maximum, 1, 1, minimum) == -I(maximum));
	std::mt19937_64 random(635147);
	for (int t = 0; t < 5000; ++t) {
		U n = random(), m = random() | 1, a = random(), b = random();
		check(n, m, a, b);
		check(n, m, std::int64_t(a), b);
		check(n, m, a, std::int64_t(b));
		check(n, m, std::int64_t(a), std::int64_t(b));
		check(U(std::uint32_t(n)), U(std::uint32_t(m)) + 1, a, b);
	}
	std::vector<U> fibonacci{1, 1};
	while (W(fibonacci.back()) + fibonacci[fibonacci.size() - 2] <= maximum)
		fibonacci.push_back(fibonacci.back() + fibonacci[fibonacci.size() - 2]);
	for (std::size_t i = 2; i < fibonacci.size(); ++i)
		for (U n : {U(0), U(1), U(2), fibonacci[i] / 2, fibonacci[i], maximum})
			for (U b : {U(0), U(1), fibonacci[i] - 1, maximum}) check(n, fibonacci[i], fibonacci[i - 1], b);
}
