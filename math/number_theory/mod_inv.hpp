#ifndef MOD_INV_HPP
#define MOD_INV_HPP

#include "../../components/mod_fast_core.hpp"
#include "is_prime.hpp"

class ModInv {
  protected:
	mod_fast_detail::Reducer modulus_;
	mod_fast_detail::RationalTable fractions_;
	std::vector<std::uint32_t> inverse_;
	int center_;

  public:
	explicit ModInv(std::uint32_t prime = 998244353) : modulus_(prime), fractions_(prime), center_(fractions_.direct() ? 0 : fractions_.limit()) {
		assert(is_prime(prime));
		std::uint32_t limit = fractions_.limit();
		inverse_.resize(center_ + limit + 1);
		inverse_[center_ + 1] = 1;
		for (std::uint32_t i = 2; i <= limit; ++i)
			inverse_[center_ + i] = prime - modulus_.mul(prime / i, inverse_[center_ + prime % i]);
		if (center_) for (std::uint32_t i = 1; i <= limit; ++i) inverse_[center_ - i] = prime - inverse_[center_ + i];
	}
	std::uint32_t mod() const { return modulus_.mod; }
	std::uint32_t inv(std::uint32_t x) const {
		assert(0 < x && x < mod());
		if (x <= fractions_.limit()) return inverse_[center_ + x];
		auto [a, b] = fractions_.fraction(x);
		return modulus_.mul(inverse_[center_ + a], b);
	}
};

#endif
