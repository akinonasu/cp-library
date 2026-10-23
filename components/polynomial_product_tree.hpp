#ifndef CP_COMPONENTS_POLYNOMIAL_PRODUCT_TREE_HPP
#define CP_COMPONENTS_POLYNOMIAL_PRODUCT_TREE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

namespace polynomial_detail {
template<class F, std::size_t Limit = 1U << 23>
struct ProductTree {
	static_assert(Limit >= 2);
	using T = typename F::value_type;
	std::size_t size = 1, count;
	std::vector<F> product;
	static void trim(F& f) { while (!f.empty() && f.back() == T(0)) f.pop_back(); }
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

	explicit ProductTree(const std::vector<T>& xs) : count(xs.size()) {
		assert(count <= Limit);
		while (size < count) size <<= 1;
		product.resize(2 * size);
		for (std::size_t i = 0; i < size; ++i)
			product[size + i] = i < count ? F{-xs[i], T(1)} : F{T(1)};
		for (std::size_t i = size; --i;) {
			const auto& a = product[2 * i];
			const auto& b = product[2 * i + 1];
			product[i] = b.size() == 1 ? a : multiply(a, b);
		}
	}

	std::vector<T> evaluate(const F& f) const {
		std::vector<T> result(count);
		if (!count) return result;
		auto remainder = [](const F& a, const F& b) {
			return a.size() < b.size() ? a : a.divmod(b).second;
		};
		auto visit = [&](auto&& self, std::size_t node, const F& value) -> void {
			if (value.empty()) return;
			if (node >= size) {
				result[node - size] = value[0];
				return;
			}
			self(self, node * 2, remainder(value, product[node * 2]));
			if (product[node * 2 + 1].size() > 1)
				self(self, node * 2 + 1, remainder(value, product[node * 2 + 1]));
		};
		F value = f;
		trim(value);
		visit(visit, 1, remainder(value, product[1]));
		return result;
	}

	F interpolate(const std::vector<T>& ys) const {
		assert(ys.size() == count);
		if (!count) return {};
		F derivative = product[1].diff();
		trim(derivative);
		std::vector<T> denominator = evaluate(derivative), prefix(count + 1, T(1));
		for (std::size_t i = 0; i < count; ++i) {
			assert(denominator[i] != T(0));
			prefix[i + 1] = prefix[i] * denominator[i];
		}
		T inverse = prefix[count].inv();
		std::vector<F> value(2 * size);
		for (std::size_t i = count; i--;) {
			value[size + i] = F{ys[i] * inverse * prefix[i]};
			inverse *= denominator[i];
		}
		for (std::size_t i = size; --i;) {
			value[i] = F::multiply(value[2 * i], product[2 * i + 1])
				+ F::multiply(value[2 * i + 1], product[2 * i]);
			value[2 * i] = F();
			value[2 * i + 1] = F();
		}
		value[1].resize(count);
		return std::move(value[1]);
	}
};
}

#endif
