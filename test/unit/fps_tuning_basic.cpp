#include "fps/fps.hpp"
#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

static void require(bool ok, int line = __builtin_LINE()) {
	if (!ok) {
		std::fprintf(stderr, "line %d\n", line);
		std::abort();
	}
}

static fps product(const fps &a, const fps &b, int n) {
	fps c(n);
	for (int i = 0; i < std::min<int>(n, a.size()); ++i)
		for (int j = 0; j < std::min<int>(n - i, b.size()); ++j) c[i + j] += a[i] * b[j];
	return c;
}

static fps inverse(const fps &a, int n) {
	fps b(n);
	if (!n) return b;
	b[0] = a[0].inv();
	for (int i = 1; i < n; ++i) {
		for (int j = 1; j <= std::min<int>(i, a.size() - 1); ++j) b[i] -= a[j] * b[i - j];
		b[i] *= b[0];
	}
	return b;
}

static fps exponential(const fps &a, int n) {
	fps b(n);
	if (!n) return b;
	b[0] = 1;
	for (int i = 1; i < n; ++i) {
		for (int j = 1; j <= std::min<int>(i, a.size() - 1); ++j) b[i] += a[j] * fps_mint(j) * b[i - j];
		b[i] /= fps_mint(i);
	}
	return b;
}

template <int count>
static void check_products() {
	using Ops = fps_detail::Operations<fps_mint>;
	for (unsigned int x : {0u, 1u, fps_mint::mod() - 1})
		for (unsigned int y : {0u, 1u, fps_mint::mod(), 2 * fps_mint::mod() - 1})
			for (int n : {1, 3, 8, 16}) {
				unsigned int a[16], b[16], dst[16];
				std::fill_n(a, 16, x);
				std::fill_n(b, 16, y);
				std::fill_n(dst, 16, 2 * fps_mint::mod() - 1);
				const unsigned int *as[7], *bs[7];
				std::fill_n(as, 7, a);
				std::fill_n(bs, 7, b);
				Ops::template accumulate_products<count, true>(dst, as, bs, n);
				fps_mint expected = fps_mint::word(x) * fps_mint::word(y) * count;
				for (int i = 0; i < n; ++i) require(fps_mint::word(dst[i]) == expected);
				Ops::template accumulate_products<count, false>(dst, as, bs, n);
				for (int i = 0; i < n; ++i) require(fps_mint::word(dst[i]) == expected * 2);
			}
	if constexpr (count < 7) check_products<count + 1>();
}

int main() {
	check_products<1>();
	std::mt19937 rng(731);
	for (int n : {0, 1, 2, 3, 31, 32, 33, 63, 64, 65, 95, 96, 97, 127, 128, 129, 255, 256, 257, 511, 512, 513}) {
		fps a(n), b(n);
		for (int i = 0; i < n; ++i) a[i] = rng(), b[i] = rng();
		if (!n) {
			require(a.inv().empty() && a.log().empty() && a.exp().empty() && a.sqrt().empty());
			continue;
		}
		a[0] = 1;
		b[0] = 3;
		require(a.inv() == inverse(a, n));
		require(a.div_series(b) == product(a, inverse(b, n), n));
		require(a.log().exp() == a);
		b[0] = 0;
		require(b.exp() == exponential(b, n));
		fps square = product(a, a, n), root = square.sqrt();
		require(product(root, root, n) == square);
		fps power(n);
		power[0] = 1;
		for (int k = 0; k <= 7; ++k) {
			require(a.pow(k) == power);
			power = product(power, a, n);
		}
	}
	for (int n : {1023, 1024, 1025, 4095, 4096, 4097, 32767, 32768, 32769, 65537}) {
		fps a(n), b(n);
		for (int i = 0; i < n; ++i) a[i] = rng(), b[i] = rng();
		a[0] = 1;
		b[0] = 2;
		fps one(n);
		one[0] = 1;
		require((a * a.inv()).pre(n) == one);
		require(a.log().exp() == a);
		require((a.div_series(b) * b).pre(n) == a);
		fps square = (a * a).pre(n), root = square.sqrt();
		require(root.size() == a.size() && (root * root).pre(n) == square);
		fps short_a(a.begin(), a.begin() + n / 3);
		fps short_b(b.begin(), b.begin() + n / 4);
		short_a.resize(n);
		require((short_a.div_series(short_b) * short_b).pre(n) == short_a);
	}
	for (int n : {65, 257, 1025}) {
		for (int first : {0, 1, 2, 3, 8}) {
			fps a(n);
			for (int i = first; i < n; i += 17) a[i] = rng();
			fps power(n);
			power[0] = 1;
			for (int k = 0; k <= 7; ++k) {
				require(a.pow(k) == power);
				power = product(power, a, n);
			}
			if (first) require(a.pow(LLONG_MAX) == fps(n));
		}
	}
	{
		fps a(65);
		for (auto &x : a) x = rng();
		a[0] = 3;
		for (long long k : {998244352LL, 998244353LL, 998244354LL, LLONG_MAX}) {
			fps base = a, expected(65);
			expected[0] = 1;
			for (long long e = k; e; e >>= 1, base = product(base, base, 65))
				if (e & 1) expected = product(expected, base, 65);
			require(a.pow(k) == expected);
		}
	}
	for (int length : {1, 3, 65, 257}) {
		fps a(length);
		for (auto &x : a) x = rng();
		a[0] = 1;
		for (int n : {0, 1, 2, 64, 256, 513}) {
			require(a.inv(n) == inverse(a, n));
			require(a.log(n).exp(n) == [&] { fps b = a.pre(n); b.resize(n); return b; }());
		}
	}
	{
		fps a(4097), b(3073);
		for (auto &x : a) x = rng();
		for (auto &x : b) x = rng();
		b[0] = 1;
		for (int n : {1, 31, 65, 128}) require(a.div_series(b, n) == product(a, inverse(b, n), n));
	}
	for (int n : {127, 128, 129, 257, 511, 512, 513, 1000, 1023, 1024, 1025, 4097}) {
		for (int m : {n / 4, n / 2, n / 2 + 1, n - n / 4}) {
			fps a(n), b(m);
			for (auto &x : a) x = rng();
			for (auto &x : b) x = rng();
			b.back() = 1;
			auto [q, r] = a.divmod(b);
			require(r.size() < b.size());
			fps recovered = q * b + r;
			recovered.resize(n);
			require(recovered == a);
		}
	}
}
