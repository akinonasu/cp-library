#ifndef GENERALIZED_FLOOR_SUM_HPP
#define GENERALIZED_FLOOR_SUM_HPP
#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace generalized_floor_sum_detail {
using U = std::uint64_t;
template <class T>
constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;
template <int N, class T>
T divide(T value) {
	if constexpr (integer<T>) return value / T(N);
	else {
		static const T inverse = T(1) / T(N);
		return value * inverse;
	}
}
template <int D, class T>
using Table = std::array<std::array<T, D + 1>, D + 1>;

template <int D, class T>
struct Coefficients {
	std::array<std::array<T, D + 2>, D + 1> sums{};
	std::array<std::array<T, D + 2>, D + 2> choose{};
	std::array<T, D + 1> denominator{}, inverse{};
	Coefficients() {
		for (int i = 0; i <= D + 1; ++i) {
			choose[i][0] = choose[i][i] = T(1);
			for (int j = 1; j < i; ++j) choose[i][j] = choose[i - 1][j - 1] + choose[i - 1][j];
		}
		T factorial = 1;
		for (int p = 0; p <= D; ++p) {
			denominator[p] = factorial * T(p + 1);
			for (int k = 0; k <= p + 1; ++k) {
				T value = factorial * choose[p + 1][k];
				for (int j = 0; j < p; ++j)
					if (k <= j + 1) value -= factorial / denominator[j] * choose[p + 1][j] * sums[j][k];
				sums[p][k] = value;
			}
			factorial = denominator[p];
			if constexpr (!integer<T>) inverse[p] = T(1) / denominator[p];
		}
	}
	T divide(T value, int p) const {
		if constexpr (integer<T>) return value / denominator[p];
		else return value * inverse[p];
	}
};

template <int D, class T>
const Coefficients<D, T> &coefficients() {
	static const Coefficients<D, T> value;
	return value;
}

template <int D, class T, class A, class B>
[[gnu::always_inline]] inline void translate(Table<D, T> &result, const Coefficients<D, T> &c, A a, B b,
	std::array<T, D + 1> &ak, std::array<T, D + 1> &bk) {
	if (a != A(0)) {
		ak[0] = T(1);
		for (int i = 1; i <= D; ++i) ak[i] = ak[i - 1] * T(a);
		for (int q = D; q >= 1; --q)
			for (int p = 0; p + q <= D; ++p)
				for (int j = 0; j < q; ++j) result[p][q] += c.choose[q][j] * ak[q - j] * result[p + q - j][j];
	}
	if (b != B(0)) {
		bk[0] = T(1);
		for (int i = 1; i <= D; ++i) bk[i] = bk[i - 1] * T(b);
		for (int q = D; q >= 1; --q)
			for (int p = 0; p + q <= D; ++p)
				for (int j = 0; j < q; ++j) result[p][q] += c.choose[q][j] * bk[q - j] * result[p][j];
	}
}

template <int D, class T>
Table<D, T> euclidean(U n, U m, U a, U b) {
	Table<D, T> result{};
	if (!n) return result;
	if (n == 1) {
		result[0][0] = T(1);
		T value = T(b / m);
		for (int q = 1; q <= D; ++q) result[0][q] = result[0][q - 1] * value;
		return result;
	}
	const auto &c = coefficients<D, T>();
	std::array<T, D + 1> sums{}, ak{}, bk{}, kp{};
	T x = T(n - 1);
	for (int p = 0; p <= D; ++p) {
		T value = c.sums[p][p + 1];
		for (int j = p; j >= 0; --j) value = value * x + c.sums[p][j];
		sums[p] = c.divide(value, p);
	}
	U u = a / m, v = b / m;
	a %= m;
	b %= m;
	if (a) {
		__uint128_t z = __uint128_t(a) * (n - 1) + b;
		U k = z >> 64 ? U(z / m) : U(z) / m;
		if (k) {
			auto child = euclidean<D, T>(k, a, m, m - b - 1);
			kp[0] = T(1);
			for (int j = 1; j <= D; ++j) kp[j] = kp[j - 1] * T(k);
			for (int p = 0; p < D; ++p) {
				std::array<T, D + 1> inner{};
				for (int i = 0; i < D - p; ++i) {
					T value = 0;
					for (int j = 0; j <= p + 1; ++j) value += c.sums[p][j] * child[i][j];
					inner[i] = c.divide(value, p);
				}
				for (int q = 1; p + q <= D; ++q) {
					T value = sums[p] * kp[q];
					for (int i = 0; i < q; ++i) value -= c.choose[q][i] * inner[i];
					result[p][q] = value;
				}
			}
		}
	}
	for (int p = 0; p <= D; ++p) result[p][0] = sums[p];
	translate<D>(result, c, u, v, ak, bk);
	return result;
}

template <class A, class B>
using Signed = std::enable_if_t<std::is_integral_v<A> && std::is_integral_v<B> &&
	(std::is_signed_v<A> || std::is_signed_v<B>), int>;

template <class A>
bool negative(A value) {
	if constexpr (std::is_signed_v<A>) return value < 0;
	else return false;
}

template <class T, class A>
std::pair<U, T> normalize(A value, U m) {
	U x = static_cast<U>(value);
	if (!negative(value)) return {x % m, T(x / m)};
	x = U(0) - x;
	U r = x % m, q = x / m + (r != 0);
	if constexpr (integer<T>) return {r ? m - r : 0, T(-__int128_t(q))};
	else return {r ? m - r : 0, -T(q)};
}

template <class T, bool Full = false>
auto quadratic(U n, U m, U a, U b) {
	std::conditional_t<Full, Table<2, T>, std::array<T, 3>> result{};
	if (!n) return result;
	T count = T(n), s1 = T(n >> 1) * T(n - 1 + (n & 1)), s2 = divide<3>(s1 * (T(2) * count - T(1)));
	U u = a / m, v = b / m;
	a %= m;
	b %= m;
	T f = 0, g = 0, h = 0;
	if (a) {
		__uint128_t z = __uint128_t(a) * (n - 1) + b;
		U k = z >> 64 ? U(z / m) : U(z) / m;
		if (k) {
			auto child = quadratic<T>(k, a, m, m - b - 1);
			T height = T(k);
			f = T(n - 1) * height - child[0];
			g = s1 * height - divide<2>(child[2] + child[0]);
			h = T(n - 1) * height * height - child[0] - T(2) * child[1];
		}
	}
	T at = T(u), bt = T(v);
	T f0 = f + at * s1 + bt * count;
	T f1 = g + at * s2 + bt * s1;
	T f2 = h + T(2) * (at * g + bt * f) + at * at * s2 + bt * (T(2) * at * s1 + bt * count);
	if constexpr (Full) {
		result[0][0] = count;
		result[1][0] = s1;
		result[2][0] = s2;
		result[0][1] = f0;
		result[1][1] = f1;
		result[0][2] = f2;
	} else result = {f0, f1, f2};
	return result;
}

template <class T>
T linear(U n, U m, U a, U b) {
	T result = 0;
	for (;;) {
		if (!n) return result;
		if (n == 1) return result + T(b / m);
		if (a >= m) result += divide<2>(T(n) * T(n - 1)) * T(a / m), a %= m;
		if (b >= m) result += T(n) * T(b / m), b %= m;
		__uint128_t z = __uint128_t(a) * n + b;
		if (z < m) return result;
		if (z >> 64) n = z / m, b = z % m;
		else {
			U value = z;
			n = value / m;
			b = value % m;
		}
		std::swap(a, m);
	}
}
}

template <int D, class T = __int128_t>
auto generalized_floor_sum_moments(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b) {
	static_assert(D >= 0);
	assert(m > 0);
	if constexpr (D == 0) return generalized_floor_sum_detail::Table<0, T>{std::array<T, 1>{T(n)}};
	else if constexpr (D == 1) {
		generalized_floor_sum_detail::Table<1, T> result{};
		result[0][0] = T(n);
		result[1][0] = n ? generalized_floor_sum_detail::divide<2>(T(n) * T(n - 1)) : T(0);
		result[0][1] = generalized_floor_sum_detail::linear<T>(n, m, a, b);
		return result;
	} else if constexpr (D == 2) return generalized_floor_sum_detail::quadratic<T, true>(n, m, a, b);
	else return generalized_floor_sum_detail::euclidean<D, T>(n, m, a, b);
}
template <int P, int Q, class T = __int128_t>
T generalized_floor_sum(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b) {
	static_assert(P >= 0 && Q >= 0);
	assert(m > 0);
	if constexpr (Q == 0) {
		if (!n) return T(0);
		if constexpr (P == 0) return T(n);
		else if constexpr (P == 1) return generalized_floor_sum_detail::divide<2>(T(n) * T(n - 1));
		else if constexpr (P == 2) return generalized_floor_sum_detail::divide<6>(T(n) * T(n - 1) * (T(2) * T(n) - T(1)));
		else {
			const auto &c = generalized_floor_sum_detail::coefficients<P, T>();
			T value = c.sums[P][P + 1], x = T(n - 1);
			for (int j = P; j >= 0; --j) value = value * x + c.sums[P][j];
			return c.divide(value, P);
		}
	} else if constexpr (P == 0 && Q == 1) return generalized_floor_sum_detail::linear<T>(n, m, a, b);
	else return generalized_floor_sum_moments<P + Q, T>(n, m, a, b)[P][Q];
}

template <int D, class T = __int128_t, class A, class B, generalized_floor_sum_detail::Signed<A, B> = 0>
auto generalized_floor_sum_moments(std::uint64_t n, std::uint64_t m, A a, B b) {
	using namespace generalized_floor_sum_detail;
	static_assert(D >= 0);
	assert(m > 0);
	if (!negative(a) && !negative(b)) return generalized_floor_sum_moments<D, T>(n, m, U(a), U(b));
	if constexpr (D == 0) return generalized_floor_sum_moments<0, T>(n, m, U(0), U(0));
	else {
		if (!n) return Table<D, T>{};
		auto [rb, v] = normalize<T>(b, m);
		if (n == 1) {
			Table<D, T> result{};
			result[0][0] = T(1);
			for (int q = 1; q <= D; ++q) result[0][q] = result[0][q - 1] * v;
			return result;
		}
		auto [ra, u] = normalize<T>(a, m);
		auto result = generalized_floor_sum_moments<D, T>(n, m, ra, rb);
		std::array<T, D + 1> ak{}, bk{};
		translate<D>(result, coefficients<D, T>(), u, v, ak, bk);
		return result;
	}
}

template <int P, int Q, class T = __int128_t, class A, class B, generalized_floor_sum_detail::Signed<A, B> = 0>
T generalized_floor_sum(std::uint64_t n, std::uint64_t m, A a, B b) {
	using namespace generalized_floor_sum_detail;
	static_assert(P >= 0 && Q >= 0);
	assert(m > 0);
	if constexpr (Q == 0) return generalized_floor_sum<P, Q, T>(n, m, U(0), U(0));
	else {
		if (!negative(a) && !negative(b)) return generalized_floor_sum<P, Q, T>(n, m, U(a), U(b));
		return generalized_floor_sum_moments<P + Q, T>(n, m, a, b)[P][Q];
	}
}
#endif
