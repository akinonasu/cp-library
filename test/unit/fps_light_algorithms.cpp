#include "fps/fps_light_algorithms.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

using F = light_fps;
using M = mint;
namespace A = light_fps_algorithms;

M evaluate(const F &f, M x) {
	M value = 0, power = 1;
	for (M coefficient : f) { value += coefficient * power; power *= x; }
	return value;
}

void points() {
	std::mt19937 rng(733901);
	for (int n : {0, 1, 2, 3, 7, 8, 17, 31, 32, 33, 63, 65, 100, 129, 257}) {
		for (int trial = 0; trial < 6; ++trial) {
			F f(n);
			for (auto &x : f) x = M(rng());
			std::vector<M> xs(n), ys(n);
			for (int i = 0; i < n; ++i) xs[i] = M(i);
			std::shuffle(xs.begin(), xs.end(), rng);
			for (int i = 0; i < n; ++i) ys[i] = evaluate(f, xs[i]);
			assert(A::multipoint_eval(f, xs) == ys);
			assert(A::interpolate(xs, ys) == f);
			for (auto &x : xs) x = M(rng() % 13);
			for (int i = 0; i < n; ++i) ys[i] = evaluate(f, xs[i]);
			assert(A::multipoint_eval(f, xs) == ys);
			assert(A::multipoint_eval(F(), xs) == std::vector<M>(n));
			assert(A::multipoint_eval(F(n), xs) == std::vector<M>(n));
			F large(2 * n + 3);
			for (auto &x : large) x = M(rng());
			for (int i = 0; i < n; ++i) ys[i] = evaluate(large, xs[i]);
			assert(A::multipoint_eval(large, xs) == ys);
		}
	}
}

void shift() {
	std::mt19937 rng(649351);
	for (int n : {0, 1, 2, 3, 7, 8, 31, 32, 33, 63, 65, 100, 129}) {
		for (int trial = 0; trial < 10; ++trial) {
			F f(n), expected(n);
			for (auto &x : f) x = M(rng());
			M c = trial == 0 ? M(0) : M(rng());
			std::vector<M> binomial{M(1)};
			for (int i = 0; i < n; ++i) {
				M power = 1;
				for (int j = i; j >= 0; --j) {
					expected[j] += f[i] * binomial[j] * power;
					power *= c;
				}
				binomial.push_back(M(0));
				for (int j = i + 1; j; --j) binomial[j] += binomial[j - 1];
			}
			assert(A::taylor_shift(f, c) == expected);
		}
	}
}

void rational() {
	std::mt19937 rng(857912);
	const std::array<int, 15> sizes{1, 2, 3, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257};
	for (int trial = 0; trial < 100; ++trial) {
		int n = trial < int(sizes.size()) ? sizes[trial] : 1 + rng() % 40;
		F p(trial < int(sizes.size()) ? 2 * n + 3 : rng() % 80), q(n);
		for (auto &x : p) x = M(rng());
		for (auto &x : q) x = M(rng());
		q[0] = M(1 + rng() % (M::mod() - 1));
		if (trial % 4 == 1) for (std::size_t i = std::max<std::size_t>(1, q.size() / 2); i < q.size(); ++i) q[i] = M(0);
		if (trial % 4 >= 2) for (std::size_t i = trial % 4 - 1; i < q.size(); i += 2) q[i] = M(0);
		std::vector<M> expected(std::max(200, 2 * n + 50));
		for (int i = 0; i < int(expected.size()); ++i) {
			M value = std::size_t(i) < p.size() ? p[i] : M(0);
			for (int j = 1; j <= i && std::size_t(j) < q.size(); ++j) value -= q[j] * expected[i - j];
			expected[i] = value / q[0];
		}
		for (int k : {0, 1, 2, 3, 4, 7, 31, 32, 63, 64, 127, 199})
			assert(A::bostan_mori(p, q, k) == expected[k]);
		assert(A::bostan_mori(p, q, int(expected.size()) - 1) == expected.back());
	}
	long long k = std::numeric_limits<long long>::max();
	for (int d : sizes) {
		F q(d + 1);
		q[0] = 1;
		q[d] = -3;
		for (long long index : {0LL, 1LL, 999999999999999999LL, k})
			assert(A::bostan_mori(F{M(7)}, q, index) == (index % d ? M(0) : M(7) * M(3).pow(index / d)));
	}
	assert(A::bostan_mori(F{M(7)}, F{M(1), M(-3)}, k) == M(7) * M(3).pow(k));
	assert(A::bostan_mori(F{M(7)}, F{M(1)}, k) == M(0));
	assert(A::bostan_mori(F(), F{M(1), M(2)}, k) == M(0));
}

void recurrence() {
	std::mt19937 rng(912634);
	for (int n : {0, 1, 2, 3, 7, 8, 17, 31, 32, 33, 63}) {
		for (int trial = 0; trial < 10; ++trial) {
			std::vector<M> initial(n), coefficients(n), expected(301);
			for (auto &x : initial) x = M(rng());
			for (auto &x : coefficients) x = M(rng());
			if (trial & 1) for (int i = n / 2; i < n; ++i) coefficients[i] = M(0);
			std::copy(initial.begin(), initial.end(), expected.begin());
			for (int i = n; i <= 300; ++i)
				for (int j = 0; j < n; ++j) expected[i] += coefficients[j] * expected[i - 1 - j];
			for (int k : {0, 1, 2, 7, 31, 32, 63, 100, 199, 300})
				assert(A::linear_recurrence(initial, coefficients, k) == expected[k]);
		}
	}
}

int main() {
	points();
	shift();
	rational();
	recurrence();
}
