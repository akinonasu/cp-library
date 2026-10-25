#ifndef COMB_MOD_HPP
#define COMB_MOD_HPP

#include "../../components/factorial_table_generic.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <vector>

namespace comb_mod_detail {
template <class Mint> struct Table {
	std::uint64_t modulus;
	std::vector<Mint> fact, finv;
	Table(int max_n, std::uint64_t mod) : modulus(mod) {
		const int n = int(std::min(std::uint64_t(max_n), mod - 1));
		fact.resize(n + 1);
		finv.resize(n + 1);
		factorial_table_detail::build(fact.data(), finv.data(), n);
	}
};
template <class Mint> struct State {
	int max_n;
	std::unique_ptr<Table<Mint>> data;
	explicit State(int n) : max_n(n) {}
	const Table<Mint> &get(int i) {
		const auto mod = Mint::mod();
		assert(mod > 1 && 0 <= i && i <= max_n && std::uint64_t(i) < std::uint64_t(mod));
		if (!data || data->modulus != std::uint64_t(mod)) data = std::make_unique<Table<Mint>>(max_n, mod);
		return *data;
	}
};
template <class Mint> struct Choose {
	std::shared_ptr<State<Mint>> state;
	Mint operator()(int n, int k) const {
		if (n < 0 || k < 0 || k > n) return Mint(0);
		const auto &t = state->get(n);
		return t.fact[n] * t.finv[k] * t.finv[n - k];
	}
};
template <class Mint, bool Inverse> struct Factorial {
	std::shared_ptr<State<Mint>> state;
	Mint operator[](int i) const {
		if (i < 0) return Mint(0);
		const auto &t = state->get(i);
		if constexpr (Inverse) return t.finv[i];
		else return t.fact[i];
	}
};
}

template <class Mint> struct CombMod {
	comb_mod_detail::Choose<Mint> comb;
	comb_mod_detail::Factorial<Mint, false> fact;
	comb_mod_detail::Factorial<Mint, true> finv;
};

template <class Mint>
CombMod<Mint> make_comb(int max_n = 10000000) {
	assert(0 <= max_n && max_n < std::numeric_limits<int>::max());
	auto state = std::make_shared<comb_mod_detail::State<Mint>>(max_n);
	return {{state}, {state}, {state}};
}

#endif
