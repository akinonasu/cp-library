#line 1 "test/multiplicative_prefix_sum_totient.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_totient_function"
#line 1 "math/number_theory/multiplicative_prefix_sum.hpp"



#line 1 "components/floor_quotients.hpp"



#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

class FloorQuotients {
	std::uint64_t n_, root_ = 0;
	std::size_t large_ = 0;
	std::vector<std::uint64_t> values_;

  public:
	explicit FloorQuotients(std::uint64_t n = 0) : n_(n) {
		if (!n) return;
		root_ = std::sqrt(static_cast<long double>(n));
		while (__uint128_t(root_) * root_ > n) --root_;
		while (__uint128_t(root_ + 1) * (root_ + 1) <= n) ++root_;
		large_ = n / (root_ + 1);
		values_.reserve(large_ + root_);
		for (std::size_t i = 1; i <= large_; ++i) values_.push_back(n / i);
		for (auto x = root_; x; --x) values_.push_back(x);
	}
	FloorQuotients(const FloorQuotients&) = default;
	FloorQuotients& operator=(const FloorQuotients&) = default;
	FloorQuotients(FloorQuotients&& other) noexcept : FloorQuotients() { swap(other); }
	FloorQuotients& operator=(FloorQuotients&& other) noexcept {
		if (this != &other) { FloorQuotients empty; swap(empty); swap(other); }
		return *this;
	}
	void swap(FloorQuotients& other) noexcept {
		std::swap(n_, other.n_), std::swap(root_, other.root_), std::swap(large_, other.large_);
		values_.swap(other.values_);
	}
	std::uint64_t max_n() const { return n_; }
	std::uint64_t root() const { return root_; }
	std::size_t size() const { return values_.size(); }
	std::size_t large_size() const { return large_; }
	const std::vector<std::uint64_t>& values() const { return values_; }
	bool contains(std::uint64_t x) const { return x && x <= n_ && (x <= root_ || n_ / (n_ / x) == x); }
	std::size_t index(std::uint64_t x) const {
		assert(x && x <= n_);
		return x <= root_ ? size() - x : n_ / x - 1;
	}
	std::size_t divided_index(std::size_t i, std::uint64_t divisor) const {
		assert(i < size() && divisor && divisor <= values_[i]);
		if (i >= large_) return size() - values_[i] / divisor;
		const auto d = (i + 1) * divisor;
		return d <= large_ ? d - 1 : size() - n_ / d;
	}
};


#line 1 "components/multiplicative_sum_backend.hpp"



#line 6 "components/multiplicative_sum_backend.hpp"
#include <algorithm>
#line 10 "components/multiplicative_sum_backend.hpp"

namespace multiplicative_sum_detail {
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

template<class T> struct LinearPrime {
	T weighted;
	explicit LinearPrime(const LinearWeights<T>& value) : weighted(value.weighted) {}
	explicit LinearPrime(T value) : weighted(value) {}
	operator LinearWeights<T>() const { return LinearWeights<T>(T(1), weighted); }
	friend LinearPrime operator*(const LinearPrime& a, const LinearPrime& b) { return LinearPrime(a.weighted * b.weighted); }
	friend LinearWeights<T> operator*(const LinearPrime& a, const LinearWeights<T>& b) { return LinearWeights<T>(b.count, a.weighted * b.weighted); }
	friend LinearWeights<T> operator*(const LinearWeights<T>& a, const LinearPrime& b) { return LinearWeights<T>(a.count, a.weighted * b.weighted); }
};
template<class T> struct PrimeValue { using type = T; };
template<class T> struct PrimeValue<LinearWeights<T>> { using type = LinearPrime<T>; };

template<class T> class Backend {
	using U = std::uint64_t;
	U n, root, large;
	std::size_t length;
	std::vector<U> primes;
	std::vector<typename PrimeValue<T>::type> first;
	std::vector<T> corrections;
	std::vector<std::size_t> offsets;
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
	T powerful(const std::vector<T>& base, std::size_t begin, U product, U limit) const {
		T answer = base[product <= large ? big(product) : small(limit)];
		for (std::size_t i = begin; i < first.size(); ++i) {
			const U p = primes[i];
			U power = p * p;
			if (power > limit) break;
			U next_limit = limit / power;
			std::size_t correction = offsets[i];
			for (;;) {
				const T weight = corrections[correction++];
				if constexpr (requires { weight == T(0); }) if (weight == T(0)) {
					if (next_limit < p) break;
					power *= p;
					next_limit /= p;
					continue;
				}
				if (i + 1 < first.size() && primes[i + 1] * primes[i + 1] <= next_limit)
					answer += weight * powerful(base, i + 1, product * power, next_limit);
				else answer += weight * base[product * power <= large ? big(product * power) : small(next_limit)];
				if (next_limit < p) break;
				power *= p;
				next_limit /= p;
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
		first.reserve(primes.size() - 1);
		corrections.clear();
		corrections.reserve(primes.size());
		offsets.clear();
		offsets.reserve(primes.size() - 1);
		for (std::size_t i = 0; i + 1 < primes.size(); ++i) {
			const U p = primes[i];
			first.emplace_back(f(p, 1, p));
			offsets.push_back(corrections.size());
			T previous(0);
			U power = p * p, limit = n / power;
			for (unsigned e = 2;; ++e) {
				previous = f(p, e, power) - first[i] * previous;
				corrections.push_back(previous);
				if (limit < p) break;
				power *= p;
				limit /= p;
			}
		}
		auto out = build<true>(&prefix, inv3);
		return powerful(out, 0, 1, n);
	}
	template<class Prefix> std::vector<T> prime_sum(Prefix& integer_prefix, T inv3) {
		if (!n) return {};
		first.clear();
		for (std::size_t i = 0; i + 1 < primes.size(); ++i) first.emplace_back(integer_prefix(primes[i]) - integer_prefix(primes[i] - 1));
		const auto base = build<false>(nullptr, inv3);
		std::vector<T> result(length, T(0));
		for (U i = 1; i <= large; ++i) result[i - 1] = integer_prefix(n / i) - base[big(i)];
		for (std::size_t j = 0; j < first.size() && primes[j] <= large; ++j) {
			const U p = primes[j], end = large / p;
			for (U i = 1; i <= end; ++i) result[i - 1] -= first[j] * result[i * p - 1];
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
}


#line 1 "math/number_theory/prime_sieve.hpp"



#line 5 "math/number_theory/prime_sieve.hpp"
#include <array>
#include <bit>
#line 10 "math/number_theory/prime_sieve.hpp"
#include <cstring>
#line 12 "math/number_theory/prime_sieve.hpp"

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n <= 2) return result;
	if (n >= 1024) result.reserve(std::size_t(double(n) / (std::log(double(n)) - 1.1)));
	result.push_back(2);
	if (n <= 65537) {
		const std::size_t size = n / 2;
		std::vector<std::uint8_t> prime(size, 1);
		prime[0] = 0;
		for (std::size_t p = 3; p <= (n - 1) / p; p += 2) {
			if (!prime[p / 2]) continue;
			for (std::size_t j = p * p / 2; j < size; j += p) prime[j] = 0;
		}
		std::size_t i = 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		for (; i + 8 <= size; i += 8) {
			std::uint64_t word;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			while (word) {
				result.push_back(2 * (i + (std::countr_zero(word) >> 3)) + 1);
				word &= word - 1;
			}
		}
#endif
		for (; i < size; ++i) if (prime[i]) result.push_back(2 * i + 1);
		return result;
	}
	result.push_back(3), result.push_back(5);
	constexpr std::array<unsigned, 8> residues{1, 7, 11, 13, 17, 19, 23, 29};
	constexpr auto index = [&] {
		std::array<unsigned, 30> index{};
		for (unsigned i = 0; i < 8; ++i) index[residues[i]] = i;
		return index;
	}();
	constexpr std::size_t period = 7 * 11 * 13, block_size = period * 32;
	static const auto pattern = [&] {
		std::array<std::uint8_t, period> pattern{};
		for (std::size_t i = 0; i < period; ++i)
			for (unsigned d = 0; d < 8; ++d) {
				const auto x = 30 * i + residues[d];
				pattern[i] |= std::uint8_t(bool(x % 7 && x % 11 && x % 13) << d);
			}
		return pattern;
	}();
	std::size_t root = std::sqrt(static_cast<long double>(n - 1));
	while (root > (n - 1) / root) --root;
	while (root + 1 <= (n - 1) / (root + 1)) ++root;
	const auto primes = prime_sieve(root + 1);
	struct State {
		std::size_t p;
		std::array<std::size_t, 8> next;
		std::array<std::uint8_t, 8> mask;
	};
	std::vector<State> state;
	state.reserve(primes.size());
	for (const auto p : primes) {
		if (p < 17) continue;
		State current;
		current.p = p;
		for (unsigned d = 0; d < 8; ++d) {
			const std::size_t k = p + (residues[d] + 30 - p % 30) % 30;
			const __uint128_t x = __uint128_t(p) * k;
			current.next[d] = x / 30;
			current.mask[d] = std::uint8_t(~(1u << index[x % 30]));
		}
		state.push_back(current);
	}
	const std::size_t size = (n - 1) / 30 + 1;
	std::vector<std::uint8_t> prime(block_size);
	constexpr auto offsets = [&] {
		std::array<unsigned, 64> offsets{};
		for (unsigned i = 0; i < 64; ++i) offsets[i] = 30 * (i / 8) + residues[i % 8];
		return offsets;
	}();
	for (std::size_t base = 0; base < size;) {
		const std::size_t length = std::min(block_size, size - base);
		for (std::size_t i = 0; i < length; i += period)
			std::copy_n(pattern.data(), std::min(period, length - i), prime.data() + i);
		if (base == 0) {
			prime[0] &= ~1u;
			prime[0] |= (1u << 1) | (1u << 2) | (1u << 3);
		}
		for (auto &current : state) {
			const auto p = current.p;
			for (unsigned d = 0; d < 8; ++d) {
				std::size_t j = current.next[d];
				const auto mask = current.mask[d];
				for (; j + 3 * p < length; j += 4 * p) {
					prime[j] &= mask;
					prime[j + p] &= mask;
					prime[j + 2 * p] &= mask;
					prime[j + 3 * p] &= mask;
				}
				for (; j < length; j += p) prime[j] &= mask;
				current.next[d] = j - length;
			}
		}
		if (base + length == size) {
			const auto remaining = n - 30 * (size - 1);
			for (unsigned d = 0; d < 8; ++d) if (residues[d] >= remaining) prime[length - 1] &= ~(1u << d);
		}
		std::fill(prime.begin() + length, prime.begin() + ((length + 7) / 8 * 8), 0);
		for (std::size_t i = 0; i < length; i += 8) {
			std::uint64_t word = 0;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			if constexpr (std::endian::native == std::endian::big) word = std::byteswap(word);
			else if constexpr (std::endian::native != std::endian::little) {
				word = 0;
				for (unsigned d = 0; d < 8; ++d) word |= std::uint64_t(prime[i + d]) << (8 * d);
			}
			while (word) {
				const auto x = 30 * (base + i) + offsets[std::countr_zero(word)];
				result.push_back(x);
				word &= word - 1;
			}
		}
		base += length;
	}
	return result;
}


#line 13 "math/number_theory/multiplicative_prefix_sum.hpp"

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


#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#include <type_traits>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct Mint {
	using mint = Mint;
	unsigned v = 0;
	static constexpr unsigned mod() { return 998244353; }
	[[gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[gnu::always_inline]] unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	[[gnu::always_inline]] Mint operator+() const { return *this; }
	[[gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 13 "components/fast_io.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 5 "test/multiplicative_prefix_sum_totient.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint64_t n = 0;
	input.read(n);
	MultiplicativePrefixSum<mint> sieve(n);
	auto sum = sieve.prime_sum_linear(mint(-1), mint(1));
	auto answer = sieve.sum(sum, [](std::uint64_t p, unsigned, std::uint64_t power) { return mint(power - power / p); });
	output.println(answer.val());
}
