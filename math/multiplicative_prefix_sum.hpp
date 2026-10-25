#ifndef MULTIPLICATIVE_PREFIX_SUM_HPP
#define MULTIPLICATIVE_PREFIX_SUM_HPP

#include "../components/floor_quotients.hpp"
#include "../components/multiplicative_sum_backend.hpp"
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

	bool use_backend(T& inverse) const {
		if constexpr (requires { std::uint64_t(T::mod()); T(0) == T(0); }) {
			const auto modulus = std::uint64_t(T::mod());
			if (max_n() >= (UINT64_C(1) << 32) && modulus > 1 && modulus <= UINT32_MAX && modulus % 3) {
				inverse = T((modulus * (modulus % 3 == 1 ? 2 : 1) + 1) / 3);
				return T(3) * inverse == T(1);
			}
		}
		return false;
	}

	template<class R, class Prefix> std::vector<R> prime_sum_impl(Prefix&& integer_prefix) const {
		const std::uint64_t n = max_n(), root = quotients_.root();
		const std::size_t large = quotients_.large_size(), length = size(), half = (root + 1) / 2;
		const R two = n >= 2 ? integer_prefix(2) - integer_prefix(1) : R(0);
		std::vector<R> result, small;
		result.reserve(length), small.reserve(half);
		auto initial = [&](std::uint64_t x) { return x == 1 ? R(0) : integer_prefix(x) - two * integer_prefix(x / 2) - R(1); };
		for (std::size_t i = 0; i < large; ++i) result.push_back(initial(quotients()[i]));
		for (std::size_t i = 0; i < half; ++i) small.push_back(initial(2 * i + 1));
		for (std::uint64_t p : primes_) {
			if (p == 2) continue;
			const R weight = integer_prefix(p) - integer_prefix(p - 1), before = small[(p - 2) / 2];
			const auto end = std::min<std::uint64_t>(large, n / (p * p));
			const auto split = std::min<std::uint64_t>(end, large / p);
			std::uint64_t i = 1;
			for (; i <= split; ++i) result[i - 1] -= weight * (result[i * p - 1] - before);
			for (; i <= end; ++i) result[i - 1] -= weight * (small[(n / (i * p) - 1) / 2] - before);
			std::size_t x = half;
			for (std::uint64_t q = (root / p - 1) | 1; q >= p; q -= 2) {
				const R delta = weight * (small[(q - 1) / 2] - before);
				for (const auto stop = (q * p + 1) / 2; x >= stop; --x) small[x - 1] -= delta;
			}
		}
		for (auto& value : result) value += two;
		for (auto x = root; x; --x) result.push_back(small[(x - 1) / 2] + (x >= 2 ? two : R(0)));
		return result;
	}


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
		T inverse(0);
		if (use_backend(inverse)) return multiplicative_sum_detail::Backend<T>(quotients_, primes_).prime_sum(integer_prefix, inverse);
		return prime_sum_impl<T>(integer_prefix);
	}
	std::vector<T> prime_sum_linear(T constant, T linear) const {
		auto identity_prefix = [](std::uint64_t x) -> T {
			return x & 1 ? T(x) * (T(x / 2) + T(1)) : T(x / 2) * (T(x) + T(1));
		};
		T inverse(0);
		const bool split = use_backend(inverse);
		if constexpr (requires { linear == T(0); }) {
			if (linear == T(0)) {
				if (constant == T(0)) return std::vector<T>(size(), T(0));
				if (split) {
					auto prefix = [](std::uint64_t x) { return T(x); };
					auto result = multiplicative_sum_detail::Backend<T>(quotients_, primes_).prime_sum(prefix, inverse);
					for (auto& value : result) value *= constant;
					return result;
				}
				auto counts = prime_sum_impl<std::uint64_t>([](std::uint64_t x) { return x; });
				std::vector<T> result;
				result.reserve(size());
				for (auto value : counts) result.push_back(T(value) * constant);
				return result;
			}
			if (constant == T(0)) {
				auto result = prime_sum(identity_prefix);
				for (auto& value : result) value *= linear;
				return result;
			}
		}
		if (split) {
			using Weights = multiplicative_sum_detail::LinearWeights<T>;
			auto prefix = [&](std::uint64_t x) { return Weights(T(x), identity_prefix(x)); };
			auto sums = multiplicative_sum_detail::Backend<Weights>(quotients_, primes_).prime_sum(prefix, Weights(inverse));
			std::vector<T> result;
			result.reserve(size());
			for (const auto& value : sums) result.push_back(value.count * constant + value.weighted * linear);
			return result;
		}
		const std::uint64_t n = max_n(), root = quotients_.root();
		const std::size_t large = quotients_.large_size(), length = size(), half = (root + 1) / 2;
		std::vector<T> result, small;
		std::vector<std::uint64_t> count;
		std::vector<std::uint32_t> count_small;
		result.reserve(length), small.reserve(half);
		count.reserve(large), count_small.reserve(half);
		for (std::size_t i = 0; i < large; ++i) {
			const auto x = quotients()[i], c = x / 2 + (x & 1);
			count.push_back(std::uint64_t(c - 1));
			result.push_back(T(c) * T(c) - T(1));
		}
		for (std::size_t i = 0; i < half; ++i) {
			count_small.push_back(std::uint32_t(i));
			small.push_back(T(i + 1) * T(i + 1) - T(1));
		}
		for (std::uint64_t p : primes_) {
			if (p == 2) continue;
			const T weight = T(p), before = small[(p - 2) / 2];
			const std::uint64_t count_before = count_small[(p - 2) / 2];
			const auto end = std::min<std::uint64_t>(large, n / (p * p));
			const auto split = std::min<std::uint64_t>(end, large / p);
			std::uint64_t i = 1;
			for (; i <= split; ++i) {
				count[i - 1] -= count[i * p - 1] - count_before;
				result[i - 1] -= weight * (result[i * p - 1] - before);
			}
			for (; i <= end; ++i) {
				const auto at = (n / (i * p) - 1) / 2;
				count[i - 1] -= count_small[at] - count_before;
				result[i - 1] -= weight * (small[at] - before);
			}
			std::size_t x = half;
			for (std::uint64_t q = (root / p - 1) | 1; q >= p; q -= 2) {
				const std::uint64_t count_delta = count_small[(q - 1) / 2] - count_before;
				const T delta = weight * (small[(q - 1) / 2] - before);
				for (const auto stop = (q * p + 1) / 2; x >= stop; --x) {
					count_small[x - 1] -= count_delta;
					small[x - 1] -= delta;
				}
			}
		}
		for (std::size_t i = 0; i < large; ++i) result[i] = (T(count[i]) + T(1)) * constant + (result[i] + T(2)) * linear;
		for (auto x = root; x; --x) result.push_back((T(count_small[(x - 1) / 2]) + T(x >= 2)) * constant + (small[(x - 1) / 2] + T(x >= 2 ? 2 : 0)) * linear);
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
			const T first = weights[0], second = weights[1];
			const auto length = size(), large = quotients_.large_size();
			const auto end = std::min<std::uint64_t>(large, max_n() / (p * p));
			std::size_t i = 0;
			if (powers.size() > 2) for (; i < end && quotients()[i] >= powers[2]; ++i) {
				const auto d0 = (i + 1) * p;
				T delta = first * (result[d0 <= large ? d0 - 1 : length - max_n() / d0] - before) + second;
				for (std::size_t e = 1; e + 1 < powers.size() && powers[e + 1] <= quotients()[i]; ++e) {
					const auto d = (i + 1) * powers[e];
					delta += weights[e] * (result[d <= large ? d - 1 : length - max_n() / d] - before) + weights[e + 1];
				}
				result[i] += delta;
			}
			const auto split = std::min<std::uint64_t>(end, large / p);
			for (; i < split; ++i)
				result[i] += first * (result[(i + 1) * p - 1] - before) + second;
			for (; i < end; ++i)
				result[i] += first * (result[length - max_n() / ((i + 1) * p)] - before) + second;
			std::uint64_t x = quotients_.root();
			while (x >= p * p) {
				const auto q = x / p, stop = q * p;
				T delta = first * (result[length - q] - before) + second;
				for (std::size_t e = 1; e + 1 < powers.size() && powers[e + 1] <= x; ++e)
					delta += weights[e] * (result[length - x / powers[e]] - before) + weights[e + 1];
				for (; x >= stop; --x) result[length - x] += delta;
			}
		}
		for (auto& value : result) value += T(1);
		return result;
	}
	template<class PrimePower> T sum(const std::vector<T>& prime_prefix, PrimePower prime_power) const {
		assert(prime_prefix.size() == size());
		if (!max_n()) return T(0);
		T inverse(0);
		if (use_backend(inverse)) return multiplicative_sum_detail::Backend<T>(quotients_, primes_).sum(prime_prefix, prime_power, inverse);
		std::vector<T> first, second, square_prefix{T(0)};
		first.reserve(primes_.size()), second.reserve(primes_.size()), square_prefix.reserve(primes_.size() + 1);
		for (std::uint64_t p : primes_) {
			first.push_back(prime_power(p, 1, p));
			second.push_back(prime_power(p, 2, p * p));
			square_prefix.push_back(square_prefix.back() + second.back());
		}
		T result = prime_prefix[0] + T(1);
		const auto length = size(), large = quotients_.large_size();
		auto dfs = [&](auto&& self, std::size_t i, unsigned e, std::uint64_t product, std::uint64_t power) -> T {
			const std::uint64_t p = primes_[i], limit = max_n() / product;
			T delta = e == 1 ? second[i] : prime_power(p, e + 1, power * p);
			if (p * p <= limit) delta += self(self, i, e + 1, product * p, power * p);
			T weight = e == 1 ? first[i] : e == 2 ? second[i] : prime_power(p, e, power);
			if constexpr (requires { weight == T(0); }) if (weight == T(0)) return delta;
			T tail = prime_prefix[product <= large ? product - 1 : length - limit] - prime_prefix[length - p];
			std::size_t j = i + 1;
			for (; j < primes_.size(); ++j) {
				const std::uint64_t q = primes_[j];
				if (__uint128_t(q) * q * q > limit) break;
				tail += self(self, j, 1, product * q, q);
			}
			const auto begin = j;
			for (; j < primes_.size(); ++j) {
				const std::uint64_t q = primes_[j];
				if (q * q > limit) break;
				tail += first[j] * (prime_prefix[product * q <= large ? product * q - 1 : length - limit / q] - prime_prefix[length - q]);
			}
			return delta + weight * (tail + square_prefix[j] - square_prefix[begin]);
		};
		for (std::size_t i = 0; i < primes_.size(); ++i) result += dfs(dfs, i, 1, primes_[i], primes_[i]);
		return result;
	}
};

#endif
