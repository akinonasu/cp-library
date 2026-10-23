#line 1 "test/bitwise_and_convolution.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/bitwise_and_convolution"
#line 1 "convolution/bitwise_convolution.hpp"



#line 1 "components/bitwise_zeta.hpp"



#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace bitwise_zeta_detail {
template <bool inverse, bool superset, class T>
void stages(std::vector<T> &a, std::size_t start, std::size_t n, std::size_t half) {
	auto step = [&](std::size_t low, std::size_t high) {
		T &to = a[superset ? low : high];
		const T &from = a[superset ? high : low];
		if constexpr (inverse) to -= from;
		else to += from;
	};
	for (; half < n / 2; half *= 4)
		for (std::size_t block = start; block < start + n; block += 4 * half)
			for (std::size_t j = block; j < block + half; ++j) {
				step(j, j + half);
				step(j + 2 * half, j + 3 * half);
				step(j, j + 2 * half);
				step(j + half, j + 3 * half);
			}
	if (half < n)
		for (std::size_t j = start; j < start + half; ++j) step(j, j + half);
}
}

template <bool inverse = false, bool superset = false, class T>
void bitwise_zeta(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	constexpr std::size_t block = 1024;
	if (a.size() <= block) bitwise_zeta_detail::stages<inverse, superset>(a, 0, a.size(), 1);
	else {
		for (std::size_t start = 0; start < a.size(); start += block)
			bitwise_zeta_detail::stages<inverse, superset>(a, start, block, 1);
		bitwise_zeta_detail::stages<inverse, superset>(a, 0, a.size(), block);
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/walsh_hadamard.hpp"



#line 8 "components/walsh_hadamard.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace walsh_hadamard_detail {
template <class T>
void stages(std::vector<T> &a, std::size_t start, std::size_t n, std::size_t half) {
	auto step = [&](std::size_t low, std::size_t high) {
		T x = a[low], y = a[high];
		a[low] = x + y;
		a[high] = x - y;
	};
	for (; half < n / 2; half *= 4)
		for (std::size_t block = start; block < start + n; block += 4 * half)
			for (std::size_t j = block; j < block + half; ++j) {
				step(j, j + half);
				step(j + 2 * half, j + 3 * half);
				step(j, j + 2 * half);
				step(j + half, j + 3 * half);
			}
	if (half < n)
		for (std::size_t j = start; j < start + half; ++j) step(j, j + half);
}
}

template <class T>
void walsh_hadamard(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	constexpr std::size_t block = 1024;
	if (a.size() <= block) walsh_hadamard_detail::stages(a, 0, a.size(), 1);
	else {
		for (std::size_t start = 0; start < a.size(); start += block)
			walsh_hadamard_detail::stages(a, start, block, 1);
		walsh_hadamard_detail::stages(a, 0, a.size(), block);
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 6 "convolution/bitwise_convolution.hpp"
#include <algorithm>
#line 10 "convolution/bitwise_convolution.hpp"
#include <cstdint>
#include <limits>
#include <utility>
#line 14 "convolution/bitwise_convolution.hpp"

namespace bitwise_convolution_detail {
template <int kind, class T>
std::vector<T> multiply(std::vector<T> a, std::vector<T> b) {
	if (a.empty() || b.empty()) return {};
	std::size_t n = std::bit_ceil(std::max(a.size(), b.size()));
	a.resize(n, T(0));
	b.resize(n, T(0));
	if constexpr (kind == 2) {
		walsh_hadamard(a);
		walsh_hadamard(b);
	} else {
		bitwise_zeta<false, kind == 0>(a);
		bitwise_zeta<false, kind == 0>(b);
	}
	for (std::size_t i = 0; i < n; ++i) a[i] *= b[i];
	if constexpr (kind == 2) {
		walsh_hadamard(a);
		if constexpr (std::numeric_limits<T>::is_integer)
			for (T &x : a) x /= std::int64_t(n);
		else {
			T inverse = T(1) / T(n);
			for (T &x : a) x *= inverse;
		}
	} else bitwise_zeta<true, kind == 0>(a);
	return a;
}
}

template <class T>
std::vector<T> and_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<0>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> or_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<1>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> xor_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<2>(std::move(a), std::move(b));
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


#line 4 "test/bitwise_and_convolution.test.cpp"
#include <iostream>
#line 6 "test/bitwise_and_convolution.test.cpp"

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int exponent;
	std::cin >> exponent;
	int n = 1 << exponent;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : and_convolution(a, b)) std::cout << x << ' ';
	std::cout << '\n';
}
