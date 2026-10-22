#ifndef MULTIPLICATIVE_PREFIX_SUM_HPP
#define MULTIPLICATIVE_PREFIX_SUM_HPP

#include "../../components/floor_quotients.hpp"
#include "prime_sieve.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

template<class T> class MultiplicativePrefixSum {
	FloorQuotients quotients_;
	std::vector<std::size_t> primes_;

  public:
	explicit MultiplicativePrefixSum(std::uint64_t n = 0) : quotients_(n), primes_(prime_sieve(quotients_.root() + 1)) {}
	MultiplicativePrefixSum(const MultiplicativePrefixSum&) = default;
	MultiplicativePrefixSum& operator=(const MultiplicativePrefixSum&) = default;
	MultiplicativePrefixSum(MultiplicativePrefixSum&& other) noexcept : MultiplicativePrefixSum() { swap(other); }
	MultiplicativePrefixSum& operator=(MultiplicativePrefixSum&& other) noexcept {
		if (this != &other) { MultiplicativePrefixSum empty; swap(empty); swap(other); }
		return *this;
	}
	void swap(MultiplicativePrefixSum& other) noexcept { quotients_.swap(other.quotients_); primes_.swap(other.primes_); }
	std::uint64_t max_n() const { return quotients_.max_n(); }
	std::size_t size() const { return quotients_.size(); }
	const std::vector<std::uint64_t>& quotients() const { return quotients_.values(); }
	std::size_t index(std::uint64_t x) const { return quotients_.index(x); }
	T get(const std::vector<T>& table, std::uint64_t x) const {
		assert(table.size() == size());
		return x ? table[index(x)] : T(0);
	}
	template<class Prefix> std::vector<T> prime_sum(Prefix integer_prefix) const {
		std::vector<T> result;
		result.reserve(size());
		for (auto x : quotients()) result.push_back(integer_prefix(x) - T(1));
		const std::uint64_t n = max_n(), root = quotients_.root();
		const std::size_t large = quotients_.large_size(), length = size();
		for (std::uint64_t p : primes_) {
			const T weight = integer_prefix(p) - integer_prefix(p - 1);
			const T before = result[length - p + 1];
			const auto end = std::min<std::uint64_t>(large, n / (p * p));
			const auto split = std::min<std::uint64_t>(end, large / p);
			std::uint64_t i = 1;
			for (; i <= split; ++i) result[i - 1] -= weight * (result[i * p - 1] - before);
			for (; i <= end; ++i) result[i - 1] -= weight * (result[length - n / (i * p)] - before);
			std::uint64_t x = root;
			for (std::uint64_t q = root / p; q >= p; --q) {
				const T delta = weight * (result[length - q] - before);
				for (const auto stop = q * p; x >= stop; --x) result[length - x] -= delta;
			}
		}
		return result;
	}
	std::vector<T> prime_sum_linear(T constant, T linear) const {
		auto identity_prefix = [](std::uint64_t x) -> T {
			return x & 1 ? T(x) * (T(x / 2) + T(1)) : T(x / 2) * (T(x) + T(1));
		};
		if constexpr (requires { linear == T(0); }) {
			if (linear == T(0)) {
				if (constant == T(0)) return std::vector<T>(size(), T(0));
				auto result = prime_sum([](std::uint64_t x) { return T(x); });
				for (auto& value : result) value *= constant;
				return result;
			}
			if (constant == T(0)) {
				auto result = prime_sum(identity_prefix);
				for (auto& value : result) value *= linear;
				return result;
			}
		}
		auto result = prime_sum([](std::uint64_t x) { return T(x); });
		auto weighted = prime_sum(identity_prefix);
		for (std::size_t i = 0; i < size(); ++i) result[i] = result[i] * constant + weighted[i] * linear;
		return result;
	}
	template<class PrimePower> std::vector<T> prefix_sum(const std::vector<T>& prime_prefix, PrimePower prime_power) const {
		assert(prime_prefix.size() == size());
		std::vector<T> result = prime_prefix, weights;
		std::vector<std::uint64_t> powers;
		weights.reserve(64), powers.reserve(64);
		for (std::size_t j = primes_.size(); j--;) {
			const std::uint64_t p = primes_[j];
			const T before = prime_prefix[index(p)];
			weights.clear(), powers.clear();
			for (std::uint64_t power = p;; power *= p) {
				powers.push_back(power);
				weights.push_back(prime_power(p, static_cast<unsigned>(powers.size()), power));
				if (power > max_n() / p) break;
			}
			for (std::size_t i = 0; i < size() && quotients()[i] >= p * p; ++i) {
				T delta = T(0);
				for (std::size_t e = 0; e + 1 < powers.size() && powers[e + 1] <= quotients()[i]; ++e)
					delta += weights[e] * (result[quotients_.divided_index(i, powers[e])] - before) + weights[e + 1];
				result[i] += delta;
			}
		}
		for (auto& value : result) value += T(1);
		return result;
	}
	template<class PrimePower> T sum(const std::vector<T>& prime_prefix, PrimePower prime_power) const {
		assert(prime_prefix.size() == size());
		if (!max_n()) return T(0);
		std::vector<T> first, second;
		first.reserve(primes_.size()), second.reserve(primes_.size());
		for (std::uint64_t p : primes_) {
			first.push_back(prime_power(p, 1, p));
			second.push_back(prime_power(p, 2, p * p));
		}
		T result = prime_prefix[0] + T(1);
		auto dfs = [&](auto&& self, std::size_t i, unsigned e, std::uint64_t product, std::uint64_t power, T coefficient) -> void {
			const std::uint64_t p = primes_[i], limit = max_n() / product;
			result += coefficient * (e == 1 ? second[i] : prime_power(p, e + 1, power * p));
			if (p * p <= limit) self(self, i, e + 1, product * p, power * p, coefficient);
			coefficient *= e == 1 ? first[i] : e == 2 ? second[i] : prime_power(p, e, power);
			if constexpr (requires { coefficient == T(0); }) if (coefficient == T(0)) return;
			result += coefficient * (prime_prefix[index(limit)] - prime_prefix[index(p)]);
			std::size_t j = i + 1;
			for (; j < primes_.size(); ++j) {
				const std::uint64_t q = primes_[j];
				if (__uint128_t(q) * q * q > limit) break;
				self(self, j, 1, product * q, q, coefficient);
			}
			for (; j < primes_.size(); ++j) {
				const std::uint64_t q = primes_[j];
				if (q * q > limit) break;
				result += coefficient * (second[j] + first[j] * (prime_prefix[index(limit / q)] - prime_prefix[index(q)]));
			}
		};
		for (std::size_t i = 0; i < primes_.size(); ++i) dfs(dfs, i, 1, primes_[i], primes_[i], T(1));
		return result;
	}
};

#endif
