#ifndef CP_COMPONENTS_FPS_PRODUCT_TREE_HPP
#define CP_COMPONENTS_FPS_PRODUCT_TREE_HPP

#include "../fps/fps.hpp"
#include "polynomial_product_tree.hpp"

namespace fps_detail {
template<class T>
struct ProductTree : polynomial_detail::ProductTree<FormalPowerSeries<T>> {
	using F = FormalPowerSeries<T>;
	using Base = polynomial_detail::ProductTree<F>;
	using Base::Base;
	using Base::evaluate;

	static std::vector<F> build(const std::vector<T>& xs, int& size, int block, bool reciprocal,
			std::vector<std::vector<T>>* frequency = nullptr) {
		const int groups = (static_cast<int>(xs.size()) + block - 1) / block;
		while (size < groups) size <<= 1;
		std::vector<F> tree(2 * size, F{T(1)});
		if (frequency) frequency->assign(2 * size, {});
		for (int i = 0; i < groups; ++i) {
			F& p = tree[size + i];
			p.reserve(block + 1);
			for (int j = i * block; j < std::min<int>((i + 1) * block, xs.size()); ++j) {
				p.push_back(T(0));
				if (reciprocal) {
					for (int k = p.size() - 1; k >= 1; --k) p[k] -= xs[j] * p[k - 1];
				} else {
					for (int k = p.size() - 1; k >= 1; --k) p[k] = p[k - 1] - xs[j] * p[k];
					p[0] *= -xs[j];
				}
			}
		}
		for (int i = size - 1; i; --i)
			tree[i] = tree[2 * i + 1].size() == 1 ? tree[2 * i] : Base::multiply(tree[2 * i], tree[2 * i + 1]);
		return tree;
	}

	static std::vector<T> evaluate(const std::vector<T>& points, const std::vector<F>& tree,
			std::vector<std::vector<T>>&, int size, int block, F root, bool = false) {
		F f = Base::multiply_low(root, tree[1], size * block);
		std::reverse(f.begin(), f.end());
		return Base(points).evaluate(f);
	}

};
}

#endif
