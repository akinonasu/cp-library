#include "fps/bostan_mori.hpp"
#include "fps/linear_recurrence.hpp"
#include <climits>
#include <random>

int main() {
	using T = fps_mint;
	using F = FormalPowerSeries<T>;
	std::mt19937 rng(61429);
	for (int d : {31, 32, 33, 63, 64, 65, 1023, 1024, 1025, 65535, 65536, 65537, 100000}) {
		F p(d), q(d + 1);
		for (T &x : p) x = T::raw(rng() % T::mod());
		const T c = T::raw(rng() % (T::mod() - 1) + 1);
		q[0] = 7;
		q[d] = -q[0] * c;
		for (long long k : {3LL * d + 17, LLONG_MAX}) {
			const T expected = p[k % d] / q[0] * c.pow(k / d);
			assert(bostan_mori(p, q, k) == expected);
		}
		std::vector<T> coefficients(d);
		coefficients.back() = c;
		const long long k = LLONG_MAX - 37;
		assert(linear_recurrence<T>(p, coefficients, k) == p[k % d] * c.pow(k / d));
	}
}
