#include "math/generalized_floor_sum.hpp"
namespace generalized_floor_sum_detail {
template <class T>
Table<2, T> quadratic_baseline(U n, U m, U a, U b) {
	Table<2, T> result{};
	if (!n) return result;
	T count = T(n), s1 = divide<2>(count * T(n - 1)), s2 = divide<3>(s1 * (T(2) * count - T(1)));
	U u = a / m, v = b / m;
	a %= m;
	b %= m;
	T f = 0, g = 0, h = 0;
	if (a) {
		__uint128_t z = __uint128_t(a) * (n - 1) + b;
		U k = z >> 64 ? U(z / m) : U(z) / m;
		if (k) {
			auto child = quadratic_baseline<T>(k, a, m, m - b - 1);
			T height = T(k);
			f = T(n - 1) * height - child[0][1];
			g = s1 * height - divide<2>(child[0][2] + child[0][1]);
			h = T(n - 1) * height * height - child[0][1] - T(2) * child[1][1];
		}
	}
	T at = T(u), bt = T(v);
	result[0][0] = count;
	result[1][0] = s1;
	result[2][0] = s2;
	result[0][1] = f + at * s1 + bt * count;
	result[1][1] = g + at * s2 + bt * s1;
	result[0][2] = h + T(2) * (at * g + bt * f) + at * at * s2 + bt * (T(2) * at * s1 + bt * count);
	return result;
}

}
