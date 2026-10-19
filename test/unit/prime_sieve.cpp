#include "math/number_theory/prime_sieve.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

std::vector<std::size_t> reference(std::size_t n) {
	std::vector<bool> composite(n);
	for (std::size_t i = 2; i <= (n - 1) / i; ++i)
		if (!composite[i])
			for (std::size_t j = i * i; j < n; j += i) composite[j] = true;
	std::vector<std::size_t> result;
	for (std::size_t i = 2; i < n; ++i) if (!composite[i]) result.push_back(i);
	return result;
}

int main(int argc, char **) {
	const bool extended = argc > 1;
	const std::size_t limit = 5000000;
	const auto expected = reference(limit);
	auto check = [&](std::size_t n) {
		auto actual = prime_sieve(n);
		auto end = std::lower_bound(expected.begin(), expected.end(), n);
		assert(actual.size() == std::size_t(end - expected.begin()));
		assert(std::equal(actual.begin(), actual.end(), expected.begin()));
	};
	for (std::size_t n = 0; n <= 8192; ++n) check(n);
	for (std::size_t block : {32768, 33495})
		for (std::size_t b = 1; b <= (extended ? 32 : 8); ++b)
			for (int delta = -17; delta <= 17; ++delta) check(2 * block * b + delta);
	for (std::size_t p : expected) {
		if (p * p + 2 >= limit) break;
		if (!extended && p > 257 && p != 509 && p != 521 && p != 1021 && p != 2039) continue;
		for (int delta = -2; delta <= 2; ++delta) check(p * p + delta);
	}
	std::mt19937_64 random(20261017);
	for (int i = 0; i < (extended ? 300 : 64); ++i) check(random() % limit);
	check(limit);
	if (extended) {
		const std::size_t n[] = {10000001, 100000001, 500000001};
		const std::size_t counts[] = {664579, 5761455, 26355867};
		for (int i = 0; i < 3; ++i) {
			auto actual = prime_sieve(n[i]);
			assert(actual.size() == counts[i]);
			assert(std::is_sorted(actual.begin(), actual.end()));
			assert(actual.back() < n[i]);
		}
	}
	std::cout << "prime_sieve: OK\n";
}
