#ifndef PRIME_COUNT_HPP
#define PRIME_COUNT_HPP

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

inline std::uint64_t prime_count(std::uint64_t n) {
	if (n < 2) return 0;
	std::uint64_t root = std::sqrt(static_cast<long double>(n));
	while (__uint128_t(root) * root > n) --root;
	while (__uint128_t(root + 1) * (root + 1) <= n) ++root;
	std::size_t size = (root + 1) / 2;
	std::vector<std::uint32_t> small(size), rough(size);
	std::vector<std::uint64_t> large(size), quotient(size);
	std::vector<std::uint8_t> removed(root + 1);
	for (std::size_t i = 0; i < size; ++i) {
		small[i] = i;
		rough[i] = 2 * i + 1;
		quotient[i] = n / rough[i];
		large[i] = (quotient[i] - 1) / 2;
	}
	std::uint32_t count = 0;
	for (std::uint64_t p = 3; p * p <= root; p += 2) {
		if (removed[p]) continue;
		removed[p] = 1;
		for (std::uint64_t i = p * p; i <= root; i += 2 * p) removed[i] = 1;
		const std::uint64_t reciprocal = UINT64_MAX / p;
		auto divide = [p, reciprocal](std::uint64_t x) {
			std::uint64_t q = (__uint128_t(x) * reciprocal) >> 64;
			return q + (x - q * p >= p);
		};
		std::size_t next = 0, k = 0;
		for (; k < size && std::uint64_t(rough[k]) * p <= root; ++k) {
			const std::uint32_t r = rough[k];
			if (removed[r]) continue;
			large[next] = large[k] - large[small[r * p / 2] - count] + count;
			quotient[next] = quotient[k];
			rough[next++] = r;
		}
		for (; k < size; ++k) {
			const std::uint32_t r = rough[k];
			if (removed[r]) continue;
			large[next] = large[k] - small[(divide(quotient[k]) - 1) / 2] + count;
			quotient[next] = quotient[k];
			rough[next++] = r;
		}
		size = next;
		std::size_t i = (root - 1) / 2;
		for (std::uint64_t j = (root / p - 1) | 1; j >= p; j -= 2) {
			const std::uint32_t delta = small[j / 2] - count;
			const std::size_t end = j * p / 2;
			for (; i >= end; --i) small[i] -= delta;
		}
		++count;
	}
	__int128_t result = 1 + __int128_t(large[0]) + __int128_t(size) * (size - 1) / 2;
	result += (__int128_t(count) - 1) * (size - 1);
	for (std::size_t k = 1; k < size; ++k) result -= large[k];
	for (std::size_t k = 1; k < size; ++k) {
		const std::uint64_t p = rough[k], reciprocal = UINT64_MAX / p;
		auto divide = [p, reciprocal](std::uint64_t x) {
			std::uint64_t q = (__uint128_t(x) * reciprocal) >> 64;
			return q + (x - q * p >= p);
		};
		const std::size_t end = small[(divide(quotient[k]) - 1) / 2] - count;
		if (end <= k) break;
		for (std::size_t j = k + 1; j <= end; ++j)
			result += small[(divide(quotient[j]) - 1) / 2];
		result -= __int128_t(end - k) * (count + k - 1);
	}
	return static_cast<std::uint64_t>(result);
}

#endif
