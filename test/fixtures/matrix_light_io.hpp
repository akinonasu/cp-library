#ifndef TEST_MATRIX_LIGHT_IO_HPP
#define TEST_MATRIX_LIGHT_IO_HPP

#include "components/fast_io_bulk.hpp"
#include "math/matrix_light.hpp"
#include "math/mint.hpp"

inline Matrix<mint> read_light_matrix(FastInput &input, int n, int m) {
	Matrix<mint> result(n, m);
	auto *next = result.data();
	read_unsigned_blocks(input, std::size_t(n) * m, [&](const std::uint32_t *values, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) next[i] = mint::raw(values[i]);
		next += count;
	});
	return result;
}

inline void write_light_matrix(FastOutput &output, const Matrix<mint> &matrix) {
	for (int i = 0; i < matrix.rows(); ++i) for (int j = 0; j < matrix.cols(); ++j) {
		output.write(matrix(i, j).val());
		output.write(j + 1 == matrix.cols() ? '\n' : ' ');
	}
}

#endif
