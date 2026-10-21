#ifndef FPS_LIGHT_PRODUCT_TREE_HPP
#define FPS_LIGHT_PRODUCT_TREE_HPP

#include "../fps/fps_light.hpp"
#include <cstddef>
#include <utility>
#include <vector>

namespace fps_light_detail {
template<class T> struct ProductTree {
	using F = LightFormalPowerSeries<T>;
	std::size_t size = 1, count;
	std::vector<F> product;
	explicit ProductTree(const std::vector<T> &xs) : count(xs.size()) {
		assert(count < (std::size_t(1) << 23));
		while (size < count) size <<= 1;
		product.resize(2 * size);
		for (std::size_t i = 0; i < size; ++i)
			product[size + i] = i < count ? F{-xs[i], T(1)} : F{T(1)};
		for (std::size_t i = size; --i;)
			product[i] = product[2 * i + 1].size() == 1 ? product[2 * i]
				: F::multiply(product[2 * i], product[2 * i + 1]);
	}
	std::vector<T> evaluate(const F &f) const {
		std::vector<T> result(count);
		if (!count) return result;
		auto remainder = [](const F &a, const F &b) {
			return a.size() < b.size() ? a : a.divmod(b).second;
		};
		auto visit = [&](auto &&self, std::size_t node, const F &value) -> void {
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
		value.shrink();
		visit(visit, 1, remainder(value, product[1]));
		return result;
	}
};
}

#endif
