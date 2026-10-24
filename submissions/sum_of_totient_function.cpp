#line 1 "test/multiplicative_prefix_sum_totient.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_totient_function"
#line 1 "math/number_theory/totient_sum.hpp"



#line 1 "math/number_theory/prime_sieve.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

inline std::vector<std::size_t> prime_sieve(std::size_t n);

template<class F> void for_each_prime(std::size_t n, F emit) {
	if (n <= 2) return;
	emit(std::size_t(2));
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
				emit(2 * (i + (std::countr_zero(word) >> 3)) + 1);
				word &= word - 1;
			}
		}
#endif
		for (; i < size; ++i) if (prime[i]) emit(2 * i + 1);
		return;
	}
	emit(3), emit(5);
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
				emit(x);
				word &= word - 1;
			}
		}
		base += length;
	}
}

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n >= 1024) result.reserve(std::size_t(double(n) / (std::log(double(n)) - 1.1)));
	for_each_prime(n, [&](std::size_t p) { result.push_back(p); });
	return result;
}


#line 1 "math/number_theory/kth_root.hpp"



#line 6 "math/number_theory/kth_root.hpp"
#include <limits>
#line 8 "math/number_theory/kth_root.hpp"
#include <cassert>
#line 11 "math/number_theory/kth_root.hpp"

namespace kth_root_detail {
inline constexpr auto thresholds = [] {
	std::array<std::array<std::uint64_t, 13>, 25> table{};
	for (unsigned k = 16; k <= 40; ++k) for (unsigned a = 3; a <= 15; ++a) {
		std::uint64_t p = 1;
		for (unsigned i = 0; i < k; ++i) {
			if (p > UINT64_MAX / a) { p = 0; break; }
			p *= a;
		}
		table[k - 16][a - 3] = p - 1;
	}
	return table;
}();

inline std::uint64_t cube(std::uint64_t n) {
	if constexpr (std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53 && sizeof(double) == sizeof(std::uint64_t)) {
		static constexpr auto inverse_roots = [] {
			std::array<double, 64> result{};
			for (int i = 0; i < 64; ++i) {
				double a = 1 + (i + 0.5) / 64, x = 1;
				for (int j = 0; j < 8; ++j) x += x * (1 - a * x * x * x) / 3;
				result[i] = x;
			}
			return result;
		}();
		double a = double(n);
		std::uint64_t bits = std::bit_cast<std::uint64_t>(a);
		unsigned exponent = unsigned(bits >> 52) - 1023;
		constexpr double factor[] = {1, 0.79370052598409973738, 0.62996052494743658238};
		double x = inverse_roots[(bits >> 46) & 63] * factor[exponent % 3]
			* std::bit_cast<double>(std::uint64_t(1023 - exponent / 3) << 52);
		{
			double h = 1 - a * x * x * x;
			x += x * h * (1.0 / 3 + h * (2.0 / 9 + h * (14.0 / 81)));
		}
		std::uint64_t r = std::min(std::uint64_t(1 / x), std::uint64_t(2642245));
		while (r * r * r > n) --r;
		while (r < 2642245 && (r + 1) * (r + 1) * (r + 1) <= n) ++r;
		return r;
	} else {
		std::uint64_t r = std::cbrt(double(n));
		while (__uint128_t(r) * r * r > n) --r;
		while (__uint128_t(r + 1) * (r + 1) * (r + 1) <= n) ++r;
		return r;
	}
}

template<unsigned K>
constexpr double power(double x) {
	double value = 1;
	for (unsigned k = K;; x *= x) {
		if (k & 1) value *= x;
		if (!(k >>= 1)) return value;
	}
}

template<unsigned K>
inline double estimate(std::uint64_t n) {
	if constexpr (std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53 && sizeof(double) == sizeof(std::uint64_t)) {
		static constexpr auto roots = [] {
			std::array<double, 64> values{};
			for (int i = 0; i < 64; ++i) {
				double a = 1 + (i + 0.5) / 64, x = 1;
				for (int j = 0; j < 10; ++j) x += x * (1 - a * power<K>(x)) / K;
				values[i] = x;
			}
			return values;
		}();
		static constexpr auto factors = [] {
			std::array<double, K> values{};
			double x = 1;
			for (int j = 0; j < 10; ++j) x += x * (1 - 2 * power<K>(x)) / K;
			values[0] = 1;
			for (unsigned j = 1; j < K; ++j) values[j] = values[j - 1] * x;
			return values;
		}();
		double a = double(n);
		auto bits = std::bit_cast<std::uint64_t>(a);
		unsigned exponent = unsigned(bits >> 52) - 1023;
		double x = roots[(bits >> 46) & 63] * factors[exponent % K]
			* std::bit_cast<double>(std::uint64_t(1023 - exponent / K) << 52);
		double h = 1 - a * power<K>(x);
		constexpr double c = 1.0 / K;
		x += x * h * (c + h * (c * (c + 1) / 2 + h * (c * (c + 1) * (c + 2) / 6)));
		return 1 / x;
	} else return std::pow(double(n), 1.0 / K);
}

template <unsigned K>
inline std::uint64_t small(std::uint64_t n) {
	auto leq = [n](std::uint64_t x) {
		std::uint64_t a = x, r = 1;
		for (unsigned k = K;;) {
			if ((k & 1) && __builtin_mul_overflow(r, a, &r)) return false;
			if (!(k >>= 1)) return r <= n;
			if (__builtin_mul_overflow(a, a, &a)) return false;
		}
	};
	std::uint64_t lo = 1, hi = std::uint64_t(1) << ((std::bit_width(n) - 1) / K + 1);
	double estimate;
	if constexpr (K == 4) estimate = std::sqrt(std::sqrt(double(n)));
	else if constexpr (K == 8) estimate = std::sqrt(std::sqrt(std::sqrt(double(n))));
	else estimate = kth_root_detail::estimate<K>(n);
	std::uint64_t r = std::uint64_t(estimate);
	if (r >= hi) r = hi - 1;
	if (r < lo) r = lo;
	if (leq(r)) {
		if (!leq(r + 1)) return r;
		lo = r + 1;
	} else {
		if (leq(r - 1)) return r - 1;
		hi = r - 1;
	}
	while (lo + 1 < hi) {
		std::uint64_t mid = (lo + hi) / 2;
		if (leq(mid)) lo = mid;
		else hi = mid;
	}
	return lo;
}
}

inline std::uint64_t kth_root(std::uint64_t n, std::uint64_t k) {
	assert(k > 0);
	if (k == 1 || n <= 1) return n;
	if (k >= unsigned(std::bit_width(n))) return 1;
	if (k == 2) {
		std::uint64_t r = std::sqrt(double(n));
		if (__uint128_t(r) * r > n) --r;
		if (__uint128_t(r + 1) * (r + 1) <= n) ++r;
		return r;
	}
	if (k >= 16) {
		if (k > 40) return 2;
		unsigned lo = 3, hi = 1U << (63 / k + 1);
		while (lo < hi) {
			unsigned mid = (lo + hi) / 2;
			if (n > kth_root_detail::thresholds[k - 16][mid - 3]) lo = mid + 1;
			else hi = mid;
		}
		return lo - 1;
	}
	switch (k) {
	case 3: return kth_root_detail::cube(n);
	case 4: return kth_root_detail::small<4>(n);
	case 5: return kth_root_detail::small<5>(n);
	case 6: return kth_root_detail::small<6>(n);
	case 7: return kth_root_detail::small<7>(n);
	case 8: return kth_root_detail::small<8>(n);
	case 9: return kth_root_detail::small<9>(n);
	case 10: return kth_root_detail::small<10>(n);
	case 11: return kth_root_detail::small<11>(n);
	case 12: return kth_root_detail::small<12>(n);
	case 13: return kth_root_detail::small<13>(n);
	case 14: return kth_root_detail::small<14>(n);
	default: return kth_root_detail::small<15>(n);
	}
}


#line 11 "math/number_theory/totient_sum.hpp"
#include <type_traits>
#line 13 "math/number_theory/totient_sum.hpp"

namespace totient_detail {
template<class T, class I> T value(I x) {
	if constexpr (requires { std::uint64_t(T::mod()); }) {
		auto modulus = std::uint64_t(T::mod());
		if constexpr (sizeof(I) <= 8) {
			if constexpr (!std::is_signed_v<I>) return T(std::uint64_t(x) % modulus);
			else if (modulus <= std::uint64_t(INT64_MAX)) return T(std::int64_t(x) % std::int64_t(modulus));
		}
		auto residue = __int128_t(x) % __int128_t(modulus);
		return T(std::uint64_t(residue < 0 ? residue + modulus : residue));
	} else return T(x);
}

template<class T, class Count> T compute(std::uint64_t n) {
	using U = std::uint64_t;
	const U root = kth_root(n, 2);
	auto primes = prime_sieve(root + 1);
	primes.push_back(root + 1);
	const auto k = std::upper_bound(primes.begin(), primes.end(), kth_root(root, 3)) - primes.begin();
	const U cutoff = primes[k], m = cutoff * root;
	std::vector<Count> small(root + 2), large(root + 2), mu(root + 2), upper_mu(root + 2);
	std::vector<double> reciprocals(root + 1);
	for (U i = 1; i <= root; ++i) reciprocals[i] = 1.0 / double(i);
	auto divide = [&](U x, U d) -> U {
		if (n <= (U(1) << 53) && std::numeric_limits<double>::is_iec559
			&& std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits >= 53) {
			U q = double(x) * reciprocals[d];
			while (q * d > x) --q;
			while ((q + 1) * d <= x) ++q;
			return q;
		}
		return x / d;
	};
	for (U i = 1; i <= root; ++i) small[i] = i, large[i] = divide(n, i);
	for (std::size_t j = 0; j < std::size_t(k); ++j) {
		U p = primes[j], t = root / p, quotient = n / p;
		for (U i = 1; i <= t; ++i) large[i] -= large[i * p];
		for (U i = t + 1; i <= root; ++i) large[i] -= small[divide(quotient, i)];
		U end = root;
		for (U x = t; x; --x) {
			auto delta = small[x];
			for (U stop = x * p; end >= stop; --end) small[end] -= delta;
		}
	}
	mu[1] = 2;
	for (std::size_t j = k; j + 1 < primes.size() && primes[j] * primes[j] <= m; ++j) {
		U p = primes[j], square = p * p, t = root / p, quotient = n / p, bound = m / p;
		std::size_t next = j + 1;
		for (; primes[next] <= t; ++next) mu[p * primes[next]] += 2;
		for (; primes[next] <= bound; ++next) upper_mu[divide(quotient, primes[next])] += 2;
		t /= p, quotient /= p, bound /= p;
		if (square <= root) ++mu[square];
		else ++upper_mu[quotient];
		for (next = k; primes[next] <= t; ++next) ++mu[square * primes[next]];
		for (; primes[next] <= bound; ++next) ++upper_mu[divide(quotient, primes[next])];
	}
	for (U i = 1; i <= root; ++i) mu[i] += mu[i - 1];
	upper_mu[root] += mu[root];
	for (U i = root; i; --i) upper_mu[i - 1] += upper_mu[i];
	for (U i = 1; i <= root; ++i) mu[i] -= small[i], upper_mu[i] -= large[i];
	struct Rough { U value; int mu; };
	std::vector<Rough> roughs;
	for (U i = 2; i <= root; ++i) if (small[i] != small[i - 1]) roughs.push_back({i, int(mu[i] - mu[i - 1])});
	roughs.push_back({root + 1, 0});
	for (U i = n / m; i; --i) {
		U x = divide(n, i), bound = kth_root(x, 2);
		std::size_t j = 0;
		Count sum = Count(1) - large[i] + small[bound] * mu[bound];
		for (; roughs[j].value <= root / i; ++j) {
			U t = i * roughs[j].value;
			sum -= upper_mu[t] + large[t] * roughs[j].mu;
		}
		for (; roughs[j].value <= bound; ++j) {
			U t = divide(x, roughs[j].value);
			sum -= mu[t] + small[t] * roughs[j].mu;
		}
		upper_mu[i] = sum;
	}
	for (std::size_t j = k; j--;) {
		U p = primes[j], t = root / p, quotient = n / p;
		for (U i = 1; i <= t; ++i) upper_mu[i] -= upper_mu[i * p];
		for (U i = t + 1; i <= root; ++i) upper_mu[i] -= mu[divide(quotient, i)];
		U end = root;
		for (U x = t; x; --x) {
			auto delta = mu[x];
			for (U stop = x * p; end >= stop; --end) mu[end] -= delta;
		}
	}
	auto triangular = [](U x) {
		return x & 1 ? value<T>(x) * value<T>(x / 2 + 1) : value<T>(x / 2) * (value<T>(x) + T(1));
	};
	T answer(0);
	for (U i = 1; i <= root; ++i)
		answer += value<T>(i) * value<T>(upper_mu[i]) + value<T>(mu[i] - mu[i - 1]) * triangular(divide(n, i));
	return answer - triangular(root) * value<T>(mu[root]);
}
}

template<class T = std::uint64_t> T totient_sum(std::uint64_t n) {
	constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;
	using A = std::conditional_t<integer, __uint128_t, T>;
	A answer(0);
	if (n < 64) {
		std::vector<unsigned> phi(n + 1);
		for (unsigned i = 0; i <= n; ++i) phi[i] = i;
		for (unsigned p = 2; p <= n; ++p) if (phi[p] == p)
			for (unsigned i = p; i <= n; i += p) phi[i] -= phi[i] / p;
		for (unsigned i = 1; i <= n; ++i) answer += A(phi[i]);
	} else if (n <= (std::uint64_t(1) << 40)) answer = totient_detail::compute<A, std::int64_t>(n);
	else answer = totient_detail::compute<A, __int128_t>(n);
	if constexpr (integer) { assert(answer <= __uint128_t(std::numeric_limits<T>::max())); }
	return T(answer);
}


#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 7 "math/mint.hpp"

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
	output.println(totient_sum<mint>(n).val());
}
