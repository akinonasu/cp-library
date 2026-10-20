#ifndef PRIME_SIEVE_HPP
#define PRIME_SIEVE_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n <= 2) return result;
	if (n >= 1024) result.reserve(std::size_t(double(n) / (std::log(double(n)) - 1.1)));
	result.push_back(2);
	const std::size_t count = n / 2;
	std::size_t block_size = 32768;
	std::vector<std::uint8_t> prime(std::min(count, block_size), 1);
	auto append = [&](std::size_t base, std::size_t length) {
		std::size_t i = 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		for (; i + 8 <= length; i += 8) {
			std::uint64_t word;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			while (word) {
				result.push_back(2 * (base + i + (__builtin_ctzll(word) >> 3)) + 1);
				word &= word - 1;
			}
		}
#endif
		for (; i < length; ++i) if (prime[i]) result.push_back(2 * (base + i) + 1);
	};
	if (count <= block_size) {
		prime[0] = 0;
		for (std::size_t p = 3; p <= (n - 1) / p; p += 2) {
			if (!prime[p / 2]) continue;
			for (std::size_t j = p * p / 2; j < count; j += p) prime[j] = 0;
		}
		append(0, count);
		return result;
	}
	std::size_t limit = std::size_t(std::sqrt(static_cast<long double>(n - 1)));
	while (limit > (n - 1) / limit) --limit;
	while (limit + 1 <= (n - 1) / (limit + 1)) ++limit;
	constexpr std::size_t period = 1155;
	block_size = (std::max(block_size, limit) + period - 1) / period * period;
	prime.resize(block_size);
	constexpr auto pattern = [] {
		std::array<std::uint8_t, period> value{};
		for (std::size_t i = 0; i < period; ++i)
			value[i] = (2 * i + 1) % 3 && (2 * i + 1) % 5 && (2 * i + 1) % 7 && (2 * i + 1) % 11;
		return value;
	}();
	auto primes = prime_sieve(limit + 1);
	std::vector<std::size_t> next(primes.size());
	for (std::size_t i = 1; i < primes.size(); ++i) next[i] = primes[i] * primes[i] / 2;
	for (std::size_t base = 0; base < count;) {
		std::size_t length = std::min(block_size, count - base);
		for (std::size_t offset = 0; offset < length; offset += period)
			std::copy_n(pattern.begin(), std::min(period, length - offset), prime.begin() + offset);
		if (!base) {
			prime[0] = 0;
			prime[1] = prime[2] = prime[3] = prime[5] = 1;
		}
		for (std::size_t i = 5; i < primes.size(); ++i) {
			std::size_t j = next[i];
			for (; j < length; j += primes[i]) prime[j] = 0;
			next[i] = j - length;
		}
		append(base, length);
		base += length;
	}
	return result;
}

#endif
