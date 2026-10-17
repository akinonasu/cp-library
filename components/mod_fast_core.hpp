#ifndef MOD_FAST_CORE_HPP
#define MOD_FAST_CORE_HPP

#include <algorithm>
#include <cassert>
#include <bit>
#include <cstdint>
#include <utility>
#include <vector>

namespace mod_fast_detail {
struct Reducer {
	std::uint32_t mod;
	std::uint64_t reciprocal;
	explicit Reducer(std::uint32_t m) : mod(m), reciprocal(m > 1 ? std::uint64_t((__uint128_t(1) << 64) / m) : 0) {
		assert(1 <= m && m < (std::uint32_t(1) << 31));
	}
	std::uint32_t reduce(std::uint64_t x) const {
		if (mod == 1) return 0;
		std::uint64_t q = (__uint128_t(x) * reciprocal) >> 64;
		std::uint32_t r = x - q * mod;
		return r < mod ? r : r - mod;
	}
	std::uint32_t mul(std::uint32_t a, std::uint32_t b) const { return reduce(std::uint64_t(a) * b); }
	std::uint32_t pow(std::uint32_t a, std::uint64_t e) const {
		std::uint32_t r = mod == 1 ? 0 : 1;
		for (; e; e >>= 1, a = mul(a, a)) if (e & 1) r = mul(r, a);
		return r;
	}
};

class RationalTable {
	struct Fraction { std::uint16_t a, b; };
	std::vector<Fraction> fractions_;
	std::uint32_t p_, shift_ = 0, limit_;

  public:
	explicit RationalTable(std::uint32_t p) : p_(p), limit_(p - 1) {
		assert(2 <= p && p < (std::uint32_t(1) << 31));
		if (p <= 4096) return;
		std::uint32_t d = 1;
		while (std::uint64_t(d) * d * d < p) d *= 2;
		std::uint32_t width = std::bit_ceil((p + d * d - 1) / (d * d));
		shift_ = std::countr_zero(width);
		limit_ = (p + d - 1) / d + d * width / 2;
		assert(limit_ < p);
		fractions_.resize((p + width - 1) / width);
		std::uint32_t a = 0, b = 1, c = 1, e = d;
		for (std::uint32_t i = 0; i < fractions_.size(); ++i) {
			std::uint64_t x = std::min((2ULL * i + 1) * width, 2ULL * p - 1);
			while (2ULL * p * c < x * e) {
				std::uint32_t k = (d + b) / e, next_a = k * c - a, next_b = k * e - b;
				a = c, b = e, c = next_a, e = next_b;
			}
			if (x * b - 2ULL * p * a <= 2ULL * p * c - x * e)
				fractions_[i] = {std::uint16_t(a), std::uint16_t(b)};
			else fractions_[i] = {std::uint16_t(c), std::uint16_t(e)};
		}
	}
	bool direct() const { return fractions_.empty(); }
	std::uint32_t limit() const { return limit_; }
	std::pair<int, std::uint32_t> fraction(std::uint32_t x) const {
		assert(0 < x && x < p_);
		if (direct()) return {int(x), 1};
		auto [a, b] = fractions_[x >> shift_];
		std::int64_t t = std::int64_t(x) * b - std::int64_t(a) * p_;
		assert(t != 0 && -std::int64_t(limit_) <= t && t <= limit_);
		return {int(t), b};
	}
};
}

#endif
