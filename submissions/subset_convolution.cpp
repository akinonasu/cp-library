#line 1 "test/subset_convolution.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/subset_convolution"
#line 1 "convolution/subset_convolution.hpp"



#include <algorithm>
#include <bit>
#include <cstddef>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <class T>
std::vector<T> subset_convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	std::size_t n = std::bit_ceil(std::max(a.size(), b.size()));
	int levels = std::bit_width(n) - 1, width = levels + 1;
	std::vector<T> x(n * width, T(0)), y(n * width, T(0)), result(n, T(0));
	for (std::size_t i = 0; i < a.size(); ++i) x[i * width + std::popcount(i)] = a[i];
	for (std::size_t i = 0; i < b.size(); ++i) y[i * width + std::popcount(i)] = b[i];
	auto zeta = [&](std::size_t first, std::size_t length, std::size_t half) {
		for (; half < length; half *= 2)
			for (std::size_t start = first; start < first + length; start += 2 * half) {
				int low = std::popcount(start);
				for (std::size_t j = start; j < start + half; ++j)
					for (int k = low, end = std::popcount(j); k <= end; ++k) {
						x[(j + half) * width + k] += x[j * width + k];
						y[(j + half) * width + k] += y[j * width + k];
					}
			}
	};
	std::size_t block = std::min<std::size_t>(n, 256);
	for (std::size_t start = 0; start < n; start += block) zeta(start, block, 1);
	zeta(0, n, block);
	for (std::size_t mask = 0; mask < n; ++mask) {
		int count = std::popcount(mask);
		for (int k = std::min(levels, 2 * count); k >= count; --k) {
			T sum(0);
			for (int i = k - count; i <= count; ++i)
				sum += x[mask * width + i] * y[mask * width + k - i];
			x[mask * width + k] = sum;
		}
	}
	auto mobius = [&](std::size_t first, std::size_t length, std::size_t half) {
		for (; half < length; half *= 2)
			for (std::size_t start = first; start < first + length; start += 2 * half) {
				int low = std::popcount(start) + 1, high = levels - std::countr_zero(half);
				for (std::size_t j = start; j < start + half; ++j) {
					int count = std::popcount(j - start);
					for (int k = low + count; k <= high + count; ++k)
						x[(j + half) * width + k] -= x[j * width + k];
				}
			}
	};
	for (std::size_t start = 0; start < n; start += block) mobius(start, block, 1);
	mobius(0, n, block);
	for (std::size_t mask = 0; mask < n; ++mask) result[mask] = x[mask * width + std::popcount(mask)];
	return result;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "math/mint.hpp"


#include <cassert>
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


#line 4 "test/subset_convolution.test.cpp"
#include <iostream>
#line 6 "test/subset_convolution.test.cpp"

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int exponent;
	std::cin >> exponent;
	int n = 1 << exponent;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : subset_convolution(a, b)) std::cout << x << ' ';
	std::cout << '\n';
}
