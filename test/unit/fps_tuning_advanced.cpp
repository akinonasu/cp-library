#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/power_projection.hpp"
#include "fps/linear_recurrence.hpp"
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <random>

void require(bool ok, int line = __builtin_LINE()) {
	if (!ok) std::fprintf(stderr, "%d\n", line), std::abort();
}

fps product(const fps &a, const fps &b, int n) {
	fps result(n);
	for (int i = 0; i < static_cast<int>(a.size()) && i < n; ++i)
		for (int j = 0; j < static_cast<int>(b.size()) && i + j < n; ++j) result[i + j] += a[i] * b[j];
	return result;
}

int main() {
	std::mt19937 random(731);
	for (int n : {2, 3, 8, 31, 33, 65, 129}) {
		for (int stride : {2, 3, 4, 7, 16}) {
			fps f(n), g(n), weights(n), expected(n), projections(n + 5), power(n);
			for (auto &x : f) x = random();
			for (auto &x : weights) x = random();
			for (int i = stride; i < n; i += stride) g[i] = random();
			power[0] = 1;
			for (int k = 0; k < n + 5; ++k) {
				for (int i = 0; i < n; ++i) {
					if (k < n) expected[i] += f[k] * power[i];
					projections[k] += weights[i] * power[i];
				}
				power = product(power, g, n);
			}
			require(compose(f, g) == expected);
			require(power_projection(g, weights, n + 5) == projections);
			g[0] = 19;
			auto shifted = power_projection(g, weights, n + 5);
			power.assign(n, 0);
			power[0] = 1;
			for (int k = 0; k < n + 5; ++k) {
				mint value = 0;
				for (int i = 0; i < n; ++i) value += weights[i] * power[i];
				require(shifted[k] == value);
				power = product(power, g, n);
			}
			if (n > 2) {
				g.assign(n, 0);
				g[n - 2] = 3;
				g[n - 1] = 5;
				expected.assign(n, 0);
				expected[0] = f[0];
				power.assign(n, 0);
				power[0] = 1;
				for (int k = 1; k < n; ++k) {
					power = product(power, g, n);
					for (int i = 0; i < n; ++i) expected[i] += f[k] * power[i];
				}
				require(compose(f, g) == expected);
				if (n > 3) {
					f.assign(n, 0);
					f.back() = 7;
					require(compose(f, g) == fps(n));
				}
			}
		}
	}
	for (int d : {0, 1, 2, 3, 8, 31, 33, 65, 257}) {
		for (int order : {0, 1, 2, 3, 8, 31, 257}) {
			if (order > d) continue;
			std::vector<mint> a(d), coefficients(d), sequence(d + 513);
			for (int i = 0; i < d; ++i) a[i] = sequence[i] = random();
			for (int i = 0; i < order; ++i) coefficients[i] = random();
			for (int i = d; i < static_cast<int>(sequence.size()); ++i)
				for (int j = 0; j < order; ++j) sequence[i] += coefficients[j] * sequence[i - j - 1];
			for (int k : {0, d, d + 1, d + 31, d + 128, d + 512}) require(linear_recurrence(a, coefficients, k) == sequence[k]);
			if (order == 1) require(linear_recurrence(a, coefficients, LLONG_MAX) == a.back() * coefficients[0].pow(LLONG_MAX - d + 1));
		}
	}
	for (int n : {33, 257, 1025, 4097}) {
		fps f(n), identity(n);
		for (int i = 1; i < n; ++i) f[i] = random();
		if (f[1] == mint(0)) f[1] = 1;
		identity[1] = 1;
		auto inverse = compositional_inverse(f);
		require(compose(f, inverse) == identity);
		require(compose(inverse, f) == identity);
	}
}
