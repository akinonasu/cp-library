#ifndef BERLEKAMP_MASSEY_HPP
#define BERLEKAMP_MASSEY_HPP

#include "../components/field_arithmetic.hpp"
#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>

namespace berlekamp_massey_detail {
template <class Ops>
std::vector<typename Ops::V> restore(const std::vector<typename Ops::V> &sequence) {
	using V = typename Ops::V;
	const int n = int(sequence.size());
	std::vector<V> reversed(sequence.rbegin(), sequence.rend()), current{V(1)}, previous{V(1)}, saved;
	current.reserve(sequence.size() + 1);
	previous.reserve(sequence.size() + 1);
	saved.reserve(sequence.size() + 1);
	int order = 0, shift = 1;
	V inverse = 1;
	for (int i = 0; i < n; ++i) {
		V discrepancy = Ops::dot(current.data(), reversed.data() + n - 1 - i, int(current.size()));
		if (Ops::zero(discrepancy, 0)) {
			++shift;
			continue;
		}
		bool grow = order <= i / 2;
		if (grow) saved.assign(current.begin(), current.end());
		V factor = Ops::neg(Ops::mul(discrepancy, inverse));
		if (current.size() < previous.size() + shift) current.resize(previous.size() + shift);
		Ops::axpy(current.data() + shift, previous.data(), int(previous.size()), factor);
		if (grow) {
			order = i + 1 - order;
			previous.swap(saved);
			inverse = Ops::inv(discrepancy);
			shift = 1;
		} else ++shift;
	}
	current.resize(std::size_t(order) + 1);
	current.erase(current.begin());
	for (auto &x : current) x = Ops::neg(x);
	return current;
}
}

template <class T>
std::vector<T> berlekamp_massey(const std::vector<T> &sequence) {
	static_assert(!std::is_integral_v<T> && !std::is_same_v<T, __int128_t> && !std::is_same_v<T, __uint128_t>);
	assert(sequence.size() < std::size_t(std::numeric_limits<int>::max()));
	if constexpr (field_detail::small_modint<T>) {
		std::vector<std::uint32_t> values(sequence.size());
		for (std::size_t i = 0; i < sequence.size(); ++i) values[i] = sequence[i].val();
		auto coefficients = berlekamp_massey_detail::restore<field_detail::ModularField<T>>(values);
		std::vector<T> result;
		result.reserve(coefficients.size());
		for (auto x : coefficients) result.push_back(T::raw(x));
		return result;
	} else return berlekamp_massey_detail::restore<field_detail::Field<T>>(sequence);
}

#endif
