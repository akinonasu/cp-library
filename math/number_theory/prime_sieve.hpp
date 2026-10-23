#ifndef PRIME_SIEVE_HPP
#define PRIME_SIEVE_HPP

#include <algorithm>
#include <array>
#include <bit>
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
	if (n <= 65537) {
		const std::size_t size = n / 2;
		std::vector<std::uint8_t> prime(size, 1);
		prime[0] = 0;
		for (std::size_t p = 3; p <= (n - 1) / p; p += 2) {
			if (!prime[p / 2]) continue;
			for (std::size_t j = p * p / 2; j < size; j += p) prime[j] = 0;
		}
		std::size_t i = 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		for (; i + 8 <= size; i += 8) {
			std::uint64_t word;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			while (word) {
				result.push_back(2 * (i + (std::countr_zero(word) >> 3)) + 1);
				word &= word - 1;
			}
		}
#endif
		for (; i < size; ++i) if (prime[i]) result.push_back(2 * i + 1);
		return result;
	}
	result.push_back(3), result.push_back(5);
	constexpr std::array<unsigned, 8> residues{1, 7, 11, 13, 17, 19, 23, 29};
	constexpr auto index = [&] {
		std::array<unsigned, 30> index{};
		for (unsigned i = 0; i < 8; ++i) index[residues[i]] = i;
		return index;
	}();
	constexpr std::size_t period = 7 * 11 * 13, block_size = period * 32;
	static const auto pattern = [&] {
		std::array<std::uint8_t, period> pattern{};
		for (std::size_t i = 0; i < period; ++i)
			for (unsigned d = 0; d < 8; ++d) {
				const auto x = 30 * i + residues[d];
				pattern[i] |= std::uint8_t(bool(x % 7 && x % 11 && x % 13) << d);
			}
		return pattern;
	}();
	std::size_t root = std::sqrt(static_cast<long double>(n - 1));
	while (root > (n - 1) / root) --root;
	while (root + 1 <= (n - 1) / (root + 1)) ++root;
	const auto primes = prime_sieve(root + 1);
	struct State {
		std::size_t p;
		std::array<std::size_t, 8> next;
		std::array<std::uint8_t, 8> mask;
	};
	std::vector<State> state;
	state.reserve(primes.size());
	for (const auto p : primes) {
		if (p < 17) continue;
		State current;
		current.p = p;
		for (unsigned d = 0; d < 8; ++d) {
			const std::size_t k = p + (residues[d] + 30 - p % 30) % 30;
			const __uint128_t x = __uint128_t(p) * k;
			current.next[d] = x / 30;
			current.mask[d] = std::uint8_t(~(1u << index[x % 30]));
		}
		state.push_back(current);
	}
	const std::size_t size = (n - 1) / 30 + 1;
	std::vector<std::uint8_t> prime(block_size);
	constexpr auto offsets = [&] {
		std::array<unsigned, 64> offsets{};
		for (unsigned i = 0; i < 64; ++i) offsets[i] = 30 * (i / 8) + residues[i % 8];
		return offsets;
	}();
	for (std::size_t base = 0; base < size;) {
		const std::size_t length = std::min(block_size, size - base);
		for (std::size_t i = 0; i < length; i += period)
			std::copy_n(pattern.data(), std::min(period, length - i), prime.data() + i);
		if (base == 0) {
			prime[0] &= ~1u;
			prime[0] |= (1u << 1) | (1u << 2) | (1u << 3);
		}
		for (auto &current : state) {
			const auto p = current.p;
			for (unsigned d = 0; d < 8; ++d) {
				std::size_t j = current.next[d];
				const auto mask = current.mask[d];
				for (; j + 3 * p < length; j += 4 * p) {
					prime[j] &= mask;
					prime[j + p] &= mask;
					prime[j + 2 * p] &= mask;
					prime[j + 3 * p] &= mask;
				}
				for (; j < length; j += p) prime[j] &= mask;
				current.next[d] = j - length;
			}
		}
		if (base + length == size) {
			const auto remaining = n - 30 * (size - 1);
			for (unsigned d = 0; d < 8; ++d) if (residues[d] >= remaining) prime[length - 1] &= ~(1u << d);
		}
		std::fill(prime.begin() + length, prime.begin() + ((length + 7) / 8 * 8), 0);
		for (std::size_t i = 0; i < length; i += 8) {
			std::uint64_t word = 0;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			if constexpr (std::endian::native == std::endian::big) word = std::byteswap(word);
			else if constexpr (std::endian::native != std::endian::little) {
				word = 0;
				for (unsigned d = 0; d < 8; ++d) word |= std::uint64_t(prime[i + d]) << (8 * d);
			}
			while (word) {
				const auto x = 30 * (base + i) + offsets[std::countr_zero(word)];
				result.push_back(x);
				word &= word - 1;
			}
		}
		base += length;
	}
	return result;
}

#endif
