#include "fps/power_projection.hpp"
#include "fps/compose.hpp"
#include "fps/taylor_shift.hpp"
#include <cstdlib>
#include <random>

void require(bool ok) {
	if (!ok) std::abort();
}

fps naive(const fps &f, const fps &w, int m) {
	fps result(m), power(w.size());
	if (!w.empty()) power[0] = 1;
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < static_cast<int>(w.size()); ++j) result[i] += power[j] * w[j];
		fps next(w.size());
		for (int j = 0; j < static_cast<int>(w.size()); ++j)
			for (int k = 0; k < static_cast<int>(f.size()) && j + k < static_cast<int>(w.size()); ++k)
				next[j + k] += power[j] * f[k];
		power = std::move(next);
	}
	return result;
}

struct CanonicalMint {
	unsigned int value = 0;
	static constexpr int mod() { return 998244353; }
	CanonicalMint() = default;
	CanonicalMint(unsigned int x) : value(x % mod()) {}
	static CanonicalMint raw(unsigned int x) { CanonicalMint r; r.value = x; return r; }
	unsigned int val() const { return value; }
};

int main() {
	std::mt19937 random(719038);
	auto check = [&](const fps &f, const fps &w, int m) {
		auto before_f = f, before_w = w;
		auto expected = naive(f, w, m);
		require(power_projection(f, w, m) == expected);
		require(f == before_f && w == before_w);
		FormalPowerSeries<CanonicalMint> a(f.size()), b(w.size());
		for (int i = 0; i < static_cast<int>(f.size()); ++i) a[i] = CanonicalMint::raw(f[i].val());
		for (int i = 0; i < static_cast<int>(w.size()); ++i) b[i] = CanonicalMint::raw(w[i].val());
		auto actual = power_projection(a, b, m);
		require(actual.size() == expected.size());
		for (int i = 0; i < m; ++i) require(actual[i].val() == expected[i].val());
	};
	for (int n : {0, 1, 2, 3, 8, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129}) {
		for (int trial = 0; trial < 6; ++trial) {
			fps f(n + trial), w(n);
			for (auto &x : f) x = random();
			for (auto &x : w) x = random();
			if (!f.empty() && trial % 2 == 0) f[0] = 0;
			if (trial == 2) for (int i = 1; i < std::min<int>(7, f.size()); ++i) f[i] = 0;
			if (trial == 3) for (int i = 1; i < static_cast<int>(f.size()); ++i) f[i] = 0;
			if (trial == 4) w.resize(n + 5);
			int m = trial == 5 ? 0 : n + 11;
			check(f, w, m);
			require(power_projection(f, w) == naive(f, w, w.size()));
			for (int count : {1, 2, 8}) check(f, w, count);
		}
		fps f(n), w(n);
		for (auto &x : f) x = random();
		if (n) w.back() = 1;
		if (n) {
			require(power_projection(f, n - 1, n + 3) == naive(f, w, n + 3));
			f[0] = 0;
			require(power_projection(f, n - 1) == naive(f, w, n));
		}
		check({}, w, n + 1);
		check(f, fps(n), n + 1);
		check(f, f, n + 1);
	}
	for (int first : {1, 2, 7, 32}) {
		fps f(first + 1), w(145);
		f[first] = 11;
		for (auto &x : w) x = random();
		check(f, w, 181);
		f[0] = 37;
		check(f, w, 181);
	}
	for (int degree : {0, 1, 2, 31, 32, 63, 64, 127}) {
		fps expected(220);
		mint value = 1;
		for (int i = 0; i < static_cast<int>(expected.size()); ++i) {
			if (i >= degree) {
				expected[i] = value;
				value *= mint(i + 1) / mint(i + 1 - degree);
			}
		}
		require(power_projection(fps{1, 1}, degree, expected.size()) == expected);
	}
	for (int n : {257, 1024, 4097, 16384}) {
		fps f(n), a(n), w(n);
		for (auto &x : f) x = random();
		for (auto &x : a) x = random();
		for (auto &x : w) x = random();
		f[0] = 0;
		auto p = power_projection(f, w), composed = compose(a, f, n);
		mint lhs = 0, rhs = 0;
		for (int i = 0; i < n; ++i) lhs += p[i] * a[i], rhs += composed[i] * w[i];
		require(lhs == rhs);
		if (n == 257) {
			f[0] = 123;
			p = power_projection(f, w);
			auto shifted = taylor_shift(a, mint(123));
			f[0] = 0;
			composed = compose(shifted, f, n);
			lhs = 0, rhs = 0;
			for (int i = 0; i < n; ++i) lhs += p[i] * a[i], rhs += composed[i] * w[i];
			require(lhs == rhs);
		}
	}
}
