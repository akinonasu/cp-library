#ifndef CP_COMPONENTS_POLYNOMIAL_MULTIPLY_HPP
#define CP_COMPONENTS_POLYNOMIAL_MULTIPLY_HPP

#include <algorithm>
#include <cstddef>

namespace polynomial_detail {
template<class F, std::size_t Limit = 1U << 23>
struct Products {
	static_assert(Limit >= 2);
	static F multiply(const F& a, const F& b) {
		if (a.empty() || b.empty()) return {};
		const auto n = a.size() + b.size() - 1;
		return n <= Limit ? F::multiply(a, b) : multiply_low(a, b, n);
	}
	static F multiply_low(const F& a, const F& b, std::size_t n) {
		F result(n);
		constexpr std::size_t block = Limit / 2;
		for (std::size_t i = 0; i < std::min(a.size(), n); i += block)
			for (std::size_t j = 0; j < std::min(b.size(), n - i); j += block) {
				F x(a.begin() + i, a.begin() + std::min(i + block, a.size()));
				F y(b.begin() + j, b.begin() + std::min(j + block, b.size()));
				F p = F::multiply(x, y);
				for (std::size_t k = 0; k < std::min(p.size(), n - i - j); ++k)
					result[i + j + k] += p[k];
			}
		return result;
	}
};
}

#endif
