#include "fps/polynomial_gcd_light.hpp"
#include <cassert>
#include <random>

using F = light_fps;
using H = fps_light_detail::PolynomialHalfGCD<mint>;
using namespace light_fps_algorithms;

F remainder(F a, F b) {
	a.shrink(), b.shrink();
	assert(!b.empty());
	if (a.size() < b.size()) return a;
	mint inverse = b.back().inv();
	for (int i = int(a.size() - b.size()); i >= 0; --i) {
		mint factor = a[i + b.size() - 1] * inverse;
		for (int j = 0; j < int(b.size()); ++j) a[i + j] -= factor * b[j];
	}
	a.resize(b.size() - 1), a.shrink();
	return a;
}

F naive_gcd(F a, F b) {
	a.shrink(), b.shrink();
	while (!b.empty()) { auto r = remainder(a, b); a = std::move(b), b = std::move(r); }
	if (!a.empty()) a /= a.back();
	return a;
}

void check(const F &a, const F &b, bool naive = true) {
	auto g = polynomial_gcd(a, b);
	assert(g.empty() || g.back() == mint(1));
	assert(polynomial_gcd(b, a) == g);
	if (naive) assert(g == naive_gcd(a, b));
	if (!g.empty()) assert(remainder(a, g).empty() && remainder(b, g).empty());
	auto [d, x, y] = polynomial_extended_gcd(a, b);
	assert(d == g);
	F identity = a * x + b * y;
	identity.shrink();
	assert(identity == g);
	F modulus = b;
	modulus.shrink();
	if (!modulus.empty()) {
		auto inverse = polynomial_inv_mod(a, b);
		assert(bool(inverse) == (modulus.size() == 1 || g.size() == 1));
		if (inverse) {
			assert(inverse->size() < modulus.size());
			assert(remainder(a * *inverse, modulus) == (modulus.size() == 1 ? F{} : F{1}));
		}
	}
}

void check_half(F a, F b) {
	a.shrink(), b.shrink();
	if (a.size() < b.size()) a.swap(b);
	if (!b.empty() && a.size() == b.size()) { a = remainder(a, b); a.swap(b); }
	if (a.empty()) return;
	auto matrix = H::half(a, b);
	F c = a, d = b;
	H::apply(matrix, c, d);
	assert(c.size() > a.size() / 2 && d.size() <= a.size() / 2);
	H::Matrix expected;
	const auto half = a.size() / 2;
	while (b.size() > half) {
		auto [q, r] = a.divmod(b);
		H::step(expected, q), a = std::move(b), b = std::move(r);
	}
	assert(matrix.a == expected.a);
	assert(c == a && d == b);
	F determinant = matrix.a[0] * matrix.a[3] - matrix.a[1] * matrix.a[2];
	determinant.shrink();
	assert(determinant == F{1} || determinant == F{mint(-1)});
}

struct PlainMint {
	unsigned v = 0;
	static constexpr int mod() { return 998244353; }
	PlainMint() = default;
	PlainMint(long long x) { x %= mod(); v = unsigned(x < 0 ? x + mod() : x); }
	static PlainMint raw(unsigned x) { PlainMint r; r.v = x; return r; }
	unsigned val() const { return v; }
	PlainMint &operator+=(PlainMint x) { v += x.v; if (v >= mod()) v -= mod(); return *this; }
	PlainMint &operator-=(PlainMint x) { v += mod() - x.v; if (v >= mod()) v -= mod(); return *this; }
	PlainMint &operator*=(PlainMint x) { v = std::uint64_t(v) * x.v % mod(); return *this; }
	PlainMint &operator/=(PlainMint x) { return *this *= x.inv(); }
	PlainMint operator-() const { return raw(v ? mod() - v : 0); }
	PlainMint inv() const {
		PlainMint a = *this, r = 1;
		for (unsigned k = mod() - 2; k; k >>= 1, a *= a) if (k & 1) r *= a;
		return r;
	}
	friend PlainMint operator+(PlainMint a, PlainMint b) { return a += b; }
	friend PlainMint operator-(PlainMint a, PlainMint b) { return a -= b; }
	friend PlainMint operator*(PlainMint a, PlainMint b) { return a *= b; }
	friend PlainMint operator/(PlainMint a, PlainMint b) { return a /= b; }
	friend bool operator==(PlainMint, PlainMint) = default;
};

int main() {
	std::mt19937 random(294384);
	for (F a : {F{}, F{0}, F{0, 0}, F{1}, F{2}, F{0, 3}, F{1, 2, 3, 0}})
		for (F b : {F{}, F{0}, F{0, 0}, F{1}, F{3}, F{0, 0, 4}, F{2, 4, 6}}) check(a, b);
	for (int n : {1, 2, 3, 8, 31, 32, 63, 64, 65, 127, 128, 129, 255, 256, 257, 511, 512, 513, 1023}) {
		for (int trial = 0; trial < 6; ++trial) {
			int m = trial == 0 ? n : trial == 1 ? n / 2 : trial == 2 ? 1 : 1 + random() % n;
			F a(n), b(m), common(trial == 3 ? n / 3 : 0);
			for (auto *f : {&a, &b, &common}) {
				for (auto &x : *f) x = random() % 13;
				if (!f->empty()) f->back() = 1 + random() % 12;
			}
			if (!common.empty()) a *= common, b *= common;
			if (trial == 4) a *= b;
			if (trial == 5) a.resize(a.size() + 3), b.resize(b.size() + 5);
			check(a, b);
			check_half(a, b);
		}
	}
	for (int n : {257, 1025, 8193}) {
		F common(n), a(n / 2), b(n / 2 - 1);
		for (auto *f : {&common, &a, &b}) {
			for (auto &x : *f) x = random();
			f->back() = 1;
		}
		a *= common, b *= common;
		assert(polynomial_gcd(a, b) == common);
		check(a, b, false);
		F sparse(n + 1); sparse.front() = -1, sparse.back() = 1;
		check(sparse, F{mint(-1), 1});
	}
	using P = LightFormalPowerSeries<PlainMint>;
	P a(140), b(129);
	for (auto *f : {&a, &b}) { for (auto &x : *f) x = random() % 13; f->back() = 1; }
	auto [g, x, y] = polynomial_extended_gcd(a, b);
	auto identity = a * x + b * y;
	identity.shrink();
	assert(identity == g && polynomial_gcd(a, b) == g);
	auto inverse = polynomial_inv_mod(a, b);
	assert(bool(inverse) == (g.size() == 1));
	if (inverse) assert((a * *inverse).divmod(b).second == P{1});
}
