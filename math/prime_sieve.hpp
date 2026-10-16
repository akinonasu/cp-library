#ifndef PRIME_SIEVE_HPP
#define PRIME_SIEVE_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n <= 2) return result;
	result.push_back(2);
	std::vector<std::uint8_t> composite(n / 2);
	for (std::size_t p = 3; p < n; p += 2) {
		if (composite[p / 2]) continue;
		result.push_back(p);
		if (p > (n - 1) / p) continue;
		for (std::size_t j = p * p / 2; j < composite.size(); j += p) composite[j] = 1;
	}
	return result;
}

#endif
