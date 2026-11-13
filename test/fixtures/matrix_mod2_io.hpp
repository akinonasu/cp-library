#ifndef TEST_MATRIX_MOD2_IO_HPP
#define TEST_MATRIX_MOD2_IO_HPP

#include "math/matrix_mod2.hpp"
#include "components/fast_io.hpp"

inline MatrixMod2 read_matrix_mod2(FastInput& input, int n, int m) {
	MatrixMod2 matrix(n, m);
	for (int i = 0; i < n; ++i) {
		std::string row;
		if (m) input.read(row);
		matrix.set_row(i, row);
	}
	return matrix;
}

inline void write_matrix_mod2(FastOutput& output, const MatrixMod2& matrix) {
	for (int i = 0; i < matrix.rows(); ++i) output.println(matrix.row_string(i));
}

inline DynamicBitset read_vector_mod2(FastInput& input, int n) {
	std::string text;
	if (n) input.read(text);
	assert(int(text.size()) == n);
	DynamicBitset result(n);
	for (int i = 0; i < n; ++i) result[i] = text[i] == '1';
	return result;
}

inline void write_vector_mod2(FastOutput& output, const DynamicBitset& vector) {
	std::string text(vector.size(), '0');
	for (int i = 0; i < vector.size(); ++i) text[i] += int(vector[i]);
	output.println(text);
}

#endif
