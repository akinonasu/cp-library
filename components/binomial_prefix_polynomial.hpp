#ifndef CP_COMPONENTS_BINOMIAL_PREFIX_POLYNOMIAL_HPP
#define CP_COMPONENTS_BINOMIAL_PREFIX_POLYNOMIAL_HPP

#include "binomial_prefix_core.hpp"
#include "../fps/fps.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace binomial_prefix_detail {
template<class Mint> struct PolynomialScalarArithmetic {
	using F = std::vector<Mint>;
	static void add(F& a, const F& b) {
		a.resize(std::max(a.size(), b.size()));
		for (std::size_t i = 0; i < b.size(); ++i) a[i] += b[i];
	}
	static void trim(F& a) {
		if constexpr (requires(const Mint& x) { x == Mint(0); })
			while (!a.empty() && a.back() == Mint(0)) a.pop_back();
	}
	static F multiply(const F& a, const F& b) {
		if (a.empty() || b.empty()) return {};
		F c(a.size() + b.size() - 1);
		if (std::min(a.size(), b.size()) <= 32) {
			for (std::size_t i = 0; i < a.size(); ++i)
				for (std::size_t j = 0; j < b.size(); ++j) c[i + j] += a[i] * b[j];
			return c;
		}
		std::size_t mid = (std::max(a.size(), b.size()) + 1) / 2;
		F al(a.begin(), a.begin() + std::min(mid, a.size()));
		F ah(a.begin() + std::min(mid, a.size()), a.end());
		F bl(b.begin(), b.begin() + std::min(mid, b.size()));
		F bh(b.begin() + std::min(mid, b.size()), b.end());
		F low = multiply(al, bl), high = multiply(ah, bh);
		add(al, ah), add(bl, bh);
		F cross = multiply(al, bl);
		for (std::size_t i = 0; i < low.size(); ++i) cross[i] -= low[i], c[i] += low[i];
		for (std::size_t i = 0; i < high.size(); ++i) cross[i] -= high[i], c[2 * mid + i] += high[i];
		for (std::size_t i = 0; i < cross.size() && mid + i < c.size(); ++i) c[mid + i] += cross[i];
		return c;
	}
	static F inverse(const F& a, std::size_t size) {
		Mint constant = a[0];
		F b{constant.inv()};
		while (b.size() < size) {
			std::size_t next = b.size() + std::min(b.size(), size - b.size());
			F prefix(a.begin(), a.begin() + std::min(a.size(), next));
			F error = multiply(prefix, b);
			error.resize(next);
			for (auto& x : error) x = Mint(0) - x;
			error[0] += Mint(2);
			b = multiply(b, error);
			b.resize(next);
		}
		return b;
	}
	static F remainder(F a, const F& b) {
		trim(a);
		if (a.size() < b.size()) return a;
		std::size_t size = a.size() - b.size() + 1;
		if (size <= 32 || b.size() <= 32) {
			for (std::size_t i = size; i-- > 0;) {
				Mint coefficient = a[i + b.size() - 1];
				for (std::size_t j = 0; j + 1 < b.size(); ++j) a[i + j] -= coefficient * b[j];
			}
		} else {
			F reversed(b.rbegin(), b.rend());
			F top(a.rbegin(), a.rbegin() + size);
			F quotient = multiply(top, inverse(reversed, size));
			quotient.resize(size);
			std::reverse(quotient.begin(), quotient.end());
			F product = multiply(quotient, b);
			for (std::size_t i = 0; i + 1 < b.size(); ++i) a[i] -= product[i];
		}
		a.resize(b.size() - 1);
		trim(a);
		return a;
	}
	static Mint eval(const F& a, Mint x) {
		Mint value(0);
		for (auto i = a.rbegin(); i != a.rend(); ++i) value = value * x + *i;
		return value;
	}
};

template<class Mint> struct PolynomialArithmetic : PolynomialScalarArithmetic<Mint> {};

template<> struct PolynomialArithmetic<mint> {
	using F = fps;
	static void add(F& a, const F& b) { a += b; }
	static F multiply(const F& a, const F& b) {
		if (!a.empty() && !b.empty() && a.size() + b.size() - 1 > (1 << 23))
			return F(PolynomialScalarArithmetic<mint>::multiply(a, b));
		return F::multiply(a, b);
	}
	static F remainder(F a, const F& b) {
		while (!a.empty() && a.back() == mint(0)) a.pop_back();
		if (a.size() < b.size()) return a;
		if (a.size() > (1 << 22)) return F(PolynomialScalarArithmetic<mint>::remainder(std::move(a), b));
		return a.divmod(b).second;
	}
	static mint eval(const F& a, mint x) {
		mint value(0);
		for (auto i = a.rbegin(); i != a.rend(); ++i) value = value * x + *i;
		return value;
	}
};

template<class Mint> class Polynomial {
	using Arithmetic = PolynomialArithmetic<Mint>;
	using F = typename Arithmetic::F;
	struct Point { int m, n; std::uint32_t id; };
	struct Node {
		F product, sum, modulus;
		int begin = 0, end = 0, start = 0;
	};
	static constexpr int block = 16;
	const Table<Mint>& table_;
	std::vector<Point> points_;
	std::vector<int> groups_;
	std::vector<Node> tree_;
	std::vector<Mint>& result_;
	static void trim(F& f) {
		if constexpr (requires(const Mint& x) { x == Mint(0); })
			while (!f.empty() && f.back() == Mint(0)) f.pop_back();
	}
	const Point& point(int i) const { return points_[groups_[i]]; }
	std::pair<F, F> basis(int a, int b) const {
		if (a == b) return {F{1}, F{}};
		if (b - a <= 24) {
			F product{1}, sum(b - a);
			for (int i = a; i < b; ++i) {
				for (int j = 0; j < int(product.size()); ++j) sum[j] += product[j] * table_.inverse_factorial[i];
				product.push_back(product.back());
				for (int j = int(product.size()) - 2; j >= 1; --j) product[j] = product[j - 1] - product[j] * Mint(i);
				product[0] *= Mint(0) - Mint(i);
			}
			trim(sum);
			return {std::move(product), std::move(sum)};
		}
		int mid = a + (b - a) / 2;
		auto [left, sum] = basis(a, mid);
		auto [right, tail] = basis(mid, b);
		Arithmetic::add(sum, Arithmetic::multiply(left, tail));
		left = Arithmetic::multiply(left, right);
		trim(sum);
		return {std::move(left), std::move(sum)};
	}
	void build(int node, int l, int r, int start, bool need_basis) {
		auto& cur = tree_[node];
		cur.begin = l, cur.end = r, cur.start = start;
		if (r - l <= block) {
			if (need_basis) {
				auto pair = basis(start, point(r - 1).m);
				cur.product = std::move(pair.first), cur.sum = std::move(pair.second);
			}
			cur.modulus = {1};
			for (int i = l; i < r; ++i) {
				Mint x(point(i).n);
				cur.modulus.push_back(cur.modulus.back());
				for (int j = int(cur.modulus.size()) - 2; j >= 1; --j) cur.modulus[j] = cur.modulus[j - 1] - x * cur.modulus[j];
				cur.modulus[0] *= Mint(0) - x;
			}
			return;
		}
		int mid = l + (r - l) / 2;
		build(2 * node, l, mid, start, true);
		build(2 * node + 1, mid, r, point(mid - 1).m, need_basis);
		const auto& left = tree_[2 * node];
		const auto& right = tree_[2 * node + 1];
		if (node != 1) cur.modulus = Arithmetic::multiply(left.modulus, right.modulus);
		if (need_basis) {
			cur.product = Arithmetic::multiply(left.product, right.product);
			cur.sum = left.sum;
			Arithmetic::add(cur.sum, Arithmetic::multiply(left.product, right.sum));
			trim(cur.sum);
		}
	}
	void evaluate(int node, F sum, F product) {
		const auto& cur = tree_[node];
		if (cur.end - cur.begin <= block) {
			for (int i = cur.begin; i < cur.end; ++i) {
				const auto& query = point(i);
				const int m = query.m, n = query.n;
				const Mint x(n);
				Mint tail(0), factor(1);
				for (int j = cur.start; j < m; ++j) {
					tail += factor * table_.inverse_factorial[j];
					factor *= x - Mint(j);
				}
				Mint value = Arithmetic::eval(sum, x) + Arithmetic::eval(product, x) * tail;
				for (int j = groups_[i]; j < groups_[i + 1]; ++j) {
					auto destination = points_[j].id;
					result_[destination >> 1] = destination & 1 ? table_.power2[n] - value : value;
				}
			}
			return;
		}
		const auto& left = tree_[2 * node];
		const auto& right = tree_[2 * node + 1];
		F right_sum = sum, right_product = product;
		Arithmetic::add(right_sum, Arithmetic::multiply(product, left.sum));
		right_product = Arithmetic::multiply(right_product, left.product);
		right_sum = Arithmetic::remainder(std::move(right_sum), right.modulus);
		right_product = Arithmetic::remainder(std::move(right_product), right.modulus);
		sum = Arithmetic::remainder(std::move(sum), left.modulus);
		product = Arithmetic::remainder(std::move(product), left.modulus);
		evaluate(2 * node, std::move(sum), std::move(product));
		evaluate(2 * node + 1, std::move(right_sum), std::move(right_product));
	}

  public:
	Polynomial(const Table<Mint>& table, const std::vector<std::pair<int, int>>& points,
			   const std::vector<std::uint32_t>& ids, std::vector<Mint>& result) : table_(table), result_(result) {
		assert(!points.empty() && points.size() == ids.size());
		points_.reserve(points.size());
		for (int i = 0; i < int(points.size()); ++i) points_.push_back({points[i].first, points[i].second, ids[i]});
		std::sort(points_.begin(), points_.end(), [](const Point& a, const Point& b) {
			return a.m != b.m ? a.m < b.m : a.n < b.n;
		});
		groups_.reserve(points_.size() + 1);
		for (int i = 0; i < int(points_.size()); ++i)
			if (!i || points_[i].m != points_[i - 1].m || points_[i].n != points_[i - 1].n) groups_.push_back(i);
		const int count = int(groups_.size());
		groups_.push_back(int(points_.size()));
		int size = 1;
		while (std::size_t(size) * block < std::size_t(count)) size <<= 1;
		tree_.resize(std::size_t(2) * size);
		build(1, 0, count, 0, false);
		evaluate(1, F{}, F{1});
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
