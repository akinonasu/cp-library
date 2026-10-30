#ifndef PERMUTATION_RANK_HPP
#define PERMUTATION_RANK_HPP

#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(__BMI2__) && defined(__x86_64__)
#include <immintrin.h>
#endif

namespace permutation_order_detail {
inline constexpr auto factorial = [] {
	std::array<std::uint64_t, 21> result{};
	result[0] = 1;
	for (int i = 1; i <= 20; ++i) result[i] = result[i - 1] * i;
	return result;
}();
class Available {
	std::vector<int> tree;
  public:
	explicit Available(int n) : tree(std::size_t(n) + 1) {
		for (std::size_t i = 1; i < tree.size(); ++i) tree[i] = int(i & -i);
	}
	int prefix(int r) const {
		int result = 0;
		for (; r; r &= r - 1) result += tree[r];
		return result;
	}
	void erase(int p) {
		for (std::size_t i = std::size_t(p) + 1; i < tree.size(); i += i & -i) --tree[i];
	}
	int select(int k) const {
		std::size_t p = 0;
		for (std::size_t step = std::bit_floor(tree.size() - 1); step; step >>= 1)
			if (p + step < tree.size() && tree[p + step] <= k) {
				p += step;
				k -= tree[p];
			}
		return int(p);
	}
};

inline void restore(std::vector<int> &result) {
	int n = int(result.size()), prefix = 0;
	while (prefix < n && !result[prefix]) result[prefix] = prefix, ++prefix;
	std::uint64_t available = (std::uint64_t(1) << (n - prefix)) - 1;
	for (int &digit : std::span(result).subspan(prefix)) {
#if defined(__BMI2__) && defined(__x86_64__)
		digit = std::countr_zero(_pdep_u64(std::uint64_t(1) << digit, available));
#else
		std::uint64_t bits = available;
		for (int k = digit; k; --k) bits &= bits - 1;
		digit = std::countr_zero(bits);
#endif
		available &= ~(std::uint64_t(1) << digit);
		digit += prefix;
	}
}
}

template <class Rank = std::uint64_t>
Rank permutation_rank(std::span<const int> permutation) {
	static_assert(std::numeric_limits<Rank>::is_integer || requires { Rank::mod(); });
	if (permutation.size() > std::size_t(std::numeric_limits<int>::max())) throw std::length_error("permutation_rank");
	int n = int(permutation.size());
	Rank result = 0;
	auto append = [&](int digit, int base) {
		if constexpr (std::numeric_limits<Rank>::is_integer && std::numeric_limits<Rank>::is_bounded) {
			const Rank maximum = std::numeric_limits<Rank>::max();
			if constexpr (std::numeric_limits<Rank>::digits < std::numeric_limits<int>::digits) {
				if (digit > int(maximum) || (result != 0 && base > int(maximum))) throw std::overflow_error("permutation_rank");
				if (base > int(maximum)) { result = Rank(digit); return; }
			}
			if (result != 0 && result > (maximum - digit) / base) throw std::overflow_error("permutation_rank");
		}
		result *= base;
		result += digit;
	};
	if (n <= 64) {
		std::uint64_t remaining = n == 64 ? ~std::uint64_t(0) : (std::uint64_t(1) << n) - 1;
		std::uint64_t seen = 0;
		for (int x : permutation) {
			if (x < 0 || x >= n || (seen & (std::uint64_t(1) << x))) throw std::invalid_argument("permutation_rank");
			seen |= std::uint64_t(1) << x;
		}
		for (int i = 0; i < n; ++i) {
			int x = permutation[i];
			append(std::popcount(remaining & ((std::uint64_t(1) << x) - 1)), n - i);
			remaining ^= std::uint64_t(1) << x;
		}
	} else {
		permutation_order_detail::Available remaining(n);
		std::vector<bool> seen(n);
		for (int x : permutation) {
			if (x < 0 || x >= n || seen[x]) throw std::invalid_argument("permutation_rank");
			seen[x] = true;
		}
		for (int i = 0; i < n; ++i) {
			int x = permutation[i];
			append(remaining.prefix(x), n - i);
			remaining.erase(x);
		}
	}
	return result;
}

template <class Rank>
std::optional<std::vector<int>> permutation_unrank(int n, Rank rank) {
	static_assert(std::numeric_limits<Rank>::is_integer);
	if (n < 0) throw std::invalid_argument("permutation_unrank");
	if (rank < 0) return std::nullopt;
	if constexpr (std::is_integral_v<Rank> && sizeof(Rank) <= 8) {
		if (n <= 20 && std::uint64_t(rank) >= permutation_order_detail::factorial[n]) return std::nullopt;
		if (!n) return std::vector<int>{};
	}
	using Work = std::conditional_t<std::numeric_limits<Rank>::is_bounded &&
		(std::numeric_limits<Rank>::digits < std::numeric_limits<int>::digits), int, Rank>;
	Work remaining = static_cast<Work>(std::move(rank));
	std::vector<int> result(n);
	for (std::int64_t base = 2; base <= n && remaining != 0; ++base) {
		result[n - base] = int(remaining % base);
		remaining /= base;
	}
	if (remaining != 0) return std::nullopt;
	if constexpr (std::is_integral_v<Rank> && sizeof(Rank) <= 8) {
		permutation_order_detail::restore(result);
		return result;
	}
	permutation_order_detail::Available available(n);
	for (int &digit : result) {
		digit = available.select(digit);
		available.erase(digit);
	}
	return result;
}

#endif
