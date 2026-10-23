#ifndef MULTIPLICATIVE_SUM_BACKEND_HPP
#define MULTIPLICATIVE_SUM_BACKEND_HPP

#include "floor_quotients.hpp"
#include <cstddef>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace multiplicative_sum_detail {
template<class T> class Backend {
	using U = std::uint64_t;
	U n, root, large;
	std::size_t length;
	std::vector<U> primes;
	std::vector<T> first;
	static U square_root(U value) {
		U result = std::sqrt(static_cast<long double>(value));
		while (__uint128_t(result) * result > value) --result;
		while (__uint128_t(result + 1) * (result + 1) <= value) ++result;
		return result;
	}
	static U cube_root(U value) {
		U result = std::cbrt(static_cast<long double>(value));
		while (__uint128_t(result) * result * result > value) --result;
		while (__uint128_t(result + 1) * (result + 1) * (result + 1) <= value) ++result;
		return result;
	}
	static U fifth_root(U value) {
		U result = std::pow(static_cast<long double>(value), 0.2L);
		auto power = [](U x) { const __uint128_t square = __uint128_t(x) * x; return square * square * x; };
		while (power(result) > value) --result;
		while (power(result + 1) <= value) ++result;
		return result;
	}
	std::size_t small(U x) const { return x - 1; }
	std::size_t big(U d) const { return length - d; }
	std::size_t pi(U x) const { return std::upper_bound(primes.begin(), primes.end() - 1, x) - primes.begin(); }
	struct Tree {
		std::vector<T> data;
		explicit Tree(const std::vector<T>& a, std::size_t size) : data(size + 1, T(0)) {
			T previous(0);
			for (std::size_t i = 1; i <= size; ++i) { data[i] = a[i - 1] - previous; previous = a[i - 1]; }
			for (std::size_t i = 1; i <= size; ++i) if (i + (i & -i) <= size) data[i + (i & -i)] += data[i];
		}
		void add(std::size_t i, T value) { for (++i; i < data.size(); i += i & -i) data[i] += value; }
		T sum(std::size_t i) const { T result(0); for (; i; i -= i & -i) result += data[i]; return result; }
		void write(std::vector<T>& a) {
			for (std::size_t i = data.size() - 1; i; --i) if (i + (i & -i) < data.size()) data[i + (i & -i)] -= data[i];
			T result(0); for (std::size_t i = 1; i < data.size(); ++i) { result += data[i]; a[i - 1] = result; }
		}
	};
	template<class F> T powerful(const std::vector<T>& base, std::size_t begin, U product, U limit, T coefficient, F& f) const {
		T answer = coefficient * base[product <= large ? big(product) : small(limit)];
		for (std::size_t i = begin; i < first.size(); ++i) {
			const U p = primes[i];
			if (p > limit / p) break;
			U power = p * p, next_limit = limit / p;
			T previous(0);
			for (unsigned e = 2;; ++e) {
				next_limit /= p;
				previous = f(p, e, power) - first[i] * previous;
				answer += powerful(base, i + 1, product * power, next_limit, coefficient * previous, f);
				if (power > limit / p) break;
				power *= p;
			}
		}
		return answer;
	}
	template<bool SquareFree> void enumerate(Tree& tree, std::size_t begin, U product, T coefficient, U bound, unsigned depth = 0) const {
		if (depth == 3) return;
		for (std::size_t i = begin; i < first.size() && primes[i] <= bound / product; ++i) {
			const U next = product * primes[i];
			const T value = coefficient * first[i];
			tree.add(next <= root ? small(next) : big(n / next), value);
			enumerate<SquareFree>(tree, i + SquareFree, next, value, bound, depth + 1);
		}
	}

	template<bool SquareFree> std::vector<T> build(const std::vector<T>* prefix, T inv3) const {
		const T before = SquareFree ? (*prefix)[large] : T(0);
		auto prime_large = [&](U d) {
			if constexpr (SquareFree) return (*prefix)[d - 1] - before;
			else return T(0);
		};
		const U tree_large = std::min<U>(std::floor(std::pow(n, 3.0 / 8)), large), tree_bound = n / (tree_large + 1);
		const U min_cut = std::max<U>(tree_bound / (root + 1), fifth_root(tree_bound));
		const U cut = std::max<U>(square_root(square_root(root)), min_cut);
		const auto pi_cut = pi(cut), pi_fourth = pi(square_root(root));
		std::vector<T> out(length, T(0));
		out[0] = T(1);
		for (std::size_t i = pi_cut; i < first.size(); ++i) out[small(primes[i])] = first[i];
		for (U i = 1; i < root; ++i) out[i] += out[i - 1];
		for (std::size_t i = pi_fourth, r = first.size(); i < first.size(); ++i) {
			const U p = primes[i], limit = n / p;
			while (r && U(r - 1) * primes[r - 1] > limit) --r;
			for (std::size_t j = i + SquareFree; j < r; ++j) out[big(limit / primes[j])] += first[i] * first[j];
			const U bound = std::min(primes[std::max(r, i + SquareFree)], root + 1);
			if (p <= large) out[big(large / p)] += first[i] * out[small(root)];
			if (bound <= limit) out[big(limit / bound)] -= first[i] * out[small(bound - 1)];
		}
		for (U i = large; i-- > 1;) out[big(i)] += out[big(i + 1)];
		for (std::size_t i = pi_fourth, r = first.size(); i < first.size(); ++i) {
			const U p = primes[i], limit = n / p;
			while (r && U(r - 1) * primes[r - 1] > limit) --r;
			const U bound = std::min(primes[std::max(r, i + SquareFree)], root + 1), split = large / p, end = limit / bound;
			for (U j = end; j > split; --j) out[big(j)] += first[i] * out[small(limit / j)];
			for (U j = split; j; --j) out[big(j)] += first[i] * prime_large(j * p);
		}
		const U bound_prime = primes[pi_fourth], bound_z = n / bound_prime / bound_prime / bound_prime;
		for (U i = 1; i <= bound_z; ++i) {
			const U limit = n / i;
			const auto bound1 = pi(large / i), bound2 = pi(square_root(limit / bound_prime)), bound3 = pi(cube_root(limit));
			T value(0);
			for (std::size_t j = pi_fourth; j < bound1; ++j) value += first[j] * out[big(i * primes[j])];
			for (std::size_t j = pi_fourth; j < bound2; ++j) {
				const T correction = first[j] * first[j] * (out[small(limit / primes[j] / primes[j])] - out[small(bound_prime - 1)]);
				if constexpr (SquareFree) value -= correction;
				else value += correction;
			}
			for (std::size_t j = pi_fourth; j < bound3; ++j) value += first[j] * first[j] * first[j];
			out[big(i)] += value * inv3;
		}
		for (U i = 1; i <= large; ++i) out[big(i)] += prime_large(i) + out[small(root)];
		const std::size_t tree_length = length - tree_large;
		Tree tree(out, tree_length);
		for (std::size_t i = pi_fourth; i-- > pi_cut;) {
			const U p = primes[i], limit = n / p, limit2 = limit / p;
			const U split = std::min(large / p, tree_large), end = std::min(limit2, tree_large);
			const U index2 = p * p <= root ? small(p * p) : std::min(big(limit2), tree_length);
			auto access = [&](U index) { return index2 <= index && index < tree_length ? tree.sum(index + 1) : out[index]; };
			if constexpr (SquareFree) {
				const T previous = out[small(p)];
				for (U j = 1; j <= split; ++j) out[big(j)] += first[i] * (access(big(j * p)) - previous);
				for (U j = split + 1; j <= end; ++j) out[big(j)] += first[i] * (access(small(limit / j)) - previous);
				enumerate<true>(tree, i + 1, p, first[i], tree_bound);
			} else {
				const T previous = out[small(p - 1)];
				enumerate<false>(tree, i, p, first[i], tree_bound);
				for (U j = end; j > split; --j) out[big(j)] += first[i] * (access(small(limit / j)) - previous);
				for (U j = split; j; --j) out[big(j)] += first[i] * (access(big(j * p)) - previous);
			}
		}
		tree.write(out);
		for (std::size_t i = pi_cut; i--;) {
			const U p = primes[i], limit = n / p, split = large / p;
			if constexpr (SquareFree) {
				for (U j = 1; j <= split; ++j) out[big(j)] += first[i] * out[big(j * p)];
				for (U j = split + 1; j <= large; ++j) out[big(j)] += first[i] * out[small(limit / j)];
				for (U j = root; j >= p; --j) out[small(j)] += first[i] * out[small(j / p)];
			} else {
				for (U j = p; j <= root; ++j) out[small(j)] += first[i] * out[small(j / p)];
				for (U j = large; j > split; --j) out[big(j)] += first[i] * out[small(limit / j)];
				for (U j = split; j; --j) out[big(j)] += first[i] * out[big(j * p)];
			}
		}
		return out;
	}
  public:
	Backend(const FloorQuotients& quotients, const std::vector<std::size_t>& prime_list) : n(quotients.max_n()), root(quotients.root()), large(quotients.large_size()), length(quotients.size()), primes(prime_list.begin(), prime_list.end()) { primes.push_back(root + 1); }
	template<class F> T sum(const std::vector<T>& prefix, F& f, T inv3) {
		if (!n) return T(0);
		first.clear();
		for (std::size_t i = 0; i + 1 < primes.size(); ++i) first.push_back(f(primes[i], 1, primes[i]));
		auto out = build<true>(&prefix, inv3);
		return powerful(out, 0, 1, n, T(1), f);
	}
	template<class Prefix> std::vector<T> prime_sum(Prefix& integer_prefix, T inv3) {
		if (!n) return {};
		first.clear();
		for (std::size_t i = 0; i + 1 < primes.size(); ++i) first.push_back(integer_prefix(primes[i]) - integer_prefix(primes[i] - 1));
		const auto base = build<false>(nullptr, inv3);
		std::vector<T> result(length, T(0));
		for (U i = large; i; --i) {
			T answer = integer_prefix(n / i) - base[big(i)], previous = base[0];
			for (U j = 2; j <= large / i; ++j) {
				const T current = base[small(j)];
				answer -= result[i * j - 1] * (current - previous);
				previous = current;
			}
			result[i - 1] = answer;
		}
		T cumulative(0);
		std::size_t j = 0;
		for (U i = 1; i <= root; ++i) {
			if (i == primes[j]) cumulative += first[j++];
			result[length - i] = cumulative;
		}
		for (U i = 0; i < large; ++i) result[i] += cumulative;
		return result;
	}
};

template<class T> struct LinearWeights {
	T count, weighted;
	template<class I> explicit LinearWeights(I value) : count(T(value)), weighted(T(value)) {}
	LinearWeights(T count_, T weighted_) : count(count_), weighted(weighted_) {}
	LinearWeights& operator+=(const LinearWeights& b) { count += b.count; weighted += b.weighted; return *this; }
	LinearWeights& operator-=(const LinearWeights& b) { count -= b.count; weighted -= b.weighted; return *this; }
	friend LinearWeights operator+(LinearWeights a, const LinearWeights& b) { return a += b; }
	friend LinearWeights operator-(LinearWeights a, const LinearWeights& b) { return a -= b; }
	friend LinearWeights operator*(const LinearWeights& a, const LinearWeights& b) { return LinearWeights(a.count * b.count, a.weighted * b.weighted); }
};
}

#endif
