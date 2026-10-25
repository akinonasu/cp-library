#include "convolution/convolution_mod.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

template<class T>
std::vector<std::uint32_t> naive(const std::vector<T> &a, const std::vector<T> &b, std::uint32_t mod) {
	if (a.empty() || b.empty()) return {};
	std::vector<std::uint32_t> result(a.size() + b.size() - 1);
	for (std::size_t i = 0; i < a.size(); ++i)
		for (std::size_t j = 0; j < b.size(); ++j) {
			__int128 x = (__int128(a[i]) % mod + mod) % mod;
			__int128 y = (__int128(b[j]) % mod + mod) % mod;
			result[i + j] = (result[i + j] + x * y) % mod;
		}
	return result;
}

template<class T>
void check() {
	std::mt19937_64 random(897);
	for (std::uint32_t mod : {1u, 2u, 17u, 998244353u, 1000000007u, 4294967295u})
		for (int n : {0, 1, 2, 7, 8, 16, 17, 31, 32, 33, 127, 128, 129, 257}) {
			std::vector<T> a(n);
			for (auto &value : a) value = T(random());
			if (n) a[0] = std::numeric_limits<T>::max();
			if (n > 1) a[1] = std::numeric_limits<T>::lowest();
			auto b = a;
			auto expected = naive(a, a, mod);
			assert(convolution_mod(a, a, mod) == expected);
			assert(convolution_mod(a, b, mod) == expected);
			assert(a == b);
			b.resize(n / 2);
			assert(convolution_mod(a, b, mod) == naive(a, b, mod));
			assert(convolution_mod(b, a, mod) == naive(b, a, mod));
		}
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t P, std::uint32_t Q, std::uint32_t R>
void check_reconstruction() {
	if (!__builtin_cpu_supports("avx2")) return;
	constexpr __uint128_t bound = __uint128_t(P) * Q * R / 4;
	auto inv = [](std::uint64_t x, std::uint32_t mod) {
		std::uint64_t result = 1;
		for (std::uint32_t e = mod - 2; e; e >>= 1, x = x * x % mod) if (e & 1) result = result * x % mod;
		return result;
	};
	const auto ip = inv(std::uint64_t(Q) * R % P, P), iq = inv(std::uint64_t(P) * R % Q, Q), ir = inv(std::uint64_t(P) * Q % R, R);
	std::mt19937_64 random(P);
	for (int n : {1, 2, 7, 8, 9, 16, 17, 1024}) {
		std::vector<std::uint32_t> a(n), b(n), c(n), expected(n);
		for (int i = 0; i < n; ++i) {
			__uint128_t value = (__uint128_t(random()) << 64 | random()) % bound;
			if (i < 4) value = i == 0 ? 0 : i == 1 ? 1 : i == 2 ? bound - 1 : bound / 2;
			a[i] = std::uint64_t(value % P) * ip % P;
			b[i] = std::uint64_t(value % Q) * iq % Q;
			c[i] = std::uint64_t(value % R) * ir % R;
			expected[i] = value % 1000000007;
		}
		convolution_mod_detail::combine<P, Q, R>(a.data(), b.data(), c.data(), n);
		assert(a == expected);
	}
}
#endif

int main() {
#if defined(__x86_64__) || defined(__i386__)
	check_reconstruction<167772161, 469762049, 754974721>();
	check_reconstruction<754974721, 1224736769, 2013265921>();
#endif
	check<std::int64_t>();
	check<std::uint64_t>();
	check<std::int32_t>();
	check<std::uint32_t>();
	std::mt19937_64 random(6734);
	for (unsigned mod : {998244353u, 1000000007u, 4294967295u})
		for (int n : {33, 64, 129}) for (int m : {8 * n - 1, 8 * n, 20 * n + 7}) {
			std::vector<std::int64_t> a(n), b(m);
			for (auto &x : a) x = std::int64_t(random());
			for (auto &x : b) x = std::int64_t(random());
			a.front() = INT64_MIN;
			b.back() = INT64_MAX;
			auto expected = naive(a, b, mod);
			assert(convolution_mod(a, b, mod) == expected);
			assert(convolution_mod(b, a, mod) == expected);
		}
	std::vector<std::int64_t> a(69), b(4096), expected(a.size() + b.size() - 1);
	for (auto &x : a) x = std::int64_t(random() % 101) - 50;
	for (auto &x : b) x = std::int64_t(random() % 101) - 50;
	for (std::size_t i = 0; i < a.size(); ++i)
		for (std::size_t j = 0; j < b.size(); ++j) expected[i + j] += a[i] * b[j];
	assert(convolution_integer(a, b) == expected);
	assert(convolution_integer(b, a) == expected);
}
