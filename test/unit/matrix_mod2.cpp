#include "math/matrix_mod2.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

struct ByteMatrix {
	int n, m;
	std::vector<std::vector<unsigned char>> a;
	ByteMatrix(int rows, int cols) : n(rows), m(cols), a(rows, std::vector<unsigned char>(cols)) {}
	unsigned char& operator()(int i, int j) { return a[i][j]; }
	int operator()(int i, int j) const { return a[i][j]; }
};

std::vector<int> reduce(ByteMatrix& a, bool reduced) {
	std::vector<int> pivots;
	int rank = 0;
	for (int col = 0; col < a.m && rank < a.n; ++col) {
		int pivot = rank;
		while (pivot < a.n && !a(pivot, col)) ++pivot;
		if (pivot == a.n) continue;
		std::swap(a.a[pivot], a.a[rank]);
		for (int i = reduced ? 0 : rank + 1; i < a.n; ++i) if (i != rank && a(i, col))
			for (int j = 0; j < a.m; ++j) a(i, j) ^= a(rank, j);
		pivots.push_back(col);
		++rank;
	}
	return pivots;
}

ByteMatrix multiply(const ByteMatrix& a, const ByteMatrix& b) {
	assert(a.m == b.n);
	ByteMatrix c(a.n, b.m);
	for (int i = 0; i < a.n; ++i) for (int k = 0; k < a.m; ++k) if (a(i, k))
		for (int j = 0; j < b.m; ++j) c(i, j) ^= b(k, j);
	return c;
}

ByteMatrix bytes(const MatrixMod2& a) {
	ByteMatrix b(a.rows(), a.cols());
	for (int i = 0; i < b.n; ++i) for (int j = 0; j < b.m; ++j) b(i, j) = a(i, j);
	return b;
}

MatrixMod2 matrix(const ByteMatrix& a) {
	MatrixMod2 b(a.n, a.m);
	for (int i = 0; i < a.n; ++i) for (int j = 0; j < a.m; ++j) b(i, j) = a(i, j);
	return b;
}

DynamicBitset bits(const std::vector<unsigned char>& a) {
	DynamicBitset result(int(a.size()));
	for (int i = 0; i < int(a.size()); ++i) result[i] = a[i] != 0;
	return result;
}

void equal(const MatrixMod2& a, const ByteMatrix& b) {
	assert(a.rows() == b.n && a.cols() == b.m);
	assert(a.empty() == (b.n == 0 || b.m == 0));
	assert(std::size_t(a.words_per_row()) == (std::size_t(b.m) + 63) / 64);
	for (int i = 0; i < b.n; ++i) {
		auto row = a.row_words(i);
		assert(row.size() == (std::size_t(b.m) + 63) / 64);
		if (b.m & 63) assert((row.back() >> (b.m & 63)) == 0);
		std::string text(b.m, '0');
		for (int j = 0; j < b.m; ++j) {
			assert(a(i, j) == bool(b(i, j)) && a[i][j] == bool(b(i, j)));
			text[j] += b(i, j);
		}
		assert(a.row_string(i) == text);
		assert(a.row(i) == bits(b.a[i]));
	}
}

std::vector<unsigned char> product(const ByteMatrix& a, const DynamicBitset& x) {
	assert(a.m == x.size());
	std::vector<unsigned char> result(a.n);
	for (int i = 0; i < a.n; ++i) for (int j = 0; j < a.m; ++j) result[i] ^= a(i, j) & x[j];
	return result;
}

void check_kernel(const ByteMatrix& a, const std::vector<DynamicBitset>& basis, int rank) {
	assert(int(basis.size()) == a.m - rank);
	ByteMatrix vectors(int(basis.size()), a.m);
	for (int i = 0; i < int(basis.size()); ++i) {
		assert(basis[i].size() == a.m);
		auto value = product(a, basis[i]);
		assert(std::all_of(value.begin(), value.end(), [](int x) { return x == 0; }));
		for (int j = 0; j < a.m; ++j) vectors(i, j) = basis[i][j];
	}
	assert(int(reduce(vectors, false).size()) == int(basis.size()));
}

unsigned mask(const DynamicBitset& x) {
	assert(x.size() <= 10);
	unsigned result = 0;
	for (int j = 0; j < x.size(); ++j) result |= unsigned(x[j]) << j;
	return result;
}

void check_solve(const MatrixMod2& a, const ByteMatrix& b, const DynamicBitset& rhs, bool brute) {
	auto solution = a.solve(rhs);
	std::set<unsigned> expected;
	if (brute) for (unsigned x = 0; x < (1U << b.m); ++x) {
		bool valid = true;
		for (int i = 0; i < b.n; ++i) {
			int value = 0;
			for (int j = 0; j < b.m; ++j) value ^= b(i, j) & ((x >> j) & 1U);
			if (value != rhs[i]) valid = false;
		}
		if (valid) expected.insert(x);
	}
	if (brute) assert(bool(solution) == !expected.empty());
	if (!solution) {
		ByteMatrix augmented(b.n, b.m + 1);
		for (int i = 0; i < b.n; ++i) {
			for (int j = 0; j < b.m; ++j) augmented(i, j) = b(i, j);
			augmented(i, b.m) = rhs[i];
		}
		auto original = b;
		assert(reduce(augmented, false).size() > reduce(original, false).size());
		return;
	}
	assert(solution->particular.size() == b.m);
	auto evaluated = product(b, solution->particular);
	for (int i = 0; i < b.n; ++i) assert(evaluated[i] == rhs[i]);
	auto reduced = b;
	int rank = int(reduce(reduced, false).size());
	check_kernel(b, solution->basis, rank);
	if (brute) {
		std::set<unsigned> actual;
		for (unsigned subset = 0; subset < (1U << solution->basis.size()); ++subset) {
			unsigned x = mask(solution->particular);
			for (std::size_t j = 0; j < solution->basis.size(); ++j) if ((subset >> j) & 1) x ^= mask(solution->basis[j]);
			assert(actual.insert(x).second);
		}
		assert(actual == expected);
	}
}

void check_elimination(const ByteMatrix& b, bool brute, std::mt19937_64& rng) {
	auto a = matrix(b), saved = a;
	auto canonical = b;
	auto pivots = reduce(canonical, true);
	assert(a.rank() == int(pivots.size()));
	auto [rref, found] = a.rref();
	assert(found == pivots);
	equal(rref, canonical);
	auto [echelon, leading] = a.row_echelon();
	assert(leading == pivots);
	for (int i = 0; i < echelon.rows(); ++i) {
		int first = 0;
		while (first < echelon.cols() && !echelon(i, first)) ++first;
		assert(first == (i < int(leading.size()) ? leading[i] : echelon.cols()));
	}
	auto echelon_bytes = bytes(echelon);
	reduce(echelon_bytes, true);
	equal(rref, echelon_bytes);
	check_kernel(b, a.kernel(), int(pivots.size()));
	DynamicBitset rhs(b.n);
	for (int i = 0; i < b.n; ++i) rhs[i] = rng() & 1;
	check_solve(a, b, rhs, brute);
	DynamicBitset x(b.m);
	for (int j = 0; j < b.m; ++j) x[j] = rng() & 1;
	check_solve(a, b, bits(product(b, x)), brute);
	assert(a * x == bits(product(b, x)));
	if (b.n == b.m) {
		assert(a.det() == (int(pivots.size()) == b.n));
		auto inverse = a.inverse();
		assert(bool(inverse) == (int(pivots.size()) == b.n));
		if (inverse) {
			auto id = MatrixMod2::identity(b.n);
			equal(id, multiply(b, bytes(*inverse)));
			equal(id, multiply(bytes(*inverse), b));
		}
		int trace = 0;
		for (int i = 0; i < b.n; ++i) trace ^= b(i, i);
		assert(a.trace() == bool(trace));
		assert(a.pow(0) == MatrixMod2::identity(b.n));
		assert(a.pow(1) == a);
		equal(a.pow(2), multiply(b, b));
		if (b.n <= 8) {
			auto p = bytes(MatrixMod2::identity(b.n));
			for (int i = 0; i < 7; ++i) p = multiply(p, b);
			equal(a.pow(7), p);
		}
	}
	assert(a == saved);
}

void check_shapes(std::mt19937_64& rng) {
	for (int trial = 0; trial < 300; ++trial) {
		int n = rng() % 9, m = rng() % 9, k = rng() % 9;
		ByteMatrix a(n, m), b(m, k);
		int density = trial % 5;
		for (auto& row : a.a) for (auto& value : row) value = rng() % 5 < unsigned(density);
		for (auto& row : b.a) for (auto& value : row) value = rng() & 1;
		auto x = matrix(a), y = matrix(b);
		equal(x * y, multiply(a, b));
		auto z = x;
		z *= y;
		equal(z, multiply(a, b));
		auto transpose = x.transpose();
		assert(transpose.rows() == m && transpose.cols() == n && transpose.transpose() == x);
		for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) assert(transpose(j, i) == bool(a(i, j)));
		check_elimination(a, true, rng);
		ByteMatrix c(n, m);
		for (auto& row : c.a) for (auto& value : row) value = rng() & 1;
		auto sum = a;
		for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) sum(i, j) ^= c(i, j);
		auto other = matrix(c);
		equal(x + other, sum);
		equal(x - other, sum);
		z = x; z += other; equal(z, sum);
		z = x; z -= other; equal(z, sum);
		assert(-x == x && x * true == x && true * x == x);
		equal(x * false, ByteMatrix(n, m));
		z = x; z *= false; equal(z, ByteMatrix(n, m));
		z = x; z *= true; assert(z == x);
		int rl = n ? rng() % (n + 1) : 0, rr = n ? rng() % (n + 1) : 0;
		int cl = m ? rng() % (m + 1) : 0, cr = m ? rng() % (m + 1) : 0;
		if (rl > rr) std::swap(rl, rr);
		if (cl > cr) std::swap(cl, cr);
		ByteMatrix sub(rr - rl, cr - cl);
		for (int i = rl; i < rr; ++i) for (int j = cl; j < cr; ++j) sub(i - rl, j - cl) = a(i, j);
		equal(x.submatrix(rl, rr, cl, cr), sub);
	}
}

void check_boundaries(std::mt19937_64& rng) {
	for (int n : {0, 1, 63, 64, 65, 127, 128, 129}) {
		for (int m : {0, 1, 63, 64, 65, 129}) {
			MatrixMod2 ones(n, m, true);
			ByteMatrix expected(n, m);
			for (auto& row : expected.a) std::fill(row.begin(), row.end(), 1);
			equal(ones, expected);
			assert(ones.transpose().transpose() == ones);
			ones.fill(false); equal(ones, ByteMatrix(n, m));
			if (m == n) check_elimination(expected, false, rng);
		}
		ByteMatrix a(n, n);
		for (int i = 0; i < n; ++i) {
			a(i, i) = 1;
			for (int j = i + 1; j < n; ++j) a(i, j) = rng() & 1;
		}
		check_elimination(a, false, rng);
		for (int i = 0; i < n; ++i) a(i, n ? n - 1 : 0) = 0;
		check_elimination(a, false, rng);
	}
	for (auto [n, m, k] : {std::tuple{3, 63, 129}, {65, 64, 1}, {65, 65, 129}, {1, 129, 65}, {129, 1, 65}, {4, 0, 129}}) {
		ByteMatrix a(n, m), b(m, k);
		for (auto& row : a.a) for (auto& value : row) value = rng() & 1;
		for (auto& row : b.a) for (auto& value : row) value = rng() & 1;
		equal(matrix(a) * matrix(b), multiply(a, b));
	}
	for (int inner : {65, 129}) for (int columns : {1, 7, 65, 129}) {
		ByteMatrix a(128, inner), b(inner, columns);
		for (auto& row : b.a) for (auto& value : row) value = rng() & 1;
		equal(matrix(a) * matrix(b), multiply(a, b));
		for (auto& row : a.a) std::fill(row.begin(), row.end(), 1);
		equal(matrix(a) * matrix(b), multiply(a, b));
		for (int i = 0; i < a.n; ++i) {
			int first = (i * 17) % inner;
			a(i, first) = 0;
			if (inner == 129) {
				int second = (i * 37 + 3) % inner;
				if (second == first) second = (second + 1) % inner;
				a(i, second) = 0;
			}
		}
		equal(matrix(a) * matrix(b), multiply(a, b));
	}
	ByteMatrix lookup(256, 8), rows(8, 129);
	for (int i = 0; i < 256; ++i) for (int j = 0; j < 8; ++j) lookup(i, j) = (i >> j) & 1;
	for (auto& row : rows.a) for (auto& value : row) value = rng() & 1;
	equal(matrix(lookup) * matrix(rows), multiply(lookup, rows));
	ByteMatrix panels(256, 16), panel_rows(16, 129);
	for (int i = 0; i < 256; ++i) for (int j = 0; j < 8; ++j) {
		panels(i, j) = (i >> j) & 1;
		panels(i, j + 8) = (((i * 73 + 31) & 255) >> j) & 1;
	}
	for (auto& row : panel_rows.a) for (auto& value : row) value = rng() & 1;
	equal(matrix(panels) * matrix(panel_rows), multiply(panels, panel_rows));
	for (int m : {64, 65, 129}) {
		ByteMatrix scattered(264, m);
		for (int i = 0; i < 8; ++i) {
			scattered(i, m == 129 ? i * 9 + 3 : i * 7 + 2) = 1;
			for (int j = m == 129 ? 80 : 56; j < m; ++j) scattered(i, j) = rng() & 1;
		}
		for (int i = 0; i < 256; ++i) for (int j = 0; j < m; ++j)
			for (int k = 0; k < 8; ++k) if ((i >> k) & 1) scattered(i + 8, j) ^= scattered(k, j);
		check_elimination(scattered, false, rng);
	}
	for (int m : {0, 63, 64, 65, 127, 128, 129}) {
		ByteMatrix zero(3, m);
		DynamicBitset rhs(3); rhs.set(2);
		check_solve(matrix(zero), zero, rhs, false);
		check_elimination(ByteMatrix(0, m), false, rng);
	}
}

void check_access() {
	MatrixMod2 a{{1, 0, 1}, {0, 1, 0}};
	assert(a == MatrixMod2(std::vector<std::vector<int>>{{1, 0, 1}, {0, 1, 0}}));
	assert(a == MatrixMod2(std::vector<std::string>{"101", "010"}));
	std::vector<DynamicBitset> rows{DynamicBitset(3), DynamicBitset(3)};
	rows[0].set(0).set(2); rows[1].set(1);
	assert(a == MatrixMod2(rows));
	a.set_row(0, "011");
	assert(!a(0, 0) && a(0, 1) && a(0, 2));
	a.set_row(0, rows[0]);
	assert(a.row_string(0) == "101");
	MatrixMod2 wide(3, 129);
	wide.set(0, 0); wide.set(0, 63); wide.set(1, 64); wide.set(2, 128);
	wide(0, 1) = wide(0, 0);
	wide[0][2] = wide[0][1];
	wide(1, 63) = wide(1, 64);
	wide(1, 64) = wide(1, 64);
	wide.flip(0, 0);
	assert(!wide(0, 0) && wide(0, 1) && wide(0, 2) && wide(1, 63) && wide(1, 64));
	auto copy = wide;
	wide.set(2, 128, false);
	assert(copy(2, 128) && !wide(2, 128));
	auto owned = copy.row(0);
	copy.flip(0, 1);
	assert(owned[1]);
	std::vector<std::uint64_t> words;
	for (int i = 0; i < copy.rows(); ++i) {
		auto row = copy.row_words(i);
		words.insert(words.end(), row.begin(), row.end());
		words.back() |= ~std::uint64_t(1);
	}
	assert(MatrixMod2::from_words(3, 129, words) == copy);
	assert(MatrixMod2::from_words(0, 129, {}).rows() == 0);
	auto moved = std::move(copy);
	assert(copy.rows() == 0 && copy.cols() == 0 && copy.empty());
	MatrixMod2 assigned(63, 65, true);
	assigned = std::move(moved);
	assert(moved.rows() == 0 && moved.cols() == 0);
	auto original = assigned;
	auto* alias = &assigned;
	assigned = *alias;
	assert(assigned == original);
	assigned = std::move(*alias);
	assert(assigned == original);
	MatrixMod2 other(0, 65);
	assigned.swap(other);
	assert(assigned.rows() == 0 && assigned.cols() == 65 && other == original);
	assert(MatrixMod2(3).rows() == 3 && MatrixMod2(3).cols() == 3);
	bool caught = false;
	try { MatrixMod2 bad(std::vector<std::string>{"01", "0x"}); } catch (const std::invalid_argument&) { caught = true; }
	assert(caught);
	caught = false;
	try { a.set_row(0, "01x"); } catch (const std::invalid_argument&) { caught = true; }
	assert(caught);
}

int main() {
	std::mt19937_64 rng(12098305);
	check_access();
	check_shapes(rng);
	check_boundaries(rng);
}
