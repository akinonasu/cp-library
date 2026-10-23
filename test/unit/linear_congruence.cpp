#include "math/number_theory/linear_congruence.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

using I = std::int64_t;
using W = boost::multiprecision::int256_t;

static_assert(linear_congruence(6, 4, 10) == LinearCongruenceSolution{4, 5, 2});
static_assert(!linear_congruence(6, 3, 10));
static_assert(linear_congruence(0, 0, 10) == LinearCongruenceSolution{0, 1, 10});
static_assert(linear_congruence(0, 1, 1) == LinearCongruenceSolution{0, 1, 1});
static_assert(linear_congruence(-3, -2, 7) == LinearCongruenceSolution{3, 7, 1});

I normalize(W x, I m) {
	x %= m;
	if (x < 0) x += m;
	return x.convert_to<I>();
}

void check(I a, I b, I m) {
	auto answer = linear_congruence(a, b, m);
	I g = std::gcd(normalize(a, m), m);
	assert(bool(answer) == (normalize(b, m) % g == 0));
	if (!answer) return;
	auto [residue, period, count] = *answer;
	assert(0 <= residue && residue < period);
	assert(period == m / g && count == g);
	assert(W(period) * count == m);
	for (I k : {-123, -1, 0, 1, 999}) {
		W x = W(residue) + W(period) * k;
		assert(normalize(W(a) * x - b, m) == 0);
	}
	if (period > 1) assert(normalize(W(a) * (W(residue) + 1) - b, m) != 0);
}

int main() {
	for (I m = 1; m <= 40; ++m) for (I a = -50; a <= 50; ++a) for (I b = -50; b <= 50; ++b) {
		std::vector<I> solutions;
		for (I x = 0; x < m; ++x) if ((__int128_t(a) * x - b) % m == 0) solutions.push_back(x);
		auto answer = linear_congruence(a, b, m);
		assert(bool(answer) == !solutions.empty());
		if (!answer) continue;
		assert(answer->residue == solutions.front());
		assert(answer->count == I(solutions.size()));
		assert(answer->period * answer->count == m);
		for (I i = 0; i < answer->count; ++i) assert(solutions[i] == answer->residue + answer->period * i);
	}
	const I low = std::numeric_limits<I>::min(), high = std::numeric_limits<I>::max();
	const std::array<I, 10> values{low, low + 1, -1000000007, -2, -1, 0, 1, 2, high - 1, high};
	for (I a : values) for (I b : values) for (I m : {I(1), I(2), I(3), high / 2, high - 1, high}) check(a, b, m);
	assert(linear_congruence(0, low, high) == std::nullopt);
	assert(linear_congruence(0, 0, high) == LinearCongruenceSolution({0, 1, high}));
	assert(linear_congruence(high - 1, high - 1, high) == LinearCongruenceSolution({1, high, 1}));
	std::mt19937_64 random(561421);
	for (int trial = 0; trial < 20000; ++trial) {
		I a = std::bit_cast<I>(random()), b = std::bit_cast<I>(random());
		I m = I(random() & std::uint64_t(high));
		if (!m) m = 1;
		check(a, b, m);
		I x = I(random() % m);
		check(a, normalize(W(a) * x, m), m);
	}
	I f0 = 1, f1 = 1;
	for (;;) {
		check(f0, high, f1);
		if (f0 > high - f1) break;
		I next = f0 + f1;
		f0 = f1;
		f1 = next;
	}
	for (I m : {low, I(-1), I(0)}) {
		bool caught = false;
		try { (void)linear_congruence(1, 1, m); } catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
}
