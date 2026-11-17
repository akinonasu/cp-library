#ifndef FIXED_SIZE_SUBSET_SUM_CORE_HPP
#define FIXED_SIZE_SUBSET_SUM_CORE_HPP

#include "ntt_mod.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <type_traits>

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

template <class T>
concept IntegralValue = std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8;

template <class Sequence>
std::vector<long long> wide_sums(const Sequence &values, int k) {
	using Wide = __int128_t;
	int count = int(values.size());
	auto narrow = [](Wide x) {
		if (x < std::numeric_limits<long long>::min() || x > std::numeric_limits<long long>::max())
			throw std::overflow_error("subset sum is out of range");
		return static_cast<long long>(x);
	};
	Wide smallest = 0, largest = 0, total = 0;
	for (int i = 0; i < k; ++i) smallest += Wide(values[i]), largest += Wide(values[count - 1 - i]);
	narrow(smallest);
	narrow(largest);
	for (auto x : values) total += Wide(x);
	bool complement = k > count - k;
	if (complement) k = count - k;
	if (!k) return {narrow(total)};
	if (k == 1) {
		std::vector<long long> result;
		result.reserve(values.size());
		for (auto x : values) result.push_back(narrow(complement ? total - Wide(x) : Wide(x)));
		if (complement) std::reverse(result.begin(), result.end());
		return result;
	}
	std::uint64_t ways = 1;
	for (int i = 1; i <= k && ways <= 32768; ++i) ways = ways * (count - i + 1) / i;
	std::vector<Wide> sums;
	if (ways <= 32768) {
		sums.reserve(ways);
		auto visit = [&](auto &&self, int start, int left, Wide sum) -> void {
			if (!left) { sums.push_back(sum); return; }
			for (int i = start; i <= count - left; ++i) self(self, i + 1, left - 1, sum + Wide(values[i]));
		};
		visit(visit, 0, k, 0);
		std::sort(sums.begin(), sums.end());
		sums.erase(std::unique(sums.begin(), sums.end()), sums.end());
	} else {
		Wide base = Wide(values[0]);
		std::uint64_t step = 0;
		for (auto x : values) step = std::gcd(step, std::uint64_t(Wide(x) - base));
		if ((Wide(values.back()) - base) / step > std::numeric_limits<int>::max())
			throw std::length_error("sum range is too large");
		std::vector<int> coordinates;
		coordinates.reserve(values.size());
		for (auto x : values) coordinates.push_back(int((Wide(x) - base) / step));
		long long low = 0, high = 0;
		for (int i = 0; i < k; ++i) low += coordinates[i], high += coordinates[count - 1 - i];
		if (coordinates.back() == count - 1) {
			sums.reserve(std::size_t(high - low) + 1);
			for (long long x = low; x <= high; ++x) sums.push_back(Wide(k) * base + Wide(x) * step);
		} else {
			if (high >= (1 << 23)) throw std::length_error("sum range is too large");
			auto bound = coefficient_bound(count, k);
			std::vector<unsigned char> present(std::size_t(high - low) + 1);
			std::size_t missing = present.size();
			Digits product{1};
			for (auto pass : passes) {
				missing -= pass.include(coordinates, k, int(low), present);
				multiply(product, pass.modulus);
				if (!missing || less(bound, product)) break;
				int left = 0, right = 0, candidates = 0;
				long long small = low - coordinates[k - 1], large = high - coordinates[count - k];
				for (std::size_t i = 0; i < present.size() && candidates < count; ++i) {
					if (present[i]) continue;
					long long sum = low + i;
					while (left < count && coordinates[left] < sum - large) ++left;
					while (right < count && coordinates[right] <= sum - small) ++right;
					candidates = std::max(candidates, right - left);
				}
				bound = coefficient_bound(candidates, k);
				if (less(bound, product)) break;
			}
			sums.reserve(present.size() - missing);
			for (std::size_t i = 0; i < present.size(); ++i)
				if (present[i]) sums.push_back(Wide(k) * base + Wide(low + i) * step);
		}
	}
	std::vector<long long> result;
	result.reserve(sums.size());
	for (Wide sum : sums) result.push_back(narrow(complement ? total - sum : sum));
	if (complement) std::reverse(result.begin(), result.end());
	return result;
}
}

#endif
