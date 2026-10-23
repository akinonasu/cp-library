#line 1 "test/lcm_convolution.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/lcm_convolution"
#line 1 "convolution/gcd_lcm_convolution.hpp"



#line 1 "components/divisor_zeta.hpp"



#line 1 "math/number_theory/prime_sieve.hpp"



#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n <= 2) return result;
	if (n >= 1024) result.reserve(std::size_t(double(n) / (std::log(double(n)) - 1.1)));
	result.push_back(2);
	const std::size_t count = n / 2;
	std::size_t block_size = 32768;
	std::vector<std::uint8_t> prime(std::min(count, block_size), 1);
	auto append = [&](std::size_t base, std::size_t length) {
		std::size_t i = 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		for (; i + 8 <= length; i += 8) {
			std::uint64_t word;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			while (word) {
				result.push_back(2 * (base + i + (__builtin_ctzll(word) >> 3)) + 1);
				word &= word - 1;
			}
		}
#endif
		for (; i < length; ++i) if (prime[i]) result.push_back(2 * (base + i) + 1);
	};
	if (count <= block_size) {
		prime[0] = 0;
		for (std::size_t p = 3; p <= (n - 1) / p; p += 2) {
			if (!prime[p / 2]) continue;
			for (std::size_t j = p * p / 2; j < count; j += p) prime[j] = 0;
		}
		append(0, count);
		return result;
	}
	std::size_t limit = std::size_t(std::sqrt(static_cast<long double>(n - 1)));
	while (limit > (n - 1) / limit) --limit;
	while (limit + 1 <= (n - 1) / (limit + 1)) ++limit;
	constexpr std::size_t period = 1155;
	block_size = (std::max(block_size, limit) + period - 1) / period * period;
	prime.resize(block_size);
	constexpr auto pattern = [] {
		std::array<std::uint8_t, period> value{};
		for (std::size_t i = 0; i < period; ++i)
			value[i] = (2 * i + 1) % 3 && (2 * i + 1) % 5 && (2 * i + 1) % 7 && (2 * i + 1) % 11;
		return value;
	}();
	auto primes = prime_sieve(limit + 1);
	std::vector<std::size_t> next(primes.size());
	for (std::size_t i = 1; i < primes.size(); ++i) next[i] = primes[i] * primes[i] / 2;
	for (std::size_t base = 0; base < count;) {
		std::size_t length = std::min(block_size, count - base);
		for (std::size_t offset = 0; offset < length; offset += period)
			std::copy_n(pattern.begin(), std::min(period, length - offset), prime.begin() + offset);
		if (!base) {
			prime[0] = 0;
			prime[1] = prime[2] = prime[3] = prime[5] = 1;
		}
		for (std::size_t i = 5; i < primes.size(); ++i) {
			std::size_t j = next[i];
			for (; j < length; j += primes[i]) prime[j] = 0;
			next[i] = j - length;
		}
		append(base, length);
		base += length;
	}
	return result;
}


#line 7 "components/divisor_zeta.hpp"

template <bool inverse = false, bool multiples = false, class T>
void divisor_zeta(std::vector<T> &a, const std::vector<std::size_t> &ps) {
	if (a.empty()) return;
	for (std::size_t p : ps) {
		std::size_t end = (a.size() - 1) / p;
		for (std::size_t step = 1; step <= end; ++step) {
			std::size_t i = inverse == multiples ? step : end + 1 - step;
			T &to = a[multiples ? i : i * p];
			const T &from = a[multiples ? i * p : i];
			if constexpr (inverse) to -= from;
			else to += from;
		}
	}
}

template <bool inverse = false, bool multiples = false, class T>
void divisor_zeta(std::vector<T> &a) { divisor_zeta<inverse, multiples>(a, prime_sieve(a.size())); }


#line 6 "convolution/gcd_lcm_convolution.hpp"
#include <cassert>
#line 8 "convolution/gcd_lcm_convolution.hpp"
#include <utility>
#line 10 "convolution/gcd_lcm_convolution.hpp"

namespace gcd_lcm_convolution_detail {

template <bool gcd, class T>
std::vector<T> multiply(std::vector<T> a, std::vector<T> b) {
	if (a.empty() || b.empty()) return {};
	assert(a[0] == T(0) && b[0] == T(0));
	std::size_t n = std::max(a.size(), b.size());
	a.resize(n, T(0));
	b.resize(n, T(0));
	auto ps = prime_sieve(n);
	divisor_zeta<false, gcd>(a, ps);
	divisor_zeta<false, gcd>(b, ps);
	for (std::size_t i = 1; i < n; ++i) a[i] *= b[i];
	divisor_zeta<true, gcd>(a, ps);
	return a;
}
}

template <class T>
std::vector<T> gcd_convolution(std::vector<T> a, std::vector<T> b) {
	return gcd_lcm_convolution_detail::multiply<true>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> lcm_convolution(std::vector<T> a, std::vector<T> b) {
	return gcd_lcm_convolution_detail::multiply<false>(std::move(a), std::move(b));
}


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
	static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	unsigned val() const {
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
	Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	Mint &operator*=(Mint b) {
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
	Mint operator+() const { return *this; }
	Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	Mint operator+(Mint b) const { return Mint(*this) += b; }
	Mint operator-(Mint b) const { return Mint(*this) -= b; }
	Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	bool operator!=(Mint b) const { return !(*this == b); }
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


#line 4 "test/lcm_convolution.test.cpp"
#include <iostream>
#line 6 "test/lcm_convolution.test.cpp"

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n;
	std::cin >> n;
	std::vector<mint> a(n + 1), b(n + 1);
	for (int i = 1; i <= n; ++i) std::cin >> a[i];
	for (int i = 1; i <= n; ++i) std::cin >> b[i];
	auto c = lcm_convolution(a, b);
	for (int i = 1; i <= n; ++i) std::cout << c[i] << ' ';
	std::cout << '\n';
}
