#if defined(CP_FPS_MATRIX_LIGHT_FIRST)
#include "math/linear_algebra/matrix_light.hpp"
#include "fps/fps_light.hpp"
#include "fps/fps.hpp"
#include "math/linear_algebra/matrix.hpp"
#else
#include "math/linear_algebra/matrix.hpp"
#include "fps/fps.hpp"
#include "fps/fps_light.hpp"
#include "math/linear_algebra/matrix_light.hpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

using Polynomial = std::vector<mint>;
using PolynomialMatrix = std::vector<std::vector<Polynomial>>;

PolynomialMatrix reference_product(const PolynomialMatrix &a, const PolynomialMatrix &b, int degree) {
	const int n = int(a.size());
	PolynomialMatrix result(n, std::vector<Polynomial>(n, Polynomial(degree)));
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) for (int k = 0; k < n; ++k)
		for (int x = 0; x < std::min<int>(degree, a[i][k].size()); ++x)
			for (int y = 0; x + y < degree && y < int(b[k][j].size()); ++y)
				result[i][j][x + y] += a[i][k][x] * b[k][j][y];
	return result;
}

PolynomialMatrix reference_power(PolynomialMatrix a, std::uint64_t k, int degree) {
	const int n = int(a.size());
	PolynomialMatrix result(n, std::vector<Polynomial>(n, Polynomial(degree)));
	if (degree) for (int i = 0; i < n; ++i) result[i][i][0] = 1;
	for (; k; k >>= 1) {
		if (k & 1) result = reference_product(result, a, degree);
		if (k > 1) a = reference_product(a, a, degree);
	}
	return result;
}

template <class M>
void matches(const M &actual, const PolynomialMatrix &expected, int degree) {
	assert(actual.rows() == int(expected.size()) && actual.cols() == actual.rows());
	for (int i = 0; i < actual.rows(); ++i) for (int j = 0; j < actual.cols(); ++j) {
		const auto &f = actual(i, j);
		assert(f.size() <= std::size_t(degree));
		for (int d = 0; d < degree; ++d) assert((d < int(f.size()) ? f[d] : mint(0)) == expected[i][j][d]);
	}
}

template <class M>
void check() {
	using F = typename M::value_type;
	M a{ {F{1}, F{0, 1}}, {F{0, 1}, F{1}} };
	PolynomialMatrix original{{{1}, {0, 1}}, {{0, 1}, {1}}};
	for (std::uint64_t k = 0; k <= 13; ++k) matches(a.pow(k), reference_power(original, k, int(k) + 1), int(k) + 1);
	auto identity = a.pow(0);
	assert(identity(0, 0).size() == 1 && identity(0, 0)[0] == mint(1));
	assert(identity(1, 1).size() == 1 && identity(1, 1)[0] == mint(1));
	for (int degree : {0, 1, 2, 7, 16, 32}) {
		M b = a;
		b(0, 0).resize(degree + 4);
		b(0, 0)[degree + 3] = 17;
		int calls = 0;
		bool oversized_seen = false;
		auto normalize = [&](M &matrix) {
			++calls;
			if (matrix(0, 0).size() == std::size_t(degree + 4)) oversized_seen = true;
			for (int i = 0; i < matrix.rows(); ++i) for (int j = 0; j < matrix.cols(); ++j)
				if (matrix(i, j).size() > std::size_t(degree)) matrix(i, j).resize(degree);
		};
		for (std::uint64_t k : {std::uint64_t(0), std::uint64_t(1000000000000000000ULL), std::numeric_limits<std::uint64_t>::max()}) {
			calls = 0;
			oversized_seen = false;
			PolynomialMatrix expected(2, std::vector<Polynomial>(2, Polynomial(degree)));
			mint coefficient = 1, reduced = mint::raw(k % mint::mod());
			for (int i = 0; i < degree; ++i) {
				expected[0][i % 2][i] = expected[1][1 - i % 2][i] = coefficient;
				coefficient *= (reduced - mint(i)) / mint(i + 1);
			}
			matches(b.pow(k, normalize), expected, degree);
			assert(calls && oversized_seen);
		}
	}
	std::mt19937 random(341982);
	for (int degree : {0, 1, 3, 8, 17, 65}) for (int trial = 0; trial < 3; ++trial) {
		M b(3);
		PolynomialMatrix input(3, std::vector<Polynomial>(3));
		for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
			input[i][j].resize(1 + random() % 9);
			for (auto &x : input[i][j]) x = mint(int(random() % 11) - 5);
			b(i, j) = F(input[i][j]);
		}
		auto normalize = [degree](M &matrix) {
			for (int i = 0; i < matrix.rows(); ++i) for (int j = 0; j < matrix.cols(); ++j)
				if (matrix(i, j).size() > std::size_t(degree)) matrix(i, j).resize(degree);
		};
		for (std::uint64_t k : {0, 2, 13}) matches(b.pow(k, normalize), reference_power(input, k, degree), degree);
	}
	M empty;
	assert(empty.pow(0).rows() == 0);
}

int main() {
	check<Matrix<fps>>();
	check<Matrix<light_fps>>();
	check<LightMatrix<fps>>();
	check<LightMatrix<light_fps>>();
}
