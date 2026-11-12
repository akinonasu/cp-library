#ifndef LCM_CONVOLUTION_MINT_HPP
#define LCM_CONVOLUTION_MINT_HPP

#include "divisor_zeta.hpp"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace lcm_convolution_mint_detail {
using divisor_zeta_detail::Stage;
using divisor_zeta_detail::make_plan;

template<bool inverse, class T>
void transform(std::vector<T> &a, std::vector<T> &b, const Stage &stage, std::uint32_t block, std::uint32_t split) {
	const auto &divisors = stage.divisors;
	if (divisors.empty()) return;
	std::uint32_t n = std::uint32_t(a.size() - 1), end = n / divisors[0];
	std::size_t large = std::size_t(std::upper_bound(divisors.begin(), divisors.end(), split) - divisors.begin());
	std::uint64_t large_first = large < divisors.size() ? divisors[large] : std::uint64_t(n) + 1;
	std::vector<std::size_t> cursor(std::size_t(n / large_first) + 1, large);
	std::vector<std::uint64_t> next(large);
	std::vector<std::uint32_t> source(large, 1);
	std::vector<T> x, y;
	if constexpr (!inverse) {
		x.assign(a.begin(), a.begin() + std::size_t(end) + 1);
		y.assign(b.begin(), b.begin() + std::size_t(end) + 1);
	}
	for (std::size_t j = 0; j < large; ++j) next[j] = divisors[j];
	std::uint64_t first = 1;
	if constexpr (inverse) {
		std::uint32_t last = std::min(n, block);
		for (std::uint32_t i = 1; i <= last / divisors[0]; ++i) {
			std::uint32_t limit = last / i;
			std::size_t j = 0;
			T value = a[i];
			while (j < divisors.size() && divisors[j] <= limit) a[std::size_t(i) * divisors[j++]] -= value;
			if (i < cursor.size()) cursor[i] = std::max(j, large);
		}
		for (std::size_t j = 0; j < large; ++j) {
			std::uint32_t i = last / divisors[j] + 1;
			source[j] = i;
			next[j] = std::uint64_t(i) * divisors[j];
		}
		first = std::uint64_t(last) + 1;
	}
	for (; first <= n; first += block) {
		std::uint32_t last = std::uint32_t(std::min<std::uint64_t>(n, first + block - 1));
		for (std::size_t j = 0; j < large; ++j) {
			std::uint64_t at = next[j];
			std::uint32_t i = source[j], d = divisors[j];
			if constexpr (inverse) {
				for (; at <= last; at += d, ++i) a[std::size_t(at)] -= a[i];
			} else {
				for (; at <= last; at += d, ++i) {
					a[std::size_t(at)] += x[i];
					b[std::size_t(at)] += y[i];
				}
			}
			next[j] = at;
			source[j] = i;
		}
		for (std::uint32_t i = 1; i <= last / large_first; ++i) {
			std::uint32_t limit = last / i;
			std::size_t j = cursor[i];
			T value = inverse ? a[i] : x[i];
			while (j < divisors.size() && divisors[j] <= limit) {
				std::size_t to = std::size_t(i) * divisors[j++];
				if constexpr (inverse) a[to] -= value;
				else a[to] += value, b[to] += y[i];
			}
			cursor[i] = j;
		}
	}
}

template<class T>
void multiply(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &primes) {
	std::uint32_t n = std::uint32_t(a.size() - 1);
	if (!n) return;
	std::size_t small;
	auto stages = make_plan(n, primes, small);
	std::uint32_t block = std::max(8192u, std::uint32_t(1) << ((std::bit_width(n) + 1) / 2));
	std::uint32_t split = std::max(4096u, std::uint32_t((std::uint64_t(n) + block - 1) / block));
	divisor_zeta_detail::for_each<false, false>(a.size(), primes, small, [&](std::size_t to, std::size_t from) {
		a[to] += a[from], b[to] += b[from];
	});
	for (const auto &stage : stages) transform<false>(a, b, stage, block, split);
	for (std::size_t i = 1; i < a.size(); ++i) a[i] *= b[i];
	for (auto it = stages.rbegin(); it != stages.rend(); ++it) transform<true>(a, b, *it, block, split);
	divisor_zeta_detail::for_each<true, false>(a.size(), primes, small, [&](std::size_t to, std::size_t from) { a[to] -= a[from]; });
}
}

#endif
