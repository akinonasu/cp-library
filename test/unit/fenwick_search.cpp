#include "data_structure/fenwick_tree.hpp"
#include "math/modint.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <random>
#include <type_traits>
#include <vector>

void require(bool condition) { if (!condition) std::abort(); }

template<class T, bool Padded>
void search(const std::vector<T> &values) {
	const FenwickTree<T, Padded> tree(values);
	T total{};
	for (const T &x : values) total += x;
	for (int x = -2; x <= int(total) + 2; ++x) {
		T threshold = T(x);
		if constexpr (!std::numeric_limits<T>::is_signed) if (x < 0) continue;
		int lower = int(values.size()), upper = int(values.size());
		T prefix{};
		for (int p = 0; p < int(values.size()); ++p) {
			prefix += values[p];
			if (lower == int(values.size()) && !(prefix < threshold)) lower = p;
			if (upper == int(values.size()) && threshold < prefix) upper = p;
		}
		require(tree.lower_bound(threshold) == lower);
		require(tree.upper_bound(threshold) == upper);
	}
	if (!values.empty()) {
		require(tree.lower_bound(std::numeric_limits<T>::max()) == int(values.size()));
		require(tree.upper_bound(std::numeric_limits<T>::max()) == int(values.size()));
	}
}

template<bool Padded>
void exhaustive() {
	for (int n = 0; n <= 6; ++n) {
		unsigned patterns = 1u << (2 * n);
		for (unsigned pattern = 0; pattern < patterns; ++pattern) {
			std::vector<int> values(n);
			for (int p = 0; p < n; ++p) values[p] = (pattern >> (2 * p)) & 3;
			search<int, Padded>(values);
			search<unsigned, Padded>(std::vector<unsigned>(values.begin(), values.end()));
		}
	}
}

template<class T, bool Padded>
void random_updates() {
	std::mt19937_64 rng(427109);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 1023, 1024, 1025, 2047, 2048, 2049}) {
		std::vector<T> values(n);
		for (T &x : values) x = T(rng() % 7);
		FenwickTree<T, Padded> tree(values);
		for (int repeat = 0; repeat < 300; ++repeat) {
			if (n) {
				int p = int(rng() % n);
				T x = T(rng() % 7);
				if (repeat & 1) tree.set(p, x);
				else tree.add(p, T(x - values[p]));
				values[p] = x;
				require(tree.get(p) == values[p]);
			}
			std::vector<T> prefix(n);
			std::partial_sum(values.begin(), values.end(), prefix.begin());
			T total = n ? prefix.back() : T{};
			T threshold = T(rng() % (std::uint64_t(total) + 3));
			const auto &constant = tree;
			require(constant.lower_bound(threshold) == std::lower_bound(prefix.begin(), prefix.end(), threshold) - prefix.begin());
			require(constant.upper_bound(threshold) == std::upper_bound(prefix.begin(), prefix.end(), threshold) - prefix.begin());
			require(constant.sum(n) == total);
			if (n) {
				int l = int(rng() % n), r = int(rng() % n);
				if (l > r) std::swap(l, r);
				require(constant.sum(l, r) == std::accumulate(values.begin() + l, values.begin() + r, T{}));
			}
		}
	}
}

template<class T, bool Padded>
void set_extremes() {
	using U = typename fenwick_tree_detail::Storage<T>::type;
	const T lo = std::numeric_limits<T>::lowest(), hi = std::numeric_limits<T>::max();
	std::vector<T> values{lo, hi, T(0), T(1), T(-1)};
	FenwickTree<T, Padded> tree(values);
	for (int p = 0; p < int(values.size()); ++p) {
		for (T x : {hi, lo, T(0), T(-1), T(1), hi, lo}) {
			tree.set(p, x);
			values[p] = x;
			for (int i = 0; i < int(values.size()); ++i) require(tree.get(i) == values[i]);
			for (int l = 0; l <= int(values.size()); ++l) {
				U total{};
				for (int r = l; r <= int(values.size()); ++r) {
					require(tree.sum(l, r) == T(total));
					if (r < int(values.size())) total += U(values[r]);
				}
			}
		}
	}
	FenwickTree<T, Padded> limits(std::vector<T>{T(0), hi, T(0)});
	require(limits.lower_bound(hi) == 1);
	require(limits.upper_bound(hi) == 3);
	if constexpr (std::numeric_limits<T>::is_signed) {
		require(limits.lower_bound(lo) == 0);
		require(limits.upper_bound(lo) == 0);
	}
}

struct Additive {
	int value = 0;
	Additive() = default;
	explicit Additive(int value) : value(value) {}
	Additive &operator+=(const Additive &other) { value += other.value; return *this; }
	Additive &operator-=(const Additive &other) { value -= other.value; return *this; }
	bool operator==(const Additive &) const = default;
};

template<bool Padded>
void generic() {
	FenwickTree<Additive, Padded> tree(std::vector<Additive>{Additive(1), Additive(2)});
	tree.set(0, Additive(-3));
	require(tree.get(0) == Additive(-3));
	require(tree.sum(2) == Additive(-1));
	using Mint = StaticModint<998244353>;
	FenwickTree<Mint, Padded> modular(std::vector<Mint>{1, 2, 3});
	modular.set(1, -1);
	require(modular.get(1) == Mint(-1));
	require(modular.sum(3) == Mint(3));
}

int main() {
	exhaustive<true>();
	exhaustive<false>();
	random_updates<int, true>();
	random_updates<int, false>();
	random_updates<unsigned, true>();
	random_updates<unsigned, false>();
	random_updates<std::int64_t, true>();
	random_updates<std::int64_t, false>();
	set_extremes<std::int8_t, true>();
	set_extremes<std::int8_t, false>();
	set_extremes<std::uint8_t, true>();
	set_extremes<std::uint8_t, false>();
	set_extremes<std::int64_t, true>();
	set_extremes<std::int64_t, false>();
	set_extremes<std::uint64_t, true>();
	set_extremes<std::uint64_t, false>();
#if defined(__SIZEOF_INT128__)
	random_updates<__int128_t, true>();
	random_updates<__int128_t, false>();
	set_extremes<__int128_t, true>();
	set_extremes<__int128_t, false>();
	set_extremes<__uint128_t, true>();
	set_extremes<__uint128_t, false>();
#endif
	generic<true>();
	generic<false>();
}
