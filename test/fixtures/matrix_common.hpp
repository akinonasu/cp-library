#ifndef TEST_MATRIX_COMMON_HPP
#define TEST_MATRIX_COMMON_HPP

#include "components/fast_io.hpp"
#include "math/linear_algebra/matrix.hpp"
#include "math/mint.hpp"

inline Matrix<mint> read_matrix(FastInput &in, int n, int m) {
	Matrix<mint> a(n, m);
	for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) { unsigned x = 0; in.read(x); a(i, j) = mint::raw(x); }
	return a;
}

inline void write_matrix(FastOutput &out, const Matrix<mint> &a) {
	for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < a.cols(); ++j) { out.write(a(i, j).val()); out.write(j + 1 == a.cols() ? '\n' : ' '); }
}

#endif
