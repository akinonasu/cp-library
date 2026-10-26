#ifndef TEST_MATRIX_COMMON_HPP
#define TEST_MATRIX_COMMON_HPP

#include "components/fast_io_bulk.hpp"
#include "math/matrix.hpp"
#include "math/mint.hpp"

inline Matrix<mint> read_matrix(FastInput &in, int n, int m) {
	Matrix<mint> a(n, m);
	auto *next = a.data();
	read_unsigned_blocks(in, std::size_t(n) * m, [&](const std::uint32_t *values, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) next[i] = mint::raw(values[i]);
		next += count;
	});
	return a;
}

inline void write_matrix(FastOutput &out, const Matrix<mint> &a) {
	for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < a.cols(); ++j) { out.write(a(i, j).val()); out.write(j + 1 == a.cols() ? '\n' : ' '); }
}

#endif
