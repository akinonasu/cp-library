#include "math/number_theory/discrete_log.hpp"
#include <algorithm>
#include <limits>
#include <random>
#include <unordered_map>

std::optional<std::uint32_t> reference(std::uint64_t a, std::uint64_t b, std::uint32_t mod) {
	a %= mod, b %= mod;
	std::uint64_t k = 1 % mod, offset = 0;
	for (std::uint64_t g; (g = std::gcd(a, std::uint64_t(mod))) > 1;) {
		if (k == b) return offset;
		if (b % g) return std::nullopt;
		b /= g, mod /= g, k = k * (a / g) % mod, ++offset;
	}
	if (k == b) return offset;
	std::uint64_t step = std::sqrt(double(mod)) + 1, stride = 1, value = b;
	std::unordered_map<std::uint32_t, std::uint32_t> table;
	for (std::uint32_t j = 0; j < step; ++j) {
		table[value] = j;
		value = value * a % mod, stride = stride * a % mod;
	}
	for (std::uint64_t i = 1; i <= step; ++i) {
		k = k * stride % mod;
		auto it = table.find(k);
		if (it != table.end()) return offset + i * step - it->second;
	}
	return std::nullopt;
}

int main() {
	for (std::uint32_t mod = 1; mod <= 160; ++mod) for (std::uint32_t a = 0; a < mod; ++a) {
		std::vector<std::optional<std::uint32_t>> expected(mod);
		std::uint32_t x = 1 % mod, exponent = 0;
		while (!expected[x]) {
			expected[x] = exponent++;
			x = std::uint64_t(x) * a % mod;
		}
		for (std::uint32_t b = 0; b < mod; ++b) {
			assert(discrete_log(a, b, mod) == expected[b]);
			assert(discrete_log(a + std::uint64_t(mod) * 13, b + std::uint64_t(mod) * 7, mod) == expected[b]);
		}
	}
	std::mt19937_64 rng(4642312);
	for (int i = 0; i < 400; ++i) {
		std::uint32_t mod = i < 150 ? std::uint32_t(rng()) | 1 : i < 300 ? std::uint32_t(rng()) & ~1U : 4294967291U;
		if (!mod) mod = 1;
		std::uint64_t a = rng(), b = rng();
		if (i % 2) {
			b = 1 % mod;
			std::uint64_t x = a % mod;
			for (std::uint32_t e = rng() % mod; e; e >>= 1, x = x * x % mod) if (e & 1) b = b * x % mod;
		}
		assert(discrete_log(a, b, mod) == reference(a, b, mod));
	}
	for (std::uint32_t mod : {2147483648U, 4294967295U, 4294967291U}) {
		for (std::uint32_t a : {0U, 1U, mod - 1, mod / 2, mod / 3}) {
			std::uint32_t x = 1 % mod;
			for (std::uint32_t e = 0; e < 100; ++e, x = std::uint64_t(x) * a % mod) {
				auto answer = discrete_log(a, x, mod);
				assert(answer && *answer <= e);
				assert(Barrett32(mod).pow(a, *answer) == x);
			}
		}
	}
	assert(discrete_log(std::numeric_limits<std::uint64_t>::max(), std::numeric_limits<std::uint64_t>::max(), 1) == 0);
	assert(discrete_log(2, 0, 2147483648U) == 31);
	assert(discrete_log(2, 1073741824U, 2147483648U) == 30);
	assert(!discrete_log(2, 3, 2147483648U));
	assert(!discrete_log(5, 2147483647U, 2147483648U));
	assert(discrete_log(5, Barrett32(2147483648U).pow(5, 536870911U), 2147483648U) == 536870911U);
	assert(discrete_log(3, Barrett32(998244353U).pow(3, 998244351U), 998244353U) == 998244351U);
}
