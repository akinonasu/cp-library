#include "fps/fps_light.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>

namespace {
using U = std::uint64_t;
using V = std::vector<U>;
constexpr U mod = 998244353;
U checks = 0;
U power(U a, U k) {
	U result = 1;
	for (; k; k >>= 1, a = a * a % mod) if (k & 1) result = result * a % mod;
	return result;
}
U inv(U a) { assert(a); return power(a, mod - 2); }
void check(bool condition) {
	++checks;
	if (!condition) { std::fprintf(stderr, "light FPS check failed: %llu\n", static_cast<unsigned long long>(checks)); std::abort(); }
}
void equal(const light_fps &f, const V &expected) {
	check(f.size() == expected.size());
	for (std::size_t i = 0; i < f.size(); ++i) check(f[i].val() == expected[i]);
}
light_fps make(const V &a) {
	light_fps f(a.size());
	for (std::size_t i = 0; i < a.size(); ++i) f[i] = mint::raw(static_cast<unsigned>(a[i]));
	return f;
}
V padded(V a, int n) { a.resize(n); return a; }
V product(const V &a, const V &b, int n) {
	V result(n);
	for (int i = 0; i < std::min<int>(n, a.size()); ++i)
		for (int j = 0; j < std::min<int>(n - i, b.size()); ++j)
			result[i + j] = (result[i + j] + a[i] * b[j]) % mod;
	return result;
}
V full_product(const V &a, const V &b) { return a.empty() || b.empty() ? V{} : product(a, b, int(a.size() + b.size() - 1)); }
V inverse(const V &a, int n) {
	V result(n);
	if (!n) return result;
	result[0] = inv(a[0]);
	for (int i = 1; i < n; ++i) {
		U sum = 0;
		for (int j = 1; j <= i && j < int(a.size()); ++j) sum = (sum + a[j] * result[i - j]) % mod;
		result[i] = (mod - sum) * result[0] % mod;
	}
	return result;
}
V logarithm(const V &a, int n) {
	V result(n);
	for (int i = 1; i < n; ++i) {
		U x = i < int(a.size()) ? U(i) * a[i] % mod : 0;
		for (int j = 1; j < i; ++j) if (i - j < int(a.size()))
			x = (x + mod - U(j) * result[j] % mod * a[i - j] % mod) % mod;
		result[i] = x * inv(i) % mod;
	}
	return result;
}
V exponential(const V &a, int n) {
	V result(n);
	if (n) result[0] = 1;
	for (int i = 1; i < n; ++i) {
		U x = 0;
		for (int j = 1; j <= i && j < int(a.size()); ++j) x = (x + U(j) * a[j] % mod * result[i - j]) % mod;
		result[i] = x * inv(i) % mod;
	}
	return result;
}
V series_power(V a, U k, int n) {
	a.resize(n);
	V result(n);
	if (n) result[0] = 1;
	for (; k; k >>= 1, a = product(a, a, n)) if (k & 1) result = product(result, a, n);
	return result;
}
void trim(V &a) { while (!a.empty() && !a.back()) a.pop_back(); }
std::pair<V, V> division(V a, V b) {
	trim(a); trim(b); assert(!b.empty());
	if (a.size() < b.size()) return {{}, a};
	V q(a.size() - b.size() + 1);
	U last = inv(b.back());
	for (int i = int(q.size()) - 1; i >= 0; --i) {
		q[i] = a[i + b.size() - 1] * last % mod;
		for (int j = 0; j < int(b.size()); ++j) a[i + j] = (a[i + j] + mod - q[i] * b[j] % mod) % mod;
	}
	trim(a); trim(q);
	return {q, a};
}
V random_values(std::mt19937_64 &rng, int n) {
	V a(n);
	for (auto &x : a) x = rng() % mod;
	return a;
}
void basics() {
	std::mt19937_64 rng(2385167);
	for (int n : {0, 1, 2, 3, 31, 32, 33, 63, 64, 65, 100}) {
		V a = random_values(rng, n);
		light_fps f = make(a), saved = f;
		V negative = a;
		for (auto &x : negative) x = (mod - x) % mod;
		equal(-f, negative);
		for (int m : {0, 1, 3, 32, 65}) {
			V b = random_values(rng, m), sum(std::max(n, m)), difference(sum.size());
			for (std::size_t i = 0; i < sum.size(); ++i) {
				U x = i < a.size() ? a[i] : 0, y = i < b.size() ? b[i] : 0;
				sum[i] = (x + y) % mod; difference[i] = (x + mod - y) % mod;
			}
			light_fps g = make(b);
			equal(f + g, sum); equal(f - g, difference);
			auto h = f; h += g; equal(h, sum); h = f; h -= g; equal(h, difference);
			equal(f * g, full_product(a, b));
			equal(light_fps::multiply(f, g), full_product(a, b));
		}
		for (U c : {U(0), U(1), U(17), mod - 1}) {
			V sum = a, difference = a, scaled = a;
			if (sum.empty()) sum.resize(1), difference.resize(1);
			sum[0] = (sum[0] + c) % mod; difference[0] = (difference[0] + mod - c) % mod;
			for (auto &x : scaled) x = x * c % mod;
			equal(f + mint(c), sum); equal(f - mint(c), difference); equal(f * mint(c), scaled);
			if (c) { for (std::size_t i = 0; i < a.size(); ++i) scaled[i] = a[i] * inv(c) % mod; equal(f / mint(c), scaled); }
		}
		V derivative(n), integral(n);
		for (int i = 1; i < n; ++i) { derivative[i - 1] = a[i] * i % mod; integral[i] = a[i - 1] * inv(i) % mod; }
		equal(f.diff(), derivative); equal(f.integral(), integral);
		for (int d : {0, 1, 3, n, n + 1}) {
			V left(n), right(n);
			for (int i = 0; i < n; ++i) { if (i >= d) left[i] = a[i - d]; if (i + d < n) right[i] = a[i + d]; }
			equal(f << d, left); equal(f >> d, right);
			auto h = f; h <<= d; equal(h, left); h = f; h >>= d; equal(h, right);
		}
		for (int take : {0, 1, n, n + 1}) equal(f.pre(take), V(a.begin(), a.begin() + std::min(n, take)));
		for (U x : {U(0), U(1), U(37), mod - 1}) {
			U expected = 0, xp = 1;
			for (U coefficient : a) { expected = (expected + coefficient * xp) % mod; xp = xp * x % mod; }
			check(f.eval(mint(x)).val() == expected);
		}
		for (int d : {0, 1, 3, n + 1}) {
			V factor(std::max(1, d + 1)); factor[0] = 1; factor[d] = (factor[d] + 13) % mod;
			auto h = f; h.onemul(d, 13); equal(h, product(a, factor, n));
			h.onediv(d, 13); equal(h, a);
		}
		equal(f * f, full_product(a, a));
		auto doubled = f; doubled += doubled;
		V twice = a; for (auto &x : twice) x = 2 * x % mod; equal(doubled, twice);
		check(f == saved);
	}
	light_fps trailing{1, 0, 0}; trailing.shrink(); equal(trailing, {1});
	light_fps zero(10); zero.shrink(); check(zero.empty());
}
void series() {
	std::mt19937_64 rng(175621);
	for (int n = 0; n <= 100; ++n) {
		V a = random_values(rng, n);
		if (n) a[0] = 1;
		auto f = make(a);
		equal(f.inv(), inverse(a, n));
		equal(f.log(), logarithm(a, n));
		if (n) { V nonunit = a; nonunit[0] = 7; equal(make(nonunit).inv(n + 3), inverse(nonunit, n + 3)); }
		if (n) a[0] = 0;
		f = make(a);
		equal(f.exp(), exponential(a, n));
		for (int d : {0, 1, n + 3}) {
			V unit = a; if (unit.empty()) unit.resize(1); unit[0] = 1;
			equal(make(unit).inv(d), inverse(unit, d));
			equal(make(unit).log(d), logarithm(unit, d));
			equal(f.exp(d), exponential(a, d));
		}
		V denominator = random_values(rng, n + 1); denominator[0] = 7;
		for (int d : {0, n, n + 3}) equal(f.div_series(make(denominator), d), product(a, inverse(denominator, d), d));
		equal(f / make(denominator), product(a, inverse(denominator, n), n));
		for (U k : {U(0), U(1), U(2), U(5), U(37)}) {
			equal(f.pow(static_cast<long long>(k)), series_power(a, k, n));
			if (n) { V shifted = a; shifted[0] = 13; equal(make(shifted).pow(static_cast<long long>(k)), series_power(shifted, k, n)); }
		}
		if (n <= 16) equal(f.pow(std::numeric_limits<long long>::max()), series_power(a, std::numeric_limits<long long>::max(), n));
	}
	for (int n : {129, 257, 513}) {
		V a = random_values(rng, n); a[0] = 1;
		equal(make(a).inv(), inverse(a, n));
		equal(make(a).log(), logarithm(a, n));
		a[0] = 0; equal(make(a).exp(), exponential(a, n));
	}
	for (int n : {0, 1, 2, 3, 31, 33, 65, 100}) for (int m : {1, 2, 3, 32, 64, 101}) {
		V a = random_values(rng, n), b = random_values(rng, m); b.back() = 1;
		if (n > 2) a.back() = 0;
		if (m > 2) b.back() = 0, b[m - 2] = 7;
		auto [q, r] = make(a).divmod(make(b));
		auto [expected_q, expected_r] = division(a, b);
		equal(q, expected_q); equal(r, expected_r); equal(make(a) % make(b), expected_r);
	}
}
void square_roots() {
	std::mt19937_64 rng(185436);
	check(power(3, (mod - 1) / 2) == mod - 1);
	check(light_fps{3}.sqrt().empty());
	check((light_fps{0, 1}).sqrt().empty());
	check(light_fps{1}.sqrt(0).empty());
	for (int n : {0, 1, 2, 3, 7, 17, 31, 32, 33, 63, 64, 65, 100, 257}) {
		equal(light_fps(n).sqrt(), V(n));
		for (int shift : {0, 1, 3, 17}) {
			if (2 * shift >= n) continue;
			V a = random_values(rng, n);
			std::fill(a.begin(), a.begin() + shift, 0);
			a[shift] = 1 + rng() % (mod - 1);
			V f = product(a, a, n);
			auto r = make(f).sqrt();
			check(r.size() == std::size_t(n));
			V canonical(n); for (int i = 0; i < n; ++i) canonical[i] = r[i].val();
			equal(make(canonical) * make(canonical), full_product(canonical, canonical));
			check(product(canonical, canonical, n) == f);
			check(canonical[shift] == std::min(a[shift], mod - a[shift]));
			for (int i = 0; i < shift; ++i) check(!canonical[i]);
		}
	}
	for (int n : {1, 3, 33, 100}) {
		V monomial(n); monomial[0] = 49;
		V expected(n); expected[0] = 7;
		equal(make(monomial).sqrt(n + 3), padded(expected, n + 3));
	}
}
void dense_boundaries() {
	std::mt19937_64 rng(813729);
	for (int n : {127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025, 4095, 4096, 4097, 4098, 4099, 4100, 4101, 4103, 16385}) {
		V a = random_values(rng, n);
		a[0] = 7;
		auto f = make(a), inverse_f = f.inv();
		V unit(n); unit[0] = 1;
		equal((f * inverse_f).pre(n), unit);
		equal(inverse_f.pre(32), inverse(a, 32));
		a[0] = 1;
		f = make(a);
		auto logarithm_f = f.log();
		equal(logarithm_f.pre(32), logarithm(a, 32));
		check(logarithm_f.exp() == f);
		check(logarithm_f.diff().pre(n - 1) == (f.diff() * f.inv()).pre(n - 1));
		a[0] = 0;
		f = make(a);
		auto exponential_f = f.exp();
		equal(exponential_f.pre(32), exponential(a, 32));
		check(exponential_f.log() == f);
		check(exponential_f.diff().pre(n - 1) == (exponential_f * f.diff()).pre(n - 1));
		for (int shift : {0, 3}) {
			V square_root = random_values(rng, n);
			std::fill(square_root.begin(), square_root.begin() + shift, 0);
			square_root[shift] = 7;
			auto r = make(square_root), square = (r * r).pre(n), computed = square.sqrt();
			check(computed.size() == std::size_t(n));
			check((computed * computed).pre(n) == square);
			check(computed[shift] == mint(7));
		}
	}
}

void closed_form_tails() {
	for (int n : {8192, 8193, 8194, 8195, 8196, 8197}) {
		V ones(n, 1), ramp(n), logarithm(n), reciprocal(n);
		logarithm[1] = 1;
		for (int i = 0; i < n; ++i) ramp[i] = i + 1;
		for (int i = 2; i < n; ++i) logarithm[i] = mod - (mod / U(i)) * logarithm[mod % U(i)] % mod;
		reciprocal[0] = 1; reciprocal[1] = mod - 2; reciprocal[2] = 1;
		equal(make(ramp).inv(), reciprocal);
		equal(make(ones).log(), logarithm);
		equal(make(logarithm).exp(), ones);
		equal(make(ramp).sqrt(), ones);
		equal(make(ramp).div_series(make(ones)), ones);
	}
}

}

int main() {
	basics();
	series();
	square_roots();
	dense_boundaries();
	closed_form_tails();
	std::printf("light FPS: %llu independent checks passed\n", static_cast<unsigned long long>(checks));
}
