#include "fps/fps.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

namespace {
using U = std::uint64_t;
constexpr U mod = 998244353;

U power(U a, U exponent) {
	U result = 1;
	for (; exponent; exponent >>= 1, a = a * a % mod)
		if (exponent & 1) result = result * a % mod;
	return result;
}

void check(int target, int shift, int input, std::mt19937 &random) {
	const int first = 2 * shift, degree = target + shift;
	fps a(first + input);
	for (int i = first; i < static_cast<int>(a.size()); ++i) a[i] = random();
	a[first] = 49;
	for (int i = 0; i < static_cast<int>(a.size()); ++i) {
		if (i % 3 == 0) a[i].v = a[i].v % mod + mod;
		else if (i % 3 == 1) a[i].v %= mod;
	}
	const fps saved = a;
	std::vector<U> expected(target);
	expected[0] = 7;
	const U factor = power(14, mod - 2);
	const int last = std::min<int>(degree, a.size());
	for (int i = 1; i < target; ++i) {
		U sum = 0;
		for (int j = 1; j < i; ++j) sum += expected[j] * expected[i - j] % mod;
		const U coefficient = first + i < last ? a[first + i].val() : 0;
		expected[i] = (coefficient + mod - sum % mod) * factor % mod;
	}
	const fps result = a.sqrt(degree);
	assert(result.size() == static_cast<std::size_t>(degree));
	for (int i = 0; i < shift; ++i) assert(result[i] == mint(0));
	const bool negative = result[shift].val() != 7;
	assert(result[shift].val() == (negative ? mod - 7 : 7));
	for (int i = 0; i < target; ++i) {
		const U value = negative && expected[i] ? mod - expected[i] : expected[i];
		assert(result[shift + i].val() == value);
	}
	for (std::size_t i = 0; i < a.size(); ++i) assert(a[i].v == saved[i].v);
}
}

int main() {
	std::mt19937 random(573184);
	for (int target : {512, 513, 520, 521})
		for (int shift : {0, 257})
			for (int input : {33, target - shift, target - shift + 17})
				check(target, shift, input, random);
	check(4097, 0, 4097, random);
	check(4097, 1025, 65, random);
}
