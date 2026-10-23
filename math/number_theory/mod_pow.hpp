#ifndef MOD_POW_HPP
#define MOD_POW_HPP

#include "mod_inv.hpp"
#include "../../components/splitmix64.hpp"
#include <initializer_list>

class ModPow : public ModInv {
	mod_fast_detail::Reducer order_;
	std::uint32_t root_ = 1, shift_, mask_;
	std::vector<std::uint32_t> low_, high_, logarithm_;
	std::uint32_t power_(std::uint32_t e) const { return modulus_.mul(low_[e & mask_], high_[e >> shift_]); }
	void build_log_() {
		std::uint32_t p = mod(), limit = fractions_.limit(), order = p - 1;
		logarithm_.resize(inverse_.size());
		if (!center_) {
			std::uint32_t x = 1;
			for (std::uint32_t i = 0; i < order; ++i, x = modulus_.mul(x, root_)) logarithm_[x] = i;
			return;
		}
		std::vector<std::uint16_t> factor(limit + 1);
		for (std::uint32_t q = 2; std::uint64_t(q) * q <= limit; ++q) if (!factor[q]) {
			for (std::uint32_t i = q * q; i <= limit; i += q) if (!factor[i]) factor[i] = q;
		}
		std::uint32_t step = std::min(order, 4 * (mask_ + 1)), capacity = std::bit_ceil(2 * step);
		std::vector<std::uint64_t> baby(capacity);
		auto hash = [=](std::uint32_t x) {
			x ^= x >> 16, x *= 0x7feb352dU, x ^= x >> 15;
			return x & (capacity - 1);
		};
		std::uint32_t value = 1;
		for (std::uint32_t i = 0; i < step; ++i, value = modulus_.mul(value, root_)) {
			auto k = hash(value);
			while (baby[k]) k = (k + 1) & (capacity - 1);
			baby[k] = (std::uint64_t(value) << 32) | i;
		}
		std::uint32_t giant = power_(order - step);
		auto bsgs = [&](std::uint32_t x) {
			for (std::uint32_t i = 0; i < order; i += step, x = modulus_.mul(x, giant)) {
				auto k = hash(x);
				while (baby[k] && std::uint32_t(baby[k] >> 32) != x) k = (k + 1) & (capacity - 1);
				if (baby[k]) {
					std::uint32_t result = i + std::uint32_t(baby[k]);
					return result < order ? result : result - order;
				}
			}
			assert(false);
			return std::uint32_t(0);
		};
		SplitMix64 rng((std::uint64_t(p) << 32) | root_);
		for (std::uint32_t i = 2; i <= limit; ++i) {
			std::uint32_t result = 0;
			if (factor[i]) result = order_.reduce(std::uint64_t(logarithm_[center_ + factor[i]]) + logarithm_[center_ + i / factor[i]]);
			else if (std::uint64_t(i) * i > p) {
				result = order_.reduce(std::uint64_t(logarithm_[center_ + p % i]) + order / 2 + order - logarithm_[center_ + p / i]);
			} else {
				bool found = false;
				if (i >= 100) for (int attempt = 0; attempt < 64 && !found; ++attempt) {
					std::uint32_t k = order_.reduce(rng()), x = modulus_.mul(i, power_(k));
					auto [a, b] = fractions_.fraction(x);
					std::uint64_t sum = order - k + (a < 0 ? order / 2 : 0);
					auto accumulate = [&](std::uint32_t value, bool subtract) {
						while (value > 1) {
							std::uint32_t q = factor[value] ? factor[value] : value;
							if (q >= i) return false;
							sum += subtract ? order - logarithm_[center_ + q] : logarithm_[center_ + q];
							value /= q;
						}
						return true;
					};
					if (accumulate(a < 0 ? -a : a, false) && accumulate(b, true)) found = true, result = order_.reduce(sum);
				}
				if (!found) result = bsgs(i);
			}
			logarithm_[center_ + i] = result;
		}
		for (std::uint32_t i = 1; i <= limit; ++i)
			logarithm_[center_ - i] = order_.reduce(std::uint64_t(logarithm_[center_ + i]) + order / 2);
	}

  public:
	explicit ModPow(std::uint32_t prime = 998244353, std::uint32_t root = 0) : ModInv(prime), order_(prime - 1) {
		std::uint32_t remaining = prime - 1;
		std::vector<std::uint32_t> factors;
		for (std::uint32_t q = 2; std::uint64_t(q) * q <= remaining; ++q) if (remaining % q == 0) {
			factors.push_back(q);
			do { remaining /= q; } while (remaining % q == 0);
		}
		if (remaining > 1) factors.push_back(remaining);
		auto valid = [&](std::uint32_t g) {
			return 0 < g && g < prime && std::all_of(factors.begin(), factors.end(), [&](auto q) { return modulus_.pow(g, (prime - 1) / q) != 1; });
		};
		if (root) { assert(valid(root)); root_ = root; }
		else while (!valid(root_)) ++root_;
		shift_ = (std::bit_width(prime - 2) + 1) / 2;
		std::uint32_t width = std::uint32_t(1) << shift_;
		mask_ = width - 1;
		low_.resize(width), high_.resize((prime - 2) / width + 1);
		low_[0] = high_[0] = 1;
		for (std::uint32_t i = 1; i < width; ++i) low_[i] = modulus_.mul(low_[i - 1], root_);
		std::uint32_t stride = modulus_.mul(low_.back(), root_);
		for (std::uint32_t i = 1; i < high_.size(); ++i) high_[i] = modulus_.mul(high_[i - 1], stride);
		build_log_();
	}
	std::uint32_t primitive_root() const { return root_; }
	std::uint32_t log(std::uint32_t x) const {
		assert(0 < x && x < mod());
		if (x <= fractions_.limit()) return logarithm_[center_ + x];
		auto [a, b] = fractions_.fraction(x);
		std::uint32_t result = logarithm_[center_ + a] + (mod() - 1) - logarithm_[center_ + b];
		return result < mod() - 1 ? result : result - (mod() - 1);
	}
	std::uint32_t pow_root(std::uint64_t exponent) const { return power_(order_.reduce(exponent)); }
	std::uint32_t pow(std::uint32_t a, std::uint64_t exponent) const {
		assert(a < mod());
		if (!a) return exponent == 0 ? 1 : 0;
		return power_(order_.mul(log(a), order_.reduce(exponent)));
	}
};

#endif
