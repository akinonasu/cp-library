#include "components/polynomial_product_tree.hpp"
#include "fps/fps.hpp"
#include "fps/fps_light.hpp"
#include <cassert>
#include <random>

template<class Base>
struct LimitedSeries : Base {
	using Base::Base;
	LimitedSeries() = default;
	LimitedSeries(Base f) : Base(std::move(f)) {}
	static LimitedSeries multiply(const LimitedSeries& a, const LimitedSeries& b) {
		assert(a.empty() || b.empty() || a.size() + b.size() - 1 <= 32);
		return Base::multiply(a, b);
	}
	std::pair<LimitedSeries, LimitedSeries> divmod(const LimitedSeries& b) const {
		auto [q, r] = Base::divmod(b);
		return {std::move(q), std::move(r)};
	}
};

template<class F>
void check() {
	using T = typename F::value_type;
	std::mt19937 random(84191);
	for (int lead : {0, 1, 7}) {
		F a(17), b(17), expected(33);
		for (auto& x : a) x = T(random());
		for (auto& x : b) x = T(random());
		a.back() = T(lead);
		for (std::size_t i = 0; i < a.size(); ++i)
			for (std::size_t j = 0; j < b.size(); ++j) expected[i + j] += a[i] * b[j];
		assert((polynomial_detail::ProductTree<F, 32>::multiply(a, b) == expected));
		for (int n : {0, 1, 16, 17, 31, 32}) {
			auto prefix = expected;
			prefix.resize(n);
			assert((polynomial_detail::ProductTree<F, 32>::multiply_low(a, b, n) == prefix));
		}
	}
	for (int n : {0, 1, 2, 15, 16, 17, 31, 32}) {
		std::vector<T> xs(n), ys(n);
		F f(n), expected{T(1)};
		for (int i = 0; i < n; ++i) {
			xs[i] = T(i * 7);
			f[i] = T(random());
			F next(expected.size() + 1);
			for (std::size_t j = 0; j < expected.size(); ++j) {
				next[j] -= expected[j] * xs[i];
				next[j + 1] += expected[j];
			}
			expected = std::move(next);
		}
		for (int i = 0; i < n; ++i) ys[i] = f.eval(xs[i]);
		polynomial_detail::ProductTree<F, 32> tree(xs);
		assert(tree.product[1] == expected);
		assert(tree.evaluate(f) == ys);
		assert(tree.interpolate(ys) == f);
		if (n) {
			xs.back() = xs[0];
			ys.back() = ys[0];
			assert((polynomial_detail::ProductTree<F, 32>(xs).evaluate(f) == ys));
		}
	}
}

int main() {
	check<LimitedSeries<fps>>();
	check<LimitedSeries<light_fps>>();
}
