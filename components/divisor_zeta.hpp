#ifndef DIVISOR_ZETA_HPP
#define DIVISOR_ZETA_HPP

#include "../math/prime_sieve.hpp"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace divisor_zeta_detail {

struct Stage {
	std::vector<std::uint32_t> divisors;
	Stage(std::uint32_t n, std::span<const std::size_t> primes) {
		std::vector<std::uint64_t> marked(std::size_t(n) / 64 + 1);
		auto visit = [&](auto &&self, std::uint32_t value, std::size_t first) -> void {
			std::uint32_t limit = n / value;
			for (std::size_t j = first; j < primes.size() && primes[j] <= limit; ++j) {
				std::uint32_t next = value * std::uint32_t(primes[j]);
				marked[next >> 6] |= std::uint64_t(1) << (next & 63);
				self(self, next, j);
			}
		};
		visit(visit, 1, 0);
		for (std::size_t i = 0; i < marked.size(); ++i)
			for (auto bits = marked[i]; bits; bits &= bits - 1)
				divisors.push_back(std::uint32_t(i * 64 + std::countr_zero(bits)));
	}
};

inline std::vector<Stage> make_plan(std::uint32_t n, const std::vector<std::size_t> &primes, std::size_t &small) {
	auto first = std::upper_bound(primes.begin(), primes.end(), 31);
	small = std::size_t(first - primes.begin());
	std::vector<Stage> stages;
	auto append = [&](std::size_t bound) {
		auto last = std::upper_bound(first, primes.end(), bound);
		if (first != last) stages.emplace_back(n, std::span(first, last));
		first = last;
	};
	std::uint64_t bound = 199;
	append(bound);
	while (bound * bound <= (n - 1) / (bound * bound)) {
		bound *= bound;
		append(std::size_t(bound));
	}
	if (first != primes.end()) stages.emplace_back(n, std::span(first, primes.end()));
	return stages;
}

template <bool inverse, bool multiples, class F>
void for_each(std::size_t n, const std::vector<std::size_t> &ps, std::size_t count, F update) {
	if (!n) return;
	for (std::size_t j = 0; j < count; ++j) {
		const std::size_t p = ps[j], end = (n - 1) / p;
		auto apply = [&](std::size_t i) { update(multiples ? i : i * p, multiples ? i * p : i); };
		if constexpr (inverse == multiples) {
			std::size_t i = 1;
			for (; i + 3 <= end; i += 4) apply(i), apply(i + 1), apply(i + 2), apply(i + 3);
			for (; i <= end; ++i) apply(i);
		} else {
			std::size_t i = end;
			for (; i >= 4; i -= 4) apply(i), apply(i - 1), apply(i - 2), apply(i - 3);
			for (; i; --i) apply(i);
		}
	}
}

}

template <bool inverse = false, bool multiples = false, class T>
void divisor_zeta(std::vector<T> &a, const std::vector<std::size_t> &ps) {
	divisor_zeta_detail::for_each<inverse, multiples>(a.size(), ps, ps.size(), [&](std::size_t to, std::size_t from) {
		if constexpr (inverse) a[to] -= a[from];
		else a[to] += a[from];
	});
}

template <bool inverse = false, bool multiples = false, class T>
void divisor_zeta(std::vector<T> &a) { divisor_zeta<inverse, multiples>(a, prime_sieve(a.size())); }

#endif
