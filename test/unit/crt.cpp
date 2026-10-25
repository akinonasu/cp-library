#include "math/crt.hpp"
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

static_assert(crt(2, 6, 5, 9) == CRTSolution{14, 18});
static_assert(!crt(1, 2, 0, 4));
static_assert(crt(-1, 6, -1, 9) == CRTSolution{17, 18});

I normalize(W x, I m) {
	x %= m;
	if (x < 0) x += m;
	return x.convert_to<I>();
}

void check(I r0, I m0, I r1, I m1) {
	I g = std::gcd(m0, m1);
	bool exists = (W(r1) - r0) % g == 0;
	W period = W(m0) / g * m1;
	bool overflow = false;
	std::optional<CRTSolution> solution;
	try { solution = crt(r0, m0, r1, m1); } catch (const std::overflow_error &) { overflow = true; }
	assert(overflow == (exists && period > std::numeric_limits<I>::max()));
	if (overflow) return;
	assert(bool(solution) == exists);
	if (!solution) return;
	assert(solution->modulus == period);
	assert(0 <= solution->residue && solution->residue < solution->modulus);
	assert(normalize(solution->residue, m0) == normalize(r0, m0));
	assert(normalize(solution->residue, m1) == normalize(r1, m1));
	assert(crt(r1, m1, r0, m0) == solution);
}

int main() {
	for (I m0 = 1; m0 <= 12; ++m0) for (I m1 = 1; m1 <= 12; ++m1) for (I r0 = -15; r0 <= 15; ++r0) for (I r1 = -15; r1 <= 15; ++r1) {
		auto result = crt(r0, m0, r1, m1);
		I period = std::lcm(m0, m1), answer = -1;
		for (I x = 0; x < period; ++x) if ((x - r0) % m0 == 0 && (x - r1) % m1 == 0) { answer = x; break; }
		assert(bool(result) == (answer >= 0));
		if (result) assert(result->residue == answer && result->modulus == period);
	}
	assert(crt({}, {}) == CRTSolution({0, 1}));
	assert(crt({2, 5, 14}, {6, 9, 18}) == CRTSolution({14, 18}));
	assert(!crt({2, 5, 1}, {6, 9, 2}));
	const I low = std::numeric_limits<I>::min(), high = std::numeric_limits<I>::max();
	std::array<I, 7> residues{low, low + 1, -1, 0, 1, high - 1, high};
	std::array<I, 7> moduli{1, 2, 3, high / 2, high / 2 + 1, high - 1, high};
	for (I r0 : residues) for (I r1 : residues) for (I m0 : moduli) for (I m1 : moduli) check(r0, m0, r1, m1);
	std::mt19937_64 random(647751);
	for (int t = 0; t < 10000; ++t) {
		I m0 = I(random() & high), m1 = I(random() & high);
		if (!m0) m0 = 1;
		if (!m1) m1 = 1;
		check(std::bit_cast<I>(random()), m0, std::bit_cast<I>(random()), m1);
		I x = std::bit_cast<I>(random());
		check(x, m0, x, m1);
		check(std::bit_cast<I>(random()), m0, std::bit_cast<I>(random()), m0);
	}
	for (int t = 0; t < 2000; ++t) {
		std::vector<I> r(8), m(8);
		for (int i = 0; i < 8; ++i) r[i] = I(random() % 101) - 50, m[i] = I(random() % 12) + 1;
		auto solution = crt(r, m);
		I period = 1;
		for (I x : m) period = std::lcm(period, x);
		I answer = -1;
		for (I x = 0; x < period; ++x) {
			bool valid = true;
			for (int i = 0; i < 8; ++i) valid &= (x - r[i]) % m[i] == 0;
			if (valid) { answer = x; break; }
		}
		assert(bool(solution) == (answer >= 0));
		if (solution) assert(solution->residue == answer && solution->modulus == period);
	}
	for (int kind = 0; kind < 4; ++kind) {
		bool caught = false;
		try {
			if (kind == 0) (void)crt(0, 0, 0, 1);
			if (kind == 1) (void)crt(0, 1, 0, low);
			if (kind == 2) (void)crt({0}, {});
			if (kind == 3) (void)crt({0, 0}, {1, -1});
		} catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
}
