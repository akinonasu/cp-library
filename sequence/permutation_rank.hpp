#ifndef PERMUTATION_RANK_HPP
#define PERMUTATION_RANK_HPP

#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace permutation_order_detail {
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
	using Work = std::conditional_t<std::numeric_limits<Rank>::is_bounded &&
		(std::numeric_limits<Rank>::digits < std::numeric_limits<int>::digits), int, Rank>;
	Work remaining = static_cast<Work>(std::move(rank));
	std::vector<int> result(n);
	for (std::int64_t base = 2; base <= n && remaining != 0; ++base) {
		result[n - base] = int(remaining % base);
		remaining /= base;
	}
	if (remaining != 0) return std::nullopt;
	permutation_order_detail::Available available(n);
	for (int &digit : result) {
		digit = available.select(digit);
		available.erase(digit);
	}
	return result;
}

#endif
