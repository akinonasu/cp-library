#ifndef BARRETT32_HPP
#define BARRETT32_HPP

#include <cassert>
#include <cstdint>

struct Barrett32 {
	std::uint32_t mod;
	std::uint64_t reciprocal;
	explicit Barrett32(std::uint32_t m) : mod(m), reciprocal(m > 1 ? std::uint64_t((__uint128_t(1) << 64) / m) : 0) {
		assert(m > 0);
	}
	std::uint32_t reduce(std::uint64_t x) const {
		if (mod == 1) return 0;
		std::uint64_t q = (__uint128_t(x) * reciprocal) >> 64, r = x - q * mod;
		return r < mod ? r : r - mod;
	}
	std::uint32_t mul(std::uint32_t a, std::uint32_t b) const { return reduce(std::uint64_t(a) * b); }
	std::uint32_t pow(std::uint32_t a, std::uint64_t e) const {
		std::uint32_t r = mod == 1 ? 0 : 1;
		for (; e; e >>= 1, a = mul(a, a)) if (e & 1) r = mul(r, a);
		return r;
	}
};

#endif
