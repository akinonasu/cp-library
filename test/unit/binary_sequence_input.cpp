#include "data_structure/dynamic_bitset.hpp"
#include "math/matrix_mod2.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <memory_resource>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

template <class T>
void one_dimension() {
	for (int n : {0, 1, 2, 31, 63, 64, 65, 127, 128, 129}) {
		std::vector<T> values(n);
		for (int i = 0; i < n; ++i) values[i] = T(i % 3 == 0 || i % 7 == 2);
		DynamicBitset bits(values);
		assert(bits.size() == n);
		int count = 0;
		for (int i = 0; i < n; ++i) { assert(bits[i] == bool(values[i])); count += bool(values[i]); }
		assert(bits.count() == count);
		if (n % 64) assert((bits.words().back() >> (n % 64)) == 0);
		std::pmr::vector<T> custom(values.begin(), values.end());
		assert(DynamicBitset(custom) == bits);
		if constexpr (!std::is_same_v<T, bool>) {
			auto mutable_copy = values;
			assert(DynamicBitset(std::span(mutable_copy)) == bits);
			assert(DynamicBitset(std::span<const T>(values)) == bits);
			assert(mutable_copy == values);
		}
	}
	std::array<T, 65> fixed{};
	fixed[0] = fixed[63] = fixed[64] = T(1);
	DynamicBitset bits(fixed);
	assert(bits.count() == 3 && bits[0] && bits[63] && bits[64]);
	assert(DynamicBitset(std::span(fixed)) == bits);
	assert(DynamicBitset(std::span<const T, 65>(fixed)) == bits);
	assert(DynamicBitset(std::span<const T>(fixed)) == bits);
	std::array<T, 0> empty;
	assert(DynamicBitset(empty).empty());
}

template <class T>
void matrices() {
	std::mt19937 rng(302 + sizeof(T));
	for (int trial = 0; trial < 30; ++trial) {
		int rows = trial % 7, cols = rows ? trial % 8 : 0;
		std::vector<std::vector<T>> values(rows, std::vector<T>(cols));
		std::vector<T> rhs(rows), input(cols);
		for (auto &row : values) for (int j = 0; j < cols; ++j) row[j] = T(rng() % 2);
		for (int i = 0; i < rows; ++i) rhs[i] = T(rng() % 2);
		for (int j = 0; j < cols; ++j) input[j] = T(rng() % 2);
		const auto saved = values;
		MatrixMod2 matrix(values);
		assert(matrix.rows() == rows && matrix.cols() == cols);
		assert(MatrixMod2(std::span(values)) == matrix);
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) assert(matrix(i, j) == bool(values[i][j]));
			matrix.set_row(i, values[i]);
		}
		auto product = matrix * input;
		for (int i = 0; i < rows; ++i) {
			bool expected = false;
			for (int j = 0; j < cols; ++j) expected ^= bool(values[i][j]) && bool(input[j]);
			assert(product[i] == expected);
		}
		std::vector<unsigned> solutions;
		for (unsigned mask = 0; mask < (1U << cols); ++mask) {
			bool valid = true;
			for (int i = 0; i < rows; ++i) {
				bool result = false;
				for (int j = 0; j < cols; ++j) result ^= bool(values[i][j]) && ((mask >> j) & 1);
				valid &= result == bool(rhs[i]);
			}
			if (valid) solutions.push_back(mask);
		}
		auto answer = matrix.solve(rhs);
		assert(bool(answer) == !solutions.empty());
		if (answer) {
			assert(matrix * answer->particular == DynamicBitset(rhs));
			assert((std::size_t(1) << answer->basis.size()) == solutions.size());
			for (const auto &direction : answer->basis) assert((matrix * direction).none());
		}
		if constexpr (!std::is_same_v<T, bool>) {
			assert(matrix * std::span<const T>(input) == product);
			assert(bool(matrix.solve(std::span(rhs))) == bool(answer));
			if (rows) matrix.set_row(0, std::span(values[0]));
		}
		assert(saved == values);
	}
	std::array<std::array<T, 3>, 2> fixed{};
	fixed[0][0] = fixed[0][2] = fixed[1][1] = T(1);
	MatrixMod2 matrix(fixed);
	assert(matrix.row_string(0) == "101" && matrix.row_string(1) == "010");
	assert(MatrixMod2(std::span(fixed)) == matrix);
	const auto &constant = fixed;
	assert(MatrixMod2(std::span(constant)) == matrix);
	std::array<T, 3> row{T(0), T(1), T(1)};
	matrix.set_row(0, row);
	assert(matrix.row_string(0) == "011");
	matrix.set_row(0, std::span<const T, 3>(row));
	assert(matrix.row_string(0) == "011");
	std::array<T, 3> input{T(1), T(0), T(1)};
	assert((matrix * std::span(input)).to_string() == "01");
	std::array<T, 2> rhs{T(1), T(0)};
	auto answer = matrix.solve(std::span<const T, 2>(rhs));
	assert(answer && matrix * answer->particular == DynamicBitset(rhs));
}

template <class T>
concept Accepted = requires(const std::vector<T> &values) { DynamicBitset(values); };

template <class T>
struct RangeWithoutEmpty {
	T *first;
	std::size_t length;
	T *begin() const { return first; }
	T *end() const { return length ? first + length : first; }
	std::size_t size() const { return length; }
};

int main() {
	static_assert(Accepted<bool> && Accepted<int> && Accepted<std::uint64_t> && !Accepted<double>);
	static_assert(!binary_sequence_detail::Sequence<std::span<volatile int>>);
	one_dimension<bool>(); one_dimension<char>(); one_dimension<signed char>(); one_dimension<unsigned char>();
	one_dimension<short>(); one_dimension<unsigned short>(); one_dimension<int>(); one_dimension<unsigned int>();
	one_dimension<long>(); one_dimension<unsigned long>(); one_dimension<long long>(); one_dimension<unsigned long long>();
	one_dimension<wchar_t>(); one_dimension<char8_t>(); one_dimension<char16_t>(); one_dimension<char32_t>();
	matrices<bool>(); matrices<char>(); matrices<signed char>(); matrices<unsigned char>();
	matrices<short>(); matrices<unsigned short>(); matrices<int>(); matrices<unsigned int>();
	matrices<long>(); matrices<unsigned long>(); matrices<long long>(); matrices<unsigned long long>();
	matrices<wchar_t>(); matrices<char8_t>(); matrices<char16_t>(); matrices<char32_t>();
	std::deque<unsigned char> values{1, 0, 1, 1};
	assert(DynamicBitset(values).to_string() == "1101");
	assert(DynamicBitset("100")[2]);
	assert(DynamicBitset(std::array<int, 3>{1, 0, 0})[0]);
	MatrixMod2 text(std::vector<std::string>{"10", "01"});
	text.set_row(0, "01");
	assert(text.row_string(0) == "01" && text.row_string(1) == "01");
	int raw[3]{1, 0, 1};
	assert(DynamicBitset(raw).to_string() == "101");
	MatrixMod2 empty_rows(0, 4);
	assert(empty_rows.solve(std::array<bool, 0>{})->basis.size() == 4);
	MatrixMod2 empty_columns(3, 0);
	assert((empty_columns * std::array<int, 0>{}).none());
	assert(empty_columns.solve(std::array<int, 3>{0, 0, 0}));
	assert(!empty_columns.solve(std::array<int, 3>{0, 1, 0}));
	std::array<int, 3> row_values{1, 0, 1};
	RangeWithoutEmpty<const int> binary_view{row_values.data(), row_values.size()};
	assert(DynamicBitset(binary_view).to_string() == "101");
	std::array<std::array<int, 3>, 2> matrix_values{row_values, row_values};
	RangeWithoutEmpty<const std::array<int, 3>> matrix_view{matrix_values.data(), matrix_values.size()};
	assert(MatrixMod2(matrix_view) == MatrixMod2(matrix_values));
	RangeWithoutEmpty<const std::array<int, 3>> empty_view{nullptr, 0};
	assert(MatrixMod2(empty_view).rows() == 0 && MatrixMod2(empty_view).cols() == 0);
}
