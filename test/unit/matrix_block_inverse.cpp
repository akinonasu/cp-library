#include "math/linear_algebra/matrix.hpp"
#include "math/mint.hpp"
#include "test/fixtures/recurrence_types.hpp"
#include <cassert>
#include <random>

template <class T>
void check() {
	std::mt19937_64 random(T::mod());
	for (int n : {111, 112, 113, 224, 225}) for (int deficiency = 0; deficiency < 3; ++deficiency) {
		Matrix<T> a(n), adjugate(n);
		T determinant = 1;
		int zero = n / 3;
		for (int i = 0; i < n; ++i) {
			a(i, i) = T(random() % (T::mod() - 1) + 1);
			if (deficiency && i == zero) a(i, i) = 0;
			if (deficiency == 2 && i == zero + 1) a(i, i) = 0;
			if (a(i, i) != T(0)) determinant *= a(i, i);
		}
		if (!deficiency) for (int i = 0; i < n; ++i) adjugate(i, i) = determinant / a(i, i);
		if (deficiency == 1) adjugate(zero, zero) = determinant;
		if (deficiency) determinant = 0;
		for (int step = 0; step < n * 3; ++step) {
			int i = random() % n, j = random() % n;
			if (i == j) continue;
			T c = T(random() % T::mod());
			if (step & 1) for (int k = 0; k < n; ++k) {
				a(i, k) += c * a(j, k);
				adjugate(k, j) -= c * adjugate(k, i);
			} else for (int k = 0; k < n; ++k) {
				a(k, i) += c * a(k, j);
				adjugate(j, k) -= c * adjugate(i, k);
			}
		}
		assert(a.det() == determinant);
		assert(a.adjugate() == adjugate);
		auto inverse = a.inverse();
		assert(bool(inverse) == !deficiency);
		if (inverse) assert(*inverse == adjugate / determinant);
	}
	for (int n : {112, 224}) {
		Matrix<T> permutation(n);
		for (int i = 0; i < n; ++i) permutation(i, (i + n / 2) % n) = 1;
		assert(permutation.inverse() == std::optional(permutation));
		assert(permutation.adjugate() == permutation * permutation.det());
	}
}

int main() {
	check<mint>();
	check<RecurrenceModint<2>>();
	check<RecurrenceModint<1000000007>>();
}
