#include "math/matrix.hpp"
#include "math/mint.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

struct GenericMint {
	mint value;
	GenericMint(long long x = 0) : value(x) {}
	unsigned val() const { return value.val(); }
	GenericMint operator-() const { GenericMint r; r.value = -value; return r; }
	GenericMint &operator+=(GenericMint b) { value += b.value; return *this; }
	GenericMint &operator-=(GenericMint b) { value -= b.value; return *this; }
	GenericMint &operator*=(GenericMint b) { value *= b.value; return *this; }
	GenericMint &operator/=(GenericMint b) { value /= b.value; return *this; }
	friend GenericMint operator+(GenericMint a, GenericMint b) { return a += b; }
	friend GenericMint operator-(GenericMint a, GenericMint b) { return a -= b; }
	friend GenericMint operator*(GenericMint a, GenericMint b) { return a *= b; }
	friend GenericMint operator/(GenericMint a, GenericMint b) { return a /= b; }
	bool operator==(const GenericMint &) const = default;
};

template <class T>
void run(const std::string &operation, const std::string &kind, int n, int inner, int m) {
	std::mt19937_64 random(713);
	Matrix<T> a(n, operation == "product" ? inner : m), b(inner, m);
	for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < a.cols(); ++j) a(i, j) = T(random() % mint::mod());
	for (int i = 0; i < b.rows(); ++i) for (int j = 0; j < b.cols(); ++j) b(i, j) = T(random() % mint::mod());
	if (kind == "singular" && n > 1) for (int j = 0; j < a.cols(); ++j) a(n - 1, j) = a(0, j);
	if (kind == "zero") a.fill(T(0));
	Matrix<T> result;
	auto begin = std::chrono::steady_clock::now();
	if (operation == "product") result = a * b;
	else if (operation == "small_product") { for (int k = 0; k < 1000; ++k) { result = a * b; a(0, 0) += T(1); } }
	else if (operation == "det") { result = Matrix<T>(1); result(0, 0) = a.det(); }
	else if (operation == "rank") { result = Matrix<T>(1); result(0, 0) = T(a.rank()); }
	else if (operation == "inverse") { auto x = a.inverse(); if (x) result = std::move(*x); }
	else if (operation == "adjugate") result = a.adjugate();
	else if (operation == "pow") result = a.pow(UINT64_C(1000000000000000000));
	else if (operation == "characteristic") { auto p = a.characteristic_polynomial(); result = Matrix<T>(1, int(p.size())); for (int i = 0; i < int(p.size()); ++i) result(0, i) = p[i]; }
	else if (operation == "solve") {
		std::vector<T> rhs(n, T(1));
		auto x = a.solve(rhs);
		if (x) {
			result = Matrix<T>(1 + int(x->basis.size()), m);
			for (int j = 0; j < m; ++j) result(0, j) = x->particular[j];
			for (int i = 0; i < int(x->basis.size()); ++i) for (int j = 0; j < m; ++j) result(i + 1, j) = x->basis[i][j];
		}
	}
	double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
	std::cout << seconds << '\n' << result.rows() << ' ' << result.cols() << '\n';
	for (int i = 0; i < result.rows(); ++i) for (int j = 0; j < result.cols(); ++j) std::cout << result(i, j).val() << ' ';
	std::cout << '\n';
}

int main(int argc, char **argv) {
	if (argc != 7) return 1;
	std::ios::sync_with_stdio(false);
	if (std::string(argv[1]) == "baseline") run<GenericMint>(argv[2], argv[3], std::stoi(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
	else run<mint>(argv[2], argv[3], std::stoi(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
}
