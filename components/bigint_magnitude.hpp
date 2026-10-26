#ifndef BIGINT_MAGNITUDE_HPP
#define BIGINT_MAGNITUDE_HPP

#include "../convolution/convolution_mod.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace bigint_detail {
using Word = std::uint32_t;
using Magnitude = std::vector<Word>;
inline constexpr Word base = 1000000000;

inline void trim(Magnitude &a) {
	while (!a.empty() && !a.back()) a.pop_back();
}

inline int compare(const Magnitude &a, const Magnitude &b) {
	if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
	for (std::size_t i = a.size(); i--;) if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
	return 0;
}

inline Magnitude multiply_word(const Magnitude &a, Word b) {
	assert(b < base);
	if (!b || a.empty()) return {};
	if (b == 1) return a;
	Magnitude result(a.size());
	std::uint64_t carry = 0;
	for (std::size_t i = 0; i < a.size(); ++i) {
		std::uint64_t value = std::uint64_t(a[i]) * b + carry;
		result[i] = Word(value % base);
		carry = value / base;
	}
	if (carry) result.push_back(Word(carry));
	return result;
}

inline Word divide_word(Magnitude &a, Word b) {
	assert(b && b < base);
	if (b == 1) { trim(a); return 0; }
	std::uint64_t remainder = 0;
	for (std::size_t i = a.size(); i--;) {
		std::uint64_t value = remainder * base + a[i];
		a[i] = Word(value / b);
		remainder = value % b;
	}
	trim(a);
	return Word(remainder);
}

#if defined(__x86_64__) || defined(__i386__)
template <bool subtract>
__attribute__((target("avx2"))) inline Word carry_blocks(Word *a, const Word *b, std::size_t &offset, std::size_t n, Word carry) {
	const auto limit = _mm256_set1_epi32(base - 1), modulus = _mm256_set1_epi32(base);
	const auto one = _mm256_set1_epi32(1), zero = _mm256_setzero_si256();
	const auto indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
	for (; n - offset >= 32; offset += 32) {
		__m256i values[4];
		std::uint64_t generate = 0, propagate = 0;
		for (int j = 0; j < 4; ++j) {
			auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + offset + 8 * j));
			auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + offset + 8 * j));
			__m256i g, p;
			if constexpr (subtract) {
				values[j] = _mm256_sub_epi32(x, y);
				g = _mm256_cmpgt_epi32(y, x);
				p = _mm256_cmpeq_epi32(x, y);
			} else {
				values[j] = _mm256_add_epi32(x, y);
				g = _mm256_cmpgt_epi32(values[j], limit);
				p = _mm256_cmpeq_epi32(values[j], limit);
			}
			generate |= std::uint64_t(_mm256_movemask_ps(_mm256_castsi256_ps(g))) << (8 * j);
			propagate |= std::uint64_t(_mm256_movemask_ps(_mm256_castsi256_ps(p))) << (8 * j);
		}
		std::uint64_t incoming = (propagate + (generate << 1) + carry) ^ propagate;
		for (int j = 0; j < 4; ++j) {
			auto in = _mm256_and_si256(_mm256_srlv_epi32(_mm256_set1_epi32(int(incoming >> (8 * j))), indices), one);
			auto out = _mm256_and_si256(_mm256_srlv_epi32(_mm256_set1_epi32(int(incoming >> (8 * j + 1))), indices), one);
			auto correction = _mm256_and_si256(_mm256_sub_epi32(zero, out), modulus);
			auto value = subtract ? _mm256_add_epi32(_mm256_sub_epi32(values[j], in), correction)
								 : _mm256_sub_epi32(_mm256_add_epi32(values[j], in), correction);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + offset + 8 * j), value);
		}
		carry = Word(incoming >> 32);
	}
	return carry;
}
#endif

inline void add(Magnitude &a, const Magnitude &b) {
	std::size_t n = b.size();
	if (a.size() < n) a.resize(n);
	std::size_t i = 0;
	Word carry = 0;
#if defined(__x86_64__) || defined(__i386__)
	if (n >= 32 && __builtin_cpu_supports("avx2")) carry = carry_blocks<false>(a.data(), b.data(), i, n, carry);
#endif
	for (; i < n; ++i) {
		Word value = a[i] + b[i] + carry;
		carry = value >= base;
		a[i] = value - carry * base;
	}
	while (carry && i < a.size()) {
		Word value = a[i] + carry;
		carry = value >= base;
		a[i++] = value - carry * base;
	}
	if (carry) a.push_back(carry);
}

inline void subtract(Magnitude &a, const Magnitude &b) {
	assert(compare(a, b) >= 0);
	std::size_t n = b.size(), i = 0;
	Word borrow = 0;
#if defined(__x86_64__) || defined(__i386__)
	if (n >= 32 && __builtin_cpu_supports("avx2")) borrow = carry_blocks<true>(a.data(), b.data(), i, n, borrow);
#endif
	for (; i < n; ++i) {
		Word value = b[i] + borrow;
		borrow = a[i] < value;
		a[i] = a[i] + borrow * base - value;
	}
	while (borrow && i < a.size()) {
		Word value = a[i];
		borrow = !value;
		a[i++] = value + borrow * base - 1;
	}
	assert(!borrow);
	trim(a);
}

inline void increment(Magnitude &a) {
	std::size_t i = 0;
	while (i < a.size() && a[i] == base - 1) a[i++] = 0;
	if (i == a.size()) a.push_back(1);
	else ++a[i];
}

inline Magnitude multiply(const Magnitude &a, const Magnitude &b) {
	if (a.empty() || b.empty()) return {};
	if (a.size() == 1) return multiply_word(b, a[0]);
	if (b.size() == 1) return multiply_word(a, b[0]);
	if (std::min(a.size(), b.size()) <= 128) {
		std::vector<__uint128_t> coefficients(a.size() + b.size() - 1);
		for (std::size_t i = 0; i < a.size(); ++i)
			for (std::size_t j = 0; j < b.size(); ++j) coefficients[i + j] += std::uint64_t(a[i]) * b[j];
		Magnitude result(coefficients.size());
		__uint128_t carry = 0;
		for (std::size_t i = 0; i < coefficients.size(); ++i) {
			carry += coefficients[i];
			result[i] = Word(carry % base);
			carry /= base;
		}
		while (carry) { result.push_back(Word(carry % base)); carry /= base; }
		trim(result);
		return result;
	}
	constexpr std::size_t limit = std::size_t(1) << 24;
	if (a.size() > limit || b.size() > limit - a.size() + 1) throw std::length_error("BigInt multiplication length");
	auto x = convolution_mod_detail::multiply<convolution_mod_detail::p, 11>(a, b);
	auto y = convolution_mod_detail::multiply<convolution_mod_detail::q, 3>(a, b);
	auto z = convolution_mod_detail::multiply<convolution_mod_detail::r, 31>(a, b);
	Magnitude result(x.size());
	__uint128_t carry = 0;
	for (std::size_t i = 0; i < x.size(); ++i) {
		carry += convolution_mod_detail::restore(x[i], y[i], z[i]);
		result[i] = Word(carry % base);
		carry /= base;
	}
	while (carry) { result.push_back(Word(carry % base)); carry /= base; }
	trim(result);
	return result;
}

inline std::pair<Magnitude, Magnitude> divide_schoolbook(const Magnitude &a, const Magnitude &b) {
	assert(!b.empty());
	if (compare(a, b) < 0) return {{}, a};
	if (b.size() == 1) {
		Magnitude quotient = a;
		Word remainder = divide_word(quotient, b[0]);
		return {std::move(quotient), remainder ? Magnitude{remainder} : Magnitude{}};
	}
	std::size_t n = a.size(), m = b.size();
	Word normalization = base / (b.back() + 1);
	Magnitude u = multiply_word(a, normalization), v = multiply_word(b, normalization);
	u.resize(n + 1);
	Magnitude quotient(n - m + 1);
	for (std::size_t j = quotient.size(); j--;) {
		std::uint64_t numerator = std::uint64_t(u[j + m]) * base + u[j + m - 1];
		std::uint64_t digit = numerator / v[m - 1], remainder = numerator % v[m - 1];
		if (digit >= base) { digit = base - 1; remainder = numerator - digit * v[m - 1]; }
		while (remainder < base && digit * v[m - 2] > remainder * base + u[j + m - 2]) {
			--digit;
			remainder += v[m - 1];
		}
		std::uint64_t carry = 0;
		Word borrow = 0;
		for (std::size_t i = 0; i < m; ++i) {
			std::uint64_t product = digit * v[i] + carry;
			carry = product / base;
			Word value = Word(product % base) + borrow;
			borrow = u[j + i] < value;
			u[j + i] = u[j + i] + borrow * base - value;
		}
		std::uint64_t value = carry + borrow;
		bool negative = u[j + m] < value;
		u[j + m] = Word(u[j + m] + (negative ? base : 0) - value);
		if (negative) {
			--digit;
			Word overflow = 0;
			for (std::size_t i = 0; i < m; ++i) {
				Word sum = u[j + i] + v[i] + overflow;
				overflow = sum >= base;
				u[j + i] = sum - overflow * base;
			}
			u[j + m] += overflow;
			if (u[j + m] >= base) u[j + m] -= base;
		}
		quotient[j] = Word(digit);
	}
	u.resize(m);
	divide_word(u, normalization);
	trim(quotient);
	return {std::move(quotient), std::move(u)};
}

inline Magnitude reciprocal(const Magnitude &a) {
	assert(!a.empty() && a.back() >= base / 2);
	std::size_t n = a.size();
	if (n <= 32) {
		Magnitude numerator(2 * n + 1);
		numerator.back() = 1;
		return divide_schoolbook(numerator, a).first;
	}
	std::size_t half = (n + 1) / 2;
	Magnitude upper(a.end() - half, a.end());
	Magnitude x = reciprocal(upper);
	x.insert(x.begin(), n - half, 0);
	Magnitude error(2 * n + 1);
	error.back() = 2;
	subtract(error, multiply(a, x));
	Magnitude result = multiply(x, error);
	result.erase(result.begin(), result.begin() + 2 * n);
	trim(result);
	Magnitude difference(2 * n + 1);
	difference.back() = 1;
	subtract(difference, multiply(a, result));
	while (compare(difference, a) >= 0) { increment(result); subtract(difference, a); }
	return result;
}

inline std::pair<Magnitude, Magnitude> divide(const Magnitude &a, const Magnitude &b) {
	if (b.empty()) throw std::domain_error("BigInt division by zero");
	if (compare(a, b) < 0) return {{}, a};
	if (b.size() <= 64 || a.size() - b.size() <= 64) return divide_schoolbook(a, b);
	Word normalization = base / (b.back() + 1);
	Magnitude u = multiply_word(a, normalization), v = multiply_word(b, normalization);
	Magnitude inverse = reciprocal(v), quotient(u.size() - v.size() + 1), remainder;
	std::size_t n = v.size(), blocks = (u.size() - 1) / n + 1;
	for (std::size_t block = blocks; block--;) {
		std::size_t first = block * n, count = std::min(n, u.size() - first);
		remainder.insert(remainder.begin(), count, 0);
		std::copy_n(u.begin() + first, count, remainder.begin());
		trim(remainder);
		if (compare(remainder, v) < 0) continue;
		Magnitude digit = multiply(remainder, inverse);
		if (digit.size() <= 2 * n) digit.clear();
		else digit.erase(digit.begin(), digit.begin() + 2 * n);
		subtract(remainder, multiply(digit, v));
		if (compare(remainder, v) >= 0) { increment(digit); subtract(remainder, v); }
		assert(compare(remainder, v) < 0 && first + digit.size() <= quotient.size());
		std::copy(digit.begin(), digit.end(), quotient.begin() + first);
	}
	divide_word(remainder, normalization);
	trim(quotient);
	return {std::move(quotient), std::move(remainder)};
}
}

#endif
