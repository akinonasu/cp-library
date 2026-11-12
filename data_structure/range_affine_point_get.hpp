#ifndef RANGE_AFFINE_POINT_GET_HPP
#define RANGE_AFFINE_POINT_GET_HPP

#include "../components/range_affine_point_simd.hpp"
#include "../math/mint.hpp"
#include "dual_segtree.hpp"
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

template<class Mint = mint>
class RangeAffinePointGet {
	struct Affine { Mint a, b; };
	static Mint mapping(Affine f, Mint x) { return f.a * x + f.b; }
	static Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
	static Affine id() { return {Mint(1), Mint(0)}; }
	using Fallback = DualSegtree<Mint, Affine, mapping, composition, id>;
	static constexpr unsigned modulus = [] {
		if constexpr (requires { std::integral_constant<unsigned, Mint::mod()>{}; }) return unsigned(Mint::mod());
		else return 0U;
	}();
	static constexpr bool supported = modulus >= 3 && modulus < (1U << 30) && (modulus & 1)
		&& std::is_constructible_v<Mint, unsigned> && requires(Mint value) { unsigned(value.val()); };
	using Fast = range_affine_point_detail::SimdTree<supported ? modulus : 998244353>;
	int n = 0;
	Fast fast;
	std::unique_ptr<Fallback> fallback;
	void range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n); }
	static unsigned encode(Mint value) {
		if constexpr (std::is_same_v<Mint, mint>) return value.v;
		else if constexpr (supported) return Fast::encode(unsigned(value.val()));
		else return 0;
	}
	static Mint decode(unsigned value) {
		if constexpr (std::is_same_v<Mint, mint>) return mint::word(value);
		else if constexpr (supported) return Mint(Fast::decode(value));
		else return Mint(0);
	}
	static std::vector<Mint> repeated(int size, const Mint &value) {
		assert(0 <= size && size <= std::numeric_limits<int>::max() / 4);
		return std::vector<Mint>(std::size_t(size), value);
	}
public:
	using value_type = Mint;
	RangeAffinePointGet() = default;
	explicit RangeAffinePointGet(int size, Mint value = Mint(0)) : RangeAffinePointGet(repeated(size, value)) {}
	explicit RangeAffinePointGet(const std::vector<Mint> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max() / 4));
		n = int(values.size());
		if (supported && range_affine_point_detail::simd_available()) {
			std::vector<unsigned> words(values.size());
			for (std::size_t i = 0; i < values.size(); ++i) words[i] = encode(values[i]);
			fast.build(words);
		} else fallback = std::make_unique<Fallback>(values);
	}
	RangeAffinePointGet(const RangeAffinePointGet &other)
		: n(other.n), fast(other.fast), fallback(other.fallback ? std::make_unique<Fallback>(*other.fallback) : nullptr) {}
	RangeAffinePointGet(RangeAffinePointGet &&other) noexcept { swap(other); }
	RangeAffinePointGet &operator=(RangeAffinePointGet other) noexcept { swap(other); return *this; }
	int size() const { return n; }
	bool empty() const { return !n; }
	void apply(int l, int r, Mint a, Mint b) {
		range(l, r);
		if (l == r) return;
		if (fallback) fallback->apply(l, r, {a, b});
		else fast.apply(unsigned(l), unsigned(r), encode(a), encode(b));
	}
	void apply(int p, Mint a, Mint b) { assert(0 <= p && p < n); apply(p, p + 1, a, b); }
	Mint get(int p) const {
		assert(0 <= p && p < n);
		return fallback ? std::as_const(*fallback).get(p) : decode(fast.get(unsigned(p)));
	}
	Mint operator[](int p) const { return get(p); }
	void set(int p, Mint value) { assert(0 <= p && p < n); apply(p, p + 1, Mint(0), value); }
	void swap(RangeAffinePointGet &other) noexcept { std::swap(n, other.n); fast.swap(other.fast); fallback.swap(other.fallback); }
	friend void swap(RangeAffinePointGet &a, RangeAffinePointGet &b) noexcept { a.swap(b); }
};

#endif
