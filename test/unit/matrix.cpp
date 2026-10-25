#include "math/matrix.hpp"
#include "math/mint.hpp"
#include "math/fraction.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include <vector>

template <std::uint32_t P>
struct Mod {
	std::uint32_t x = 0;
	Mod() = default;
	Mod(long long v) : x((v % P + P) % P) {}
	static constexpr std::uint32_t mod() { return P; }
	static Mod raw(std::uint32_t v) { Mod a; a.x = v; return a; }
	std::uint32_t val() const { return x; }
	Mod operator-() const { return raw(x ? P - x : 0); }
	Mod &operator+=(Mod b) { x += b.x; if (x >= P) x -= P; return *this; }
	Mod &operator-=(Mod b) { x = x >= b.x ? x - b.x : x + P - b.x; return *this; }
	Mod &operator*=(Mod b) { x = std::uint64_t(x) * b.x % P; return *this; }
	Mod &operator/=(Mod b) { Mod p = 1; for (auto k = P - 2; k; k >>= 1, b *= b) if (k & 1) p *= b; return *this *= p; }
	friend Mod operator+(Mod a, Mod b) { return a += b; }
	friend Mod operator-(Mod a, Mod b) { return a -= b; }
	friend Mod operator*(Mod a, Mod b) { return a *= b; }
	friend Mod operator/(Mod a, Mod b) { return a /= b; }
	bool operator==(const Mod &) const = default;
};

template <class T>
T brute_det(const Matrix<T> &a) {
	int n = a.rows();
	std::vector<int> p(n);
	std::iota(p.begin(), p.end(), 0);
	T sum = 0;
	do {
		T product = 1;
		int inversions = 0;
		for (int i = 0; i < n; ++i) {
			product *= a(i, p[i]);
			for (int j = i + 1; j < n; ++j) inversions += p[i] > p[j];
		}
		sum += inversions & 1 ? -product : product;
	} while (std::next_permutation(p.begin(), p.end()));
	return sum;
}

template <class T>
Matrix<T> brute_product(const Matrix<T> &a, const Matrix<T> &b) {
	Matrix<T> c(a.rows(), b.cols());
	for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < b.cols(); ++j)
		for (int k = 0; k < a.cols(); ++k) c(i, j) += a(i, k) * b(k, j);
	return c;
}

template <class T>
void multiplication_edges() {
	std::mt19937_64 random(719);
	auto check = [&](int n, int inner, int m) {
		Matrix<T> a(n, inner), b(inner, m);
		for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) a(i, k) = T(random() % T::mod());
		for (int k = 0; k < inner; ++k) for (int j = 0; j < m; ++j) b(k, j) = T(random() % T::mod());
		assert(a * b == brute_product(a, b));
		a.fill(T(T::mod() - 1));
		b.fill(T(T::mod() - 1));
		assert(a * b == Matrix<T>(n, m, T(inner)));
	};
	for (const auto &[n, inner, m] : {
		std::array{0, 0, 0}, std::array{0, 16, 8}, std::array{4, 0, 9}, std::array{4, 16, 0},
		std::array{0, 0, 9}, std::array{4, 0, 0}, std::array{1, 1, 127}, std::array{1, 1, 128},
		std::array{1, 1, 129}, std::array{1, 127, 1}, std::array{1, 128, 1}, std::array{1, 129, 1},
		std::array{127, 1, 1}, std::array{128, 1, 1}, std::array{129, 1, 1}, std::array{2, 8, 8},
		std::array{2, 9, 8}, std::array{17, 65, 9}, std::array{65, 17, 33}, std::array{33, 65, 17}
	}) check(n, inner, m);
	for (int n = 1; n <= 9; ++n) for (int inner : {15, 16, 17, 31, 32, 33})
		for (int m : {7, 8, 9, 16, 17, 31, 32, 33}) check(n, inner, m);
	for (int m = 8; m < 16; ++m) check(5, 33, m);
	for (int n : {0, 1, 3, 4, 5, 15, 16, 17, 31, 32, 33}) {
		Matrix<T> a(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) a(i, j) = T(random() % T::mod());
		auto expected = brute_product(a, a);
		a *= a;
		assert(a == expected);
	}
}

template <class T>
void square(const Matrix<T> &a) {
	int n = a.rows();
	T det = brute_det(a);
	assert(a.det() == det);
	auto inverse = a.inverse();
	assert(bool(inverse) == (det != T(0)));
	if (inverse) {
		assert(a * *inverse == Matrix<T>::identity(n));
		assert(*inverse * a == Matrix<T>::identity(n));
	}
	auto adj = a.adjugate();
	assert(a * adj == Matrix<T>::identity(n) * det);
	assert(adj * a == Matrix<T>::identity(n) * det);
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
		Matrix<T> minor(n - 1);
		for (int r = 0, x = 0; r < n; ++r) if (r != j) {
			for (int c = 0, y = 0; c < n; ++c) if (c != i) minor(x, y++) = a(r, c);
			++x;
		}
		T expected = brute_det(minor);
		assert(adj(i, j) == (((i + j) & 1) ? -expected : expected));
	}
	auto p = a.characteristic_polynomial();
	assert(int(p.size()) == n + 1 && p.back() == T(1));
	assert(p[0] == ((n & 1) ? -det : det));
	if (n) assert(p[n - 1] == -a.trace());
	Matrix<T> value(n);
	for (int k = n; k >= 0; --k) value = value * a + Matrix<T>::identity(n) * p[k];
	assert(value == Matrix<T>(n));
	for (int z = 0; z < 4; ++z) {
		T scalar = 0;
		for (int k = n; k >= 0; --k) scalar = scalar * T(z) + p[k];
		assert(scalar == brute_det(Matrix<T>::identity(n) * T(z) - a));
	}
}

template <class T>
void rectangular(const Matrix<T> &a, std::mt19937_64 &random) {
	int n = a.rows(), m = a.cols();
	assert(a.transpose().transpose() == a);
	auto [r, pivots] = a.rref();
	auto [e, ep] = a.row_echelon();
	assert(pivots == ep && int(pivots.size()) == a.rank());
	for (int i = 0; i < int(pivots.size()); ++i) {
		assert(r(i, pivots[i]) == T(1));
		for (int j = 0; j < n; ++j) if (i != j) assert(r(j, pivots[i]) == T(0));
	}
	auto kernel = a.kernel();
	assert(int(kernel.size()) + a.rank() == m);
	for (const auto &v : kernel) assert(a * v == std::vector<T>(n, T(0)));
	std::vector<T> x(m);
	for (auto &v : x) v = T(random() % 13);
	auto b = a * x;
	auto solution = a.solve(b);
	assert(solution && a * solution->particular == b && solution->basis == kernel);
	for (const auto &v : solution->basis) {
		auto y = solution->particular;
		for (int j = 0; j < m; ++j) y[j] += v[j] * T(3);
		assert(a * y == b);
	}
	Matrix<T> bmat(n, m + 1);
	for (int i = 0; i < n; ++i) for (int j = 0; j <= m; ++j) bmat(i, j) = j == m ? T(random() % 13) : a(i, j);
	std::vector<T> rhs(n);
	for (int i = 0; i < n; ++i) rhs[i] = bmat(i, m);
	auto maybe = a.solve(rhs);
	assert(bool(maybe) == (a.rank() == bmat.rank()));
	if (maybe) assert(a * maybe->particular == rhs);
}

template <class T>
void modular() {
	std::mt19937_64 random(713);
	for (int n : {0, 1, 2, 3, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65}) {
		int m = random() % 67, k = random() % 67;
		Matrix<T> a(n, k), b(k, m);
		for (int i = 0; i < n; ++i) for (int j = 0; j < k; ++j) a(i, j) = T(random() % T::mod());
		for (int i = 0; i < k; ++i) for (int j = 0; j < m; ++j) b(i, j) = T(random() % T::mod());
		assert(a * b == brute_product(a, b));
	}
	Matrix<T> extreme(33, 65, T(T::mod() - 1)), extreme2(65, 17, T(T::mod() - 1));
	assert(extreme * extreme2 == Matrix<T>(33, 17, T(65)));
	for (int n : {17, 33, 65}) {
		Matrix<T> a(n, n + 7);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n + 7; ++j) a(i, j) = T(random() % T::mod());
		rectangular(a, random);
		Matrix<T> triangular(n);
		for (int i = 0; i < n; ++i) for (int j = i; j < n; ++j) triangular(i, j) = i == j ? T(1) : T(random() % T::mod());
		assert(triangular.det() == T(1));
		auto inverse = triangular.inverse();
		assert(inverse && triangular * *inverse == Matrix<T>::identity(n));
		auto poly = triangular.characteristic_polynomial();
		std::vector<T> expected(1, T(1));
		for (int k = 0; k < n; ++k) {
			std::vector<T> next(expected.size() + 1, T(0));
			for (int j = 0; j < int(expected.size()); ++j) { next[j] -= expected[j]; next[j + 1] += expected[j]; }
			expected = std::move(next);
		}
		assert(poly == expected);
		Matrix<T> dense(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) dense(i, j) = T(random() % T::mod());
		poly = dense.characteristic_polynomial();
		T evaluated = 0;
		for (int k = n; k >= 0; --k) evaluated = evaluated * T(3) + poly[k];
		assert(evaluated == (Matrix<T>::identity(n) * T(3) - dense).det());
	}
	for (int round = 0; round < 100; ++round) {
		int n = random() % 8, m = random() % 9;
		Matrix<T> a(n, m);
		for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) a(i, j) = T(random() % 7);
		rectangular(a, random);
		if (n == m) square(a);
	}
	for (int n = 0; n <= 5; ++n) for (int round = 0; round < 12; ++round) {
		Matrix<T> a(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) a(i, j) = T(random() % 11);
		square(a);
	}
	Matrix<T> a{{1, 2}, {3, 4}};
	auto b = a;
	b *= b;
	assert(b == a * a && a.pow(0) == Matrix<T>::identity(2));
	auto power = Matrix<T>::identity(2);
	for (int k = 0; k < 14; ++k) { assert(a.pow(k) == power); power *= a; }
	assert(a.pow(UINT64_MAX) == a.pow(UINT64_MAX - 1) * a);
	assert(a + a == T(2) * a && a - a == Matrix<T>(2));
	assert(-a + a == Matrix<T>(2) && (a / T(1)) == a);
	assert(a.submatrix(0, 1, 1, 2) == Matrix<T>({{T(2)}}));
	Matrix<T> from(std::vector<std::vector<T>>{{1, 2}, {3, 4}});
	assert(from == a && from.rows() == 2 && from.cols() == 2 && !from.empty());
	assert(from.data()[2] == T(3));
	const auto &constant = from;
	assert(constant.data()[1] == T(2) && constant[1][0] == T(3));
	Matrix<T> moved(std::move(from));
	assert(moved == a && from.rows() == 0 && from.cols() == 0 && from.empty());
	from = std::move(moved);
	assert(from == a && moved.empty());
	from.swap(moved);
	assert(moved == a && from.empty());
	from = moved;
	from.fill(T(7));
	assert(from == Matrix<T>(2, 2, T(7)) && moved == a);
}

template <std::uint32_t P, int step>
void centered_multiplication_edges() {
	using T = Mod<P>;
	constexpr std::uint32_t low = (P + 1) / 2, high = (P - 1) / 2;
	const std::array<std::uint32_t, 5> values{0, 1, P - 1, low, high};
	auto check = [&](const Matrix<T> &a, const Matrix<T> &b) {
		auto c = a * b;
		for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < b.cols(); ++j) {
			std::uint64_t expected = 0;
			for (int k = 0; k < a.cols(); ++k)
				expected = (expected + std::uint64_t(a(i, k).val()) * b(k, j).val()) % P;
			assert(c(i, j).val() == expected);
		}
		return c;
	};
	for (int n : {4, 5, 6, 7, 8, 9}) for (int m : {8, 9, 15, 16, 17})
		for (int inner : {15, 16, 17, step - 1, step, step + 1, 2 * step - 1, 2 * step, 2 * step + 1, 65})
			for (int kind = 0; kind < 5; ++kind) {
				Matrix<T> a(n, inner), b(inner, m);
				for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) {
					std::uint32_t v = low;
					if (kind == 2) v = P - 1;
					if (kind == 3 && k < step) v = k == 0 ? 1 : 0;
					if (kind == 4) v = values[(i * 3 + k) % values.size()];
					a(i, k) = T::raw(v);
				}
				for (int k = 0; k < inner; ++k) for (int j = 0; j < m; ++j) {
					std::uint32_t v = kind == 1 ? high : low;
					if (kind == 2) v = 1;
					if (kind == 3 && k < step) v = P - 1;
					if (kind == 4) v = values[(k * 2 + j * 3) % values.size()];
					b(k, j) = T::raw(v);
				}
				check(a, b);
			}
	for (int n : {16, 17, 31, 32, 33, 65}) {
		Matrix<T> a(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
			a(i, j) = T::raw(values[(i * 3 + j) % values.size()]);
		auto expected = check(a, a);
		a *= a;
		assert(a == expected);
	}
}

template <class T>
std::pair<Matrix<T>, std::vector<int>> elementary_rref(Matrix<T> a, int columns) {
	std::vector<int> pivots;
	for (int col = 0, rank = 0; col < columns && rank < a.rows(); ++col) {
		int p = rank;
		while (p < a.rows() && a(p, col) == T(0)) ++p;
		if (p == a.rows()) continue;
		for (int j = 0; j < a.cols(); ++j) std::swap(a(rank, j), a(p, j));
		T inverse = T(1) / a(rank, col);
		for (int j = 0; j < a.cols(); ++j) a(rank, j) *= inverse;
		for (int i = 0; i < a.rows(); ++i) if (i != rank) {
			T factor = a(i, col);
			for (int j = 0; j < a.cols(); ++j) a(i, j) -= factor * a(rank, j);
		}
		pivots.push_back(col);
		++rank;
	}
	return {std::move(a), std::move(pivots)};
}

template <class T>
void backward_elimination_edges() {
	std::mt19937_64 random(15427);
	for (const auto &[n, m] : {std::array{0, 0}, std::array{0, 4}, std::array{4, 0},
		std::array{1, 1}, std::array{2, 3}, std::array{3, 2}, std::array{4, 4},
		std::array{17, 9}, std::array{9, 17}, std::array{17, 17}, std::array{33, 33}})
		for (int kind = 0; kind < 4; ++kind) {
			Matrix<T> a(n, m);
			for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) a(i, j) = T(random() % 7);
			if (kind == 1) {
				for (int i = 0; i < n; ++i) for (int j = 0; j < std::min(n, m); ++j)
					if (i >= j) a(i, j) = T(i == j);
				if (n > 1) for (int j = 0; j < m; ++j) std::swap(a(0, j), a(n - 1, j));
			}
			if (kind == 2 && m) for (int i = 0; i < n; ++i) a(i, 0) = T(0);
			if (kind == 3 && n) for (int j = 0; j < m; ++j) a(n - 1, j) = n > 1 ? a(0, j) : T(0);
			auto original = a;
			auto expected = elementary_rref(a, m);
			assert(a.rref() == expected);
			assert(a.rank() == int(expected.second.size()));
			auto alias = a;
			alias = alias.rref().first;
			assert(alias == expected.first);
			for (int consistent = 0; consistent < 2; ++consistent) {
				std::vector<T> rhs(n), solution(m);
				for (auto &x : solution) x = T(random() % 7);
				for (int i = 0; i < n; ++i) {
					if (!consistent) rhs[i] = T(random() % 7);
					else for (int j = 0; j < m; ++j) rhs[i] += a(i, j) * solution[j];
				}
				Matrix<T> aug(n, m + 1);
				for (int i = 0; i < n; ++i) {
					for (int j = 0; j < m; ++j) aug(i, j) = a(i, j);
					aug(i, m) = rhs[i];
				}
				auto [r, pivots] = elementary_rref(aug, m);
				bool possible = true;
				for (int i = int(pivots.size()); i < n; ++i) possible &= r(i, m) == T(0);
				auto actual = a.solve(rhs);
				assert(bool(actual) == possible);
				if (!possible) continue;
				std::vector<T> particular(m);
				for (int i = 0; i < int(pivots.size()); ++i) particular[pivots[i]] = r(i, m);
				assert(actual->particular == particular);
				std::vector<std::vector<T>> basis;
				for (int col = 0; col < m; ++col) if (std::find(pivots.begin(), pivots.end(), col) == pivots.end()) {
					std::vector<T> v(m);
					v[col] = T(1);
					for (int i = 0; i < int(pivots.size()); ++i) v[pivots[i]] = -r(i, col);
					basis.push_back(std::move(v));
				}
				assert(actual->basis == basis);
			}
			if (n == m) {
				Matrix<T> aug(n, 2 * n);
				for (int i = 0; i < n; ++i) {
					for (int j = 0; j < n; ++j) aug(i, j) = a(i, j);
					aug(i, n + i) = T(1);
				}
				auto [r, pivots] = elementary_rref(aug, n);
				auto inverse = a.inverse();
				assert(bool(inverse) == (int(pivots.size()) == n));
				if (inverse) {
					assert(*inverse == r.submatrix(0, n, n, 2 * n));
					alias = a;
					alias = *alias.inverse();
					assert(alias == *inverse);
				}
			}
			assert(a == original);
		}
}

void arithmetic_moves() {
	Matrix<mint> b(3, 4, mint(2));
	auto check = [](auto operation, mint expected) {
		Matrix<mint> a(3, 4, mint(6));
		const auto *storage = a.data();
		auto result = operation(std::move(a));
		assert(result.data() == storage && a.empty());
		assert(result == Matrix<mint>(3, 4, expected));
	};
	check([&](auto a) { return std::move(a) + b; }, mint(8));
	check([&](auto a) { return std::move(a) - b; }, mint(4));
	check([](auto a) { return std::move(a) * mint(2); }, mint(12));
	check([](auto a) { return mint(2) * std::move(a); }, mint(12));
	check([](auto a) { return std::move(a) / mint(2); }, mint(3));
}

int main() {
	arithmetic_moves();
	backward_elimination_edges<mint>();
	backward_elimination_edges<Mod<2>>();
	backward_elimination_edges<Mod<2147483647>>();
	centered_multiplication_edges<2, 32>();
	centered_multiplication_edges<6, 32>();
	centered_multiplication_edges<998244353, 32>();
	centered_multiplication_edges<1000000007, 32>();
	centered_multiplication_edges<1073741823, 32>();
	centered_multiplication_edges<1073741824, 31>();
	centered_multiplication_edges<1073741825, 31>();
	centered_multiplication_edges<2147483646, 8>();
	centered_multiplication_edges<2147483647, 8>();
	multiplication_edges<mint>();
	multiplication_edges<Mod<1>>();
	multiplication_edges<Mod<2>>();
	multiplication_edges<Mod<6>>();
	multiplication_edges<Mod<998244353>>();
	multiplication_edges<Mod<1000000007>>();
	multiplication_edges<Mod<1073741824>>();
	multiplication_edges<Mod<1073741825>>();
	multiplication_edges<Mod<2147483647>>();
	multiplication_edges<Mod<2147483648u>>();
	modular<mint>();
	modular<Mod<1000000007>>();
	modular<Mod<2147483647>>();
	modular<Mod<2>>();
	for (int bits = 0; bits < 512; ++bits) {
		Matrix<Mod<2>> a(3);
		for (int i = 0; i < 9; ++i) a(i / 3, i % 3) = Mod<2>((bits >> i) & 1);
		square(a);
	}
	std::mt19937_64 random(7919);
	for (int n = 0; n <= 7; ++n) for (int round = 0; round < 20; ++round) {
		Matrix<long long> a(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) a(i, j) = static_cast<long long>(random() % 7) - 3;
		assert(a.det() == brute_det(a));
	}
	Matrix<Fraction<>> rational{{{1, 2}, {1, 3}}, {{2, 3}, {3, 4}}};
	square(rational);
	rectangular(rational, random);
	Matrix<double> real{{1e-20, 1}, {1, 1}};
	auto inverse = real.inverse();
	assert(inverse && real.rank() == 2 && std::abs(real.det() + 1) < 1e-10);
	auto product = real * *inverse;
	for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) assert(std::abs(product(i, j) - (i == j)) < 1e-10);
	Matrix<long double> small{{1e-16L}};
	assert(small.rank() == 0 && small.rank(0) == 1);
	Matrix<double> singular{{0.1, 0.2}, {0.3, 0.6}};
	auto adj = singular.adjugate();
	Matrix<double> expected{{0.6, -0.2}, {-0.3, 0.1}};
	for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) assert(std::abs(adj(i, j) - expected(i, j)) < 1e-10);
	assert((!Matrix<mint>{{1}, {1}}.solve({1, 2})));
	assert(!Matrix<mint>(2, 0).solve({0, 1}));
}
