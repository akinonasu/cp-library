#ifndef FIXED_SIZE_SUBSET_SUM_CORE_HPP
#define FIXED_SIZE_SUBSET_SUM_CORE_HPP

#include "ntt_mod.hpp"
#include <array>
#include <stdexcept>

namespace fixed_size_subset_sum_detail {
using Word = std::uint32_t;
using Digits = std::vector<Word>;
inline void multiply(Digits& a, Word x) {
	std::uint64_t carry = 0;
	for (auto& v : a) {
		carry += std::uint64_t(v) * x;
		v = Word(carry);
		carry >>= 32;
	}
	if (carry) a.push_back(Word(carry));
}
inline void divide(Digits& a, Word x) {
	std::uint64_t carry = 0;
	for (std::size_t i = a.size(); i--;) {
		carry = (carry << 32) | a[i];
		a[i] = Word(carry / x);
		carry %= x;
	}
	while (a.size() > 1 && !a.back()) a.pop_back();
}
inline bool less(const Digits& a, const Digits& b) {
	if (a.size() != b.size()) return a.size() < b.size();
	return std::lexicographical_compare(a.rbegin(), a.rend(), b.rbegin(), b.rend());
}

template<Word Mod, Word Root>
std::size_t include_modular(const std::vector<int>& values, int k, int low, std::vector<unsigned char>& present) {
	std::size_t size = std::bit_ceil(std::size_t(low) + present.size());
	bool simd = false;
#if defined(__x86_64__) || defined(__i386__)
	simd = __builtin_cpu_supports("avx2");
#endif
	ntt_mod_detail::Plan<Mod, Root> plan(size, simd);
	std::vector<Word> frequency(size), result(size), powers(k + 1), elementary(k + 1), inverse(k + 1, 1);
	for (int x : values) frequency[x] = 1;
	ntt_mod_detail::forward(frequency, plan, simd);
	if constexpr (Mod < (Word(1) << 30)) for (auto& x : frequency) if (x >= Mod) x -= Mod;
	ntt_mod_detail::bit_reverse(frequency);
	for (int i = 2; i <= k; ++i) inverse[i] = Mod - std::uint64_t(Mod / i) * inverse[Mod % i] % Mod;
	elementary[0] = 1;
	for (std::size_t x = 0; x < size; ++x) {
		for (int j = 1; j <= k; ++j) {
			Word value = frequency[(x * j) & (size - 1)];
			powers[j] = (j & 1) || !value ? value : Mod - value;
		}
		elementary[1] = powers[1];
		for (int i = 2; i <= k; ++i) {
			__uint128_t sum = powers[i];
			for (int j = 1; j < i; ++j) sum += std::uint64_t(elementary[i - j]) * powers[j];
			elementary[i] = std::uint64_t(sum % Mod) * inverse[i] % Mod;
		}
		result[x] = elementary[k];
	}
	ntt_mod_detail::bit_reverse(result);
	ntt_mod_detail::backward(result, plan, simd);
	std::reverse(result.begin() + 1, result.end());
	std::size_t added = 0;
	for (std::size_t i = 0; i < present.size(); ++i) {
		Word x = result[low + i];
		bool nonzero = x && x != Mod;
		added += !present[i] && nonzero;
		present[i] |= nonzero;
	}
	return added;
}

struct Pass {
	Word modulus;
	std::size_t (*include)(const std::vector<int>&, int, int, std::vector<unsigned char>&);
};
inline constexpr std::array passes{
	Pass{998244353, include_modular<998244353, 3>},
	Pass{2130706433, include_modular<2130706433, 3>},
	Pass{2113929217, include_modular<2113929217, 5>},
	Pass{2088763393, include_modular<2088763393, 5>},
	Pass{2013265921, include_modular<2013265921, 31>},
	Pass{1811939329, include_modular<1811939329, 13>},
	Pass{1711276033, include_modular<1711276033, 29>},
	Pass{1484783617, include_modular<1484783617, 5>},
	Pass{1300234241, include_modular<1300234241, 3>},
	Pass{1224736769, include_modular<1224736769, 3>}
};

inline Digits coefficient_bound(int n, int k) {
	if (n < k) return {0};
	Digits bound{1}, limit{1};
	for (auto pass : passes) multiply(limit, pass.modulus);
	multiply(limit, Word(k));
	int degree = std::min(k - 1, n - k + 1);
	for (int i = 1; i <= degree; ++i) {
		multiply(bound, Word(n - i + 1));
		divide(bound, Word(i));
		if (!less(bound, limit)) throw std::length_error("too many subsets");
	}
	divide(bound, Word(k));
	return bound;
}
}

#endif
