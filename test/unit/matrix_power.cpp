#include "math/linear_algebra/matrix.hpp"
#include "math/mint.hpp"
#include "test/fixtures/recurrence_types.hpp"
#include <cassert>
#include <limits>
#include <random>

template <class T>
Matrix<T> reference_product(const Matrix<T> &a, const Matrix<T> &b) {
	Matrix<T> result(a.rows(), b.cols());
	for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < b.cols(); ++j)
		for (int k = 0; k < a.cols(); ++k) result(i, j) += a(i, k) * b(k, j);
	return result;
}

template <class T>
Matrix<T> reference_power(Matrix<T> a, std::uint64_t k) {
	auto result = Matrix<T>::identity(a.rows());
	for (; k; k >>= 1) {
		if (k & 1) result = reference_product(result, a);
		if (k > 1) a = reference_product(a, a);
	}
	return result;
}

template <class T>
void check(const Matrix<T> &a, std::uint64_t k) {
	auto expected = reference_power(a, k);
	assert(a.pow(k) == expected);
	if constexpr (matrix_detail::small_modint<T>) if constexpr (matrix_detail::prime_modulus(T::mod())) {
		if (!a.rows()) return;
		std::vector<T> values(a.data(), a.data() + std::size_t(a.rows()) * a.cols());
		if (auto fast = matrix_detail::FrobeniusPower<T>(values, a.rows()).run(k))
			for (std::size_t i = 0; i < fast->size(); ++i) assert((*fast)[i] == expected.data()[i]);
	}
}

template <class T>
void modular() {
	std::mt19937_64 random(9982);
	for (int n : {0, 1, 2, 3, 7, 8, 15, 16, 17, 31, 32, 33}) {
		Matrix<T> a(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) a(i, j) = T(random() % T::mod());
		for (std::uint64_t k : {0ULL, 1ULL, 2ULL, 255ULL, 256ULL, 1000000000000000000ULL}) check(a, k);
	}
	for (int kind = 0; kind < 7; ++kind) {
		int n = kind == 6 ? 65 : 17;
		Matrix<T> a(n);
		for (int i = 0; i < n; ++i) {
			if (kind != 0 && kind != 3) a(i, i) = kind == 1 ? T(1) : kind == 2 ? T(i) : T(7);
			if (kind >= 3 && i + 1 < n && (kind == 3 || i % 3)) a(i + 1, i) = 1;
		}
		if (kind == 5) for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) a(i, j) = T(i + 1) * T(j + 1);
		for (int step = 0; step < 2 * n; ++step) {
			int i = random() % n, j = random() % n;
			if (i == j) continue;
			T c = T(random() % T::mod());
			for (int col = 0; col < n; ++col) a(i, col) += c * a(j, col);
			for (int row = 0; row < n; ++row) a(row, j) -= c * a(row, i);
		}
		check(a, kind == 6 ? 1023 : std::numeric_limits<std::uint64_t>::max());
	}
}

int main() {
	static_assert(matrix_detail::prime_modulus(2));
	static_assert(matrix_detail::prime_modulus(998244353));
	static_assert(matrix_detail::prime_modulus(2147483647));
	static_assert(!matrix_detail::prime_modulus(1));
	static_assert(!matrix_detail::prime_modulus(15));
	static_assert(!matrix_detail::prime_modulus(2147395600));
	modular<mint>();
	modular<RecurrenceModint<2>>();
	modular<RecurrenceModint<3>>();
	modular<RecurrenceModint<1000000007>>();
	modular<RecurrenceModint<2147483647>>();
	std::vector<mint> jordan{1, 1, 0, 1};
	matrix_detail::FrobeniusPower<mint> failed(jordan, 2);
	failed.random = 0;
	assert(!failed.run(3));
	check(Matrix<mint>{ {1, 1}, {0, 1} }, 3);
	Matrix<RecurrenceModint<15>> ring(17);
	for (int i = 0; i < 17; ++i) for (int j = 0; j < 17; ++j) ring(i, j) = i * j + i + j;
	check(ring, 1000000000000000000ULL);
	Matrix<int> integer(17);
	for (int i = 0; i + 1 < 17; ++i) integer(i, i + 1) = 1;
	check(integer, std::numeric_limits<std::uint64_t>::max());
	Matrix<double> real(17);
	for (int i = 0; i < 17; ++i) real(i, i) = i % 2 ? 1 : -1;
	check(real, std::numeric_limits<std::uint64_t>::max());
}
