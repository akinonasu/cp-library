#include "fps/fps.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <utility>
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

void check_power(int n, int first, std::mt19937 &random) {
	fps a(n), expected(n);
	for (int i = first; i < n; ++i) a[i] = random();
	a[first] = 7;
	expected[0] = 1;
	for (int k = 0; k < 5; ++k) {
		fps next(n);
		for (int i = 0; i < n; ++i) if (expected[i] != mint(0))
			for (int j = first; i + j < n; ++j) next[i + j] += expected[i] * a[j];
		expected = std::move(next);
	}
	const fps saved = a;
	assert(a.pow(5) == expected && a == saved);
}

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
void check_append(int n, std::mt19937 &random) {
	fps a(n);
	for (auto &x : a) x = random();
	a[0] = 1;
	const fps expected = a.pow(5);
	for (int prefix : {0, 2, n / 2}) {
		fps root(prefix, mint(3)), raised(prefix, mint(5));
		fps_detail::blocked_poly::native_sqrt(a.data(), a.size(), root, n, mint(1));
		fps_detail::blocked_poly::native_power(a.data(), a.size(), raised, n, 5, mint(1));
		assert(root.size() == static_cast<std::size_t>(prefix + n));
		assert(raised.size() == root.size());
		for (int i = 0; i < prefix; ++i) assert(root[i] == mint(3) && raised[i] == mint(5));
		assert(fps(root.begin() + prefix, root.end()) == a.sqrt());
		assert(fps(raised.begin() + prefix, raised.end()) == expected);
		for (int zeros : {0, 1, 17, n + 3}) {
			fps shifted(prefix, mint(5));
			fps_detail::blocked_poly::native_power(a.data(), a.size(), shifted, n, 5, mint(1), zeros);
			assert(shifted.size() == static_cast<std::size_t>(prefix + zeros + n));
			for (int i = 0; i < prefix; ++i) assert(shifted[i] == mint(5));
			for (int i = 0; i < zeros; ++i) assert(shifted[prefix + i] == mint(0));
			assert(fps(shifted.begin() + prefix + zeros, shifted.end()) == expected);
		}
	}
}
#endif
}

int main() {
	std::mt19937 random(573184);
	for (int target : {512, 513, 520, 521})
		for (int shift : {0, 257})
			for (int input : {33, target - shift, target - shift + 17})
				check(target, shift, input, random);
	check(4097, 0, 4097, random);
	check(4097, 1025, 65, random);
	for (int n : {257, 513, 1025})
		for (int first : {0, 1, 17, n / 32, n / 8, (n - 1) / 5}) check_power(n, first, random);
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	for (int n : {256, 257, 512, 513, 521}) check_append(n, random);
#endif
}
