#include "math/linear_algebra/matrix_light.hpp"
#include "math/linear_algebra/matrix.hpp"
#include "math/mint.hpp"
#include "math/fraction.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <random>
#include <type_traits>

template <class A, class B>
void same(const A &a, const B &b) {
	assert(a.rows() == b.rows() && a.cols() == b.cols());
	for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < a.cols(); ++j) {
		if constexpr (std::is_floating_point_v<typename A::value_type>) assert(std::abs(a(i, j) - b(i, j)) < 1e-9 * (1 + std::abs(a(i, j))));
		else assert(a(i, j) == b(i, j));
	}
}

template <class T>
void field_operations(const Matrix<T> &a, const LightMatrix<T> &b) {
	assert(a.rank() == b.rank());
	auto [e, ep] = a.row_echelon();
	auto [f, fp] = b.row_echelon();
	same(e, f);
	assert(ep == fp);
	auto [r, rp] = a.rref();
	auto [s, sp] = b.rref();
	same(r, s);
	assert(rp == sp);
	assert(a.kernel() == b.kernel());
	for (const auto &v : b.kernel()) assert(b * v == std::vector<T>(b.rows(), T(0)));
	for (int kind = 0; kind < 2; ++kind) {
		std::vector<T> rhs(a.rows());
		for (int i = 0; i < a.rows(); ++i) rhs[i] = kind ? T(i + 1) : T(0);
		auto x = a.solve(rhs), y = b.solve(rhs);
		assert(bool(x) == bool(y));
		if (x) {
			assert(x->particular == y->particular && x->basis == y->basis);
			assert(b * y->particular == rhs);
		}
	}
	if (a.rows() != a.cols()) return;
	assert(a.det() == b.det());
	auto x = a.inverse();
	auto y = b.inverse();
	assert(bool(x) == bool(y));
	if (x) {
		same(*x, *y);
		same(b * *y, LightMatrix<T>::identity(b.rows()));
	}
	assert(a.characteristic_polynomial() == b.characteristic_polynomial());
	same(a.adjugate(), b.adjugate());
	same(b * b.adjugate(), LightMatrix<T>::identity(b.rows()) * b.det());
	same(a.pow(13), b.pow(13));
}

int main() {
	std::mt19937 random(1821);
	for (const auto &[n, m] : std::array<std::array<int, 2>, 12>{
		std::array{0, 0}, {0, 3}, {3, 0}, {1, 1}, {1, 5}, {5, 1},
		{2, 2}, {3, 5}, {5, 3}, {5, 5}, {7, 8}, {8, 8}}) {
		for (int kind = 0; kind < 3; ++kind) {
			Matrix<mint> a(n, m);
			LightMatrix<mint> b(n, m);
			for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) a(i, j) = b(i, j) = mint(random() % 7);
			if (kind == 1 && n > 1) for (int j = 0; j < m; ++j) a(n - 1, j) = b(n - 1, j) = a(0, j);
			if (kind == 2) a.fill(0), b.fill(0);
			field_operations(a, b);
			same(a.transpose() * a, b.transpose() * b);
		}
	}
	Matrix<int> integer{ {2, 3, 1}, {4, 1, 5}, {0, 2, 3} };
	LightMatrix<int> integer_light{ {2, 3, 1}, {4, 1, 5}, {0, 2, 3} };
	assert(integer.det() == -42 && integer_light.det() == -42);
	same(integer * integer, integer_light * integer_light);
	Matrix<double> real{ {2, 1}, {1, 3} };
	LightMatrix<double> real_light{ {2, 1}, {1, 3} };
	assert(std::abs(real_light.det() - 5) < 1e-12);
	same(*real.inverse(), *real_light.inverse());
	same(real_light * *real_light.inverse(), LightMatrix<double>::identity(2));
	using Q = Fraction<>;
	Matrix<Q> rational{ {Q(1, 2), Q(1, 3)}, {Q(2, 3), Q(3, 4)} };
	LightMatrix<Q> rational_light{ {Q(1, 2), Q(1, 3)}, {Q(2, 3), Q(3, 4)} };
	field_operations(rational, rational_light);
}
