#include "fps/berlekamp_massey.hpp"
#include "math/mint.hpp"
#include "math/fraction.hpp"
#include "test/fixtures/recurrence_types.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

template <class T>
bool possible(const std::vector<T> &a, int d) {
	int n = int(a.size()), h = n - d, row = 0;
	std::vector<std::vector<T>> matrix(h, std::vector<T>(d + 1));
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < d; ++j) matrix[i][j] = a[i + d - j - 1];
		matrix[i][d] = a[i + d];
	}
	for (int col = 0; col < d && row < h; ++col) {
		int pivot = row;
		while (pivot < h && matrix[pivot][col] == T(0)) ++pivot;
		if (pivot == h) continue;
		matrix[row].swap(matrix[pivot]);
		T inverse = T(1) / matrix[row][col];
		for (int j = col; j <= d; ++j) matrix[row][j] *= inverse;
		for (int i = row + 1; i < h; ++i) {
			T factor = matrix[i][col];
			for (int j = col; j <= d; ++j) matrix[i][j] = matrix[i][j] - factor * matrix[row][j];
		}
		++row;
	}
	for (int i = row; i < h; ++i) if (matrix[i][d] != T(0)) return false;
	return true;
}

template <class T>
void validate(const std::vector<T> &a, const std::vector<T> &c) {
	assert(c.size() <= a.size());
	for (std::size_t i = c.size(); i < a.size(); ++i) {
		T value = 0;
		for (std::size_t j = 0; j < c.size(); ++j) value += c[j] * a[i - j - 1];
		assert(value == a[i]);
	}
}

template <class T>
void check(const std::vector<T> &a, bool minimal = true) {
	auto original = a;
	auto c = berlekamp_massey(a);
	validate(a, c);
	if (minimal) for (int d = 0; d < int(c.size()); ++d) assert(!possible(a, d));
	assert(a == original);
}

template <unsigned P>
void exhaustive(int bound) {
	using T = RecurrenceModint<P>;
	for (int n = 0, count = 1; n <= bound; ++n, count *= P)
		for (int code = 0; code < count; ++code) {
			std::vector<T> a(n);
			for (int i = 0, x = code; i < n; ++i, x /= P) a[i] = T(x % P);
			check(a);
		}
}

template <class T>
void random_cases() {
	std::mt19937_64 random(9982);
	for (int n : {0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 511})
		for (int kind = 0; kind < 6; ++kind) {
			std::vector<T> a(n);
			for (int i = 0; i < n; ++i) {
				a[i] = kind == 0 ? T(0) : kind == 1 ? T(T::mod() - 1) : T(random() % T::mod());
				if (kind == 3 && i < n / 2) a[i] = 0;
				if (kind == 4 && i >= n / 2) a[i] = 0;
				if (kind == 5) a[i] = i == n - 1 ? T(1) : T(0);
			}
			check(a, n <= 10);
			std::vector<RecurrenceGeneric<T>> b(a.begin(), a.end());
			auto fast = berlekamp_massey(a);
			auto generic = berlekamp_massey(b);
			assert(fast.size() == generic.size());
			for (std::size_t i = 0; i < fast.size(); ++i) assert(fast[i] == generic[i].value);
		}
	for (int d : {1, 2, 3, 8, 16, 33, 65}) {
		std::vector<T> c(d), a(2 * d + 80);
		for (auto &x : c) x = T(random() % T::mod());
		for (int i = 0; i < d; ++i) a[i] = T(random() % T::mod());
		for (int i = d; i < int(a.size()); ++i)
			for (int j = 0; j < d; ++j) a[i] += c[j] * a[i - j - 1];
		std::vector<T> prefix(a.begin(), a.begin() + 2 * d);
		auto recovered = berlekamp_massey(prefix);
		assert(recovered.size() <= c.size());
		validate(a, recovered);
	}
}

int main() {
	exhaustive<2>(9);
	exhaustive<3>(7);
	exhaustive<7>(4);
	assert(berlekamp_massey(std::vector<mint>{}).empty());
	assert(berlekamp_massey(std::vector<mint>(100)).empty());
	assert(berlekamp_massey(std::vector<mint>{1, 0, 0}) == std::vector<mint>{0});
	assert(berlekamp_massey(std::vector<mint>{0, 1, 1, 2, 3, 5, 8, 13}) == std::vector<mint>({1, 1}));
	assert(berlekamp_massey(std::vector<mint>{10, 1, 1, 2, 3, 5, 8, 13}) == std::vector<mint>({1, 1, 0}));
	for (int n : {1, 2, 3, 31, 32, 33, 127, 128, 129, 1000}) {
		std::vector<mint> a(n);
		a.back() = 1;
		assert(berlekamp_massey(a).size() == a.size());
	}
	random_cases<mint>();
	random_cases<RecurrenceModint<2>>();
	random_cases<RecurrenceModint<3>>();
	random_cases<RecurrenceModint<1000000007>>();
	random_cases<RecurrenceModint<1073741789>>();
	random_cases<RecurrenceModint<2147483647>>();
	using Rational = Fraction<>;
	for (const auto &a : std::vector<std::vector<Rational>>{
		{}, {0}, {1, 0, 0}, {Rational(1, 2), Rational(1, 4), Rational(1, 8), Rational(1, 16)},
		{1, 2, 4, 8}, {1, 2, 4, 7}, {0, 1, 1, 2, 3, 5, 8, 13}}) check(a);
}
