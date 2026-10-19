#ifndef GENERALIZED_FLOOR_SUM_MONOID_HPP
#define GENERALIZED_FLOOR_SUM_MONOID_HPP
#include "math/floor_sum/generalized_floor_sum.hpp"
#include "math/floor_sum/monoid_floor_sum.hpp"

namespace generalized_floor_sum_benchmark {
template <int D, class T>
struct Monoid {
	using Table = generalized_floor_sum_detail::Table<D, T>;
	struct Node {
		Table sums{};
		T x = 0, y = 0;
	};
	static Node op(Node a, Node b) {
		const auto &c = generalized_floor_sum_detail::coefficients<D, T>();
		std::array<T, D + 1> xp{}, yp{};
		xp[0] = yp[0] = T(1);
		for (int i = 1; i <= D; ++i) xp[i] = xp[i - 1] * a.x, yp[i] = yp[i - 1] * a.y;
		for (int p = 0; p <= D; ++p)
			for (int q = D - p; q >= 1; --q)
				for (int j = 0; j < q; ++j) b.sums[p][q] += c.choose[q][j] * yp[q - j] * b.sums[p][j];
		for (int q = 0; q <= D; ++q)
			for (int p = 0; p + q <= D; ++p)
				for (int j = 0; j <= p; ++j) a.sums[p][q] += c.choose[p][j] * xp[p - j] * b.sums[j][q];
		a.x += b.x;
		a.y += b.y;
		return a;
	}
	static Node e() { return {}; }
	static Table run(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b) {
		Node x, y;
		x.x = T(1);
		x.sums[0][0] = T(1);
		y.y = T(1);
		return monoid_floor_sum<Node, op, e>(n, m, a, b, x, y).sums;
	}
};
}

namespace generalized_floor_sum_benchmark {
template <int P, int Q, class T>
struct RectangularMonoid {
	using Table = std::array<std::array<T, Q + 1>, P + 1>;
	struct Node {
		Table sums{};
		T x = 0, y = 0;
	};
	static Node op(Node a, Node b) {
		const auto &c = generalized_floor_sum_detail::coefficients<(P > Q ? P : Q), T>();
		std::array<T, P + 1> xp{};
		std::array<T, Q + 1> yp{};
		xp[0] = yp[0] = T(1);
		for (int i = 1; i <= P; ++i) xp[i] = xp[i - 1] * a.x;
		for (int i = 1; i <= Q; ++i) yp[i] = yp[i - 1] * a.y;
		for (int p = 0; p <= P; ++p)
			for (int q = Q; q >= 1; --q)
				for (int j = 0; j < q; ++j) b.sums[p][q] += c.choose[q][j] * yp[q - j] * b.sums[p][j];
		for (int q = 0; q <= Q; ++q)
			for (int p = 0; p <= P; ++p)
				for (int j = 0; j <= p; ++j) a.sums[p][q] += c.choose[p][j] * xp[p - j] * b.sums[j][q];
		a.x += b.x;
		a.y += b.y;
		return a;
	}
	static Node e() { return {}; }
	static Table run(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b) {
		Node x, y;
		x.x = T(1);
		x.sums[0][0] = T(1);
		y.y = T(1);
		return monoid_floor_sum<Node, op, e>(n, m, a, b, x, y).sums;
	}
};
}

#endif
