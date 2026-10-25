#ifndef RANGE_AFFINE_RANGE_SUM_HPP
#define RANGE_AFFINE_RANGE_SUM_HPP

#include "../components/range_affine_sum_simd.hpp"
#include "../math/mint.hpp"
#include "lazy_segtree.hpp"
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

template<class Mint = mint>
class RangeAffineRangeSum {
	struct Summary { Mint sum; int length; };
	struct Affine { Mint a, b; };
	static Summary op(Summary a, Summary b) { return {a.sum + b.sum, a.length + b.length}; }
	static Summary e() { return {Mint(0), 0}; }
	static Summary mapping(Affine f, Summary x) { return {f.a * x.sum + f.b * Mint(x.length), x.length}; }
	static Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
	static Affine id() { return {Mint(1), Mint(0)}; }
	using Fallback = LazySegtree<Summary, op, e, Affine, mapping, composition, id>;
	static constexpr unsigned modulus = [] {
		if constexpr (requires { std::integral_constant<unsigned, Mint::mod()>{}; }) return unsigned(Mint::mod());
		else return 0U;
	}();
	static constexpr bool supported = modulus >= 3 && modulus < (1U << 30) && (modulus & 1);
	using Fast = range_affine_sum_detail::SimdTree<supported ? modulus : 998244353>;
	int n = 0;
	Fast fast;
	std::unique_ptr<Fallback> fallback;
	void range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n); }
	static Mint import(unsigned value) {
		if constexpr (supported) return Mint(value >= modulus ? value - modulus : value);
		else return Mint(value);
	}
	static std::vector<Mint> repeated(int size, const Mint &value) {
		assert(size >= 0);
		return std::vector<Mint>(std::size_t(size), value);
	}
public:
	using value_type = Mint;
	RangeAffineRangeSum() = default;
	explicit RangeAffineRangeSum(int size, Mint value = Mint(0)) : RangeAffineRangeSum(repeated(size, value)) {}
	explicit RangeAffineRangeSum(const std::vector<Mint> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = int(values.size());
		if (supported && range_affine_sum_detail::simd_available()) {
			std::vector<unsigned> canonical(values.size());
			for (std::size_t i = 0; i < values.size(); ++i) canonical[i] = unsigned(values[i].val());
			fast.build(canonical);
		} else {
			std::vector<Summary> a;
			a.reserve(values.size());
			for (const auto &value : values) a.push_back({value, 1});
			fallback = std::make_unique<Fallback>(a);
		}
	}
	RangeAffineRangeSum(const RangeAffineRangeSum &other)
		: n(other.n), fast(other.fast), fallback(other.fallback ? std::make_unique<Fallback>(*other.fallback) : nullptr) {}
	RangeAffineRangeSum(RangeAffineRangeSum &&other) noexcept { swap(other); }
	RangeAffineRangeSum &operator=(RangeAffineRangeSum other) noexcept { swap(other); return *this; }
	int size() const { return n; }
	void apply(int l, int r, Mint a, Mint b) {
		range(l, r);
		if (l == r) return;
		if (fallback) fallback->apply(l, r, {a, b});
		else fast.apply(unsigned(l), unsigned(r), unsigned(a.val()), unsigned(b.val()));
	}
	void apply(int p, Mint a, Mint b) { assert(0 <= p && p < n); apply(p, p + 1, a, b); }
	Mint prod(int l, int r) const {
		range(l, r);
		if (l == r) return Mint(0);
		if (fallback) return std::as_const(*fallback).prod(l, r).sum;
		return import(fast.prod(unsigned(l), unsigned(r)));
	}
	Mint sum(int l, int r) const { return prod(l, r); }
	Mint all_prod() const { return fallback ? fallback->all_prod().sum : import(fast.all_prod()); }
	Mint all_sum() const { return all_prod(); }
	Mint get(int p) const { assert(0 <= p && p < n); return prod(p, p + 1); }
	Mint operator[](int p) const { return get(p); }
	void set(int p, Mint value) { assert(0 <= p && p < n); apply(p, p + 1, Mint(0), value); }
	void swap(RangeAffineRangeSum &other) noexcept {
		std::swap(n, other.n); fast.swap(other.fast); fallback.swap(other.fallback);
	}
	friend void swap(RangeAffineRangeSum &a, RangeAffineRangeSum &b) noexcept { a.swap(b); }
};

#endif
