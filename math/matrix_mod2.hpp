#ifndef MATRIX_MOD2_HPP
#define MATRIX_MOD2_HPP

#include "../components/matrix_mod2_kernel.hpp"
#include "../data_structure/dynamic_bitset.hpp"
#include <cstring>
#include <initializer_list>
#include <string_view>

struct LinearSolutionMod2 {
	DynamicBitset particular;
	std::vector<DynamicBitset> basis;
};

class MatrixMod2 {
	using Word = std::uint64_t;
	int h_ = 0, w_ = 0, stride_ = 0;
	std::vector<Word> data_;
	static int word_count(int n) { return int((std::size_t(n) + 63) / 64); }
	static std::size_t allocation(int h, int stride) {
		if (stride && std::size_t(h) > std::vector<Word>().max_size() / std::size_t(stride))
			throw std::length_error("MatrixMod2");
		return std::size_t(h) * stride;
	}
	void check_row([[maybe_unused]] int i) const { assert(0 <= i && i < h_); }
	void check([[maybe_unused]] int i, [[maybe_unused]] int j) const { check_row(i); assert(0 <= j && j < w_); }
	Word* raw_row(int i) { return stride_ ? data_.data() + std::size_t(i) * stride_ : data_.data(); }
	const Word* raw_row(int i) const { return stride_ ? data_.data() + std::size_t(i) * stride_ : data_.data(); }
	void trim() {
		if (w_ & 63) for (int i = 0; i < h_; ++i) raw_row(i)[stride_ - 1] &= (Word(1) << (w_ & 63)) - 1;
	}
	std::vector<int> reduce(int columns, bool reduced) {
		return matrix_mod2_detail::eliminate(data_, h_, stride_, columns, reduced);
	}
	static std::vector<DynamicBitset> nullspace(const MatrixMod2& r, const std::vector<int>& pivots, int vars) {
		std::vector<unsigned char> is_pivot(vars);
		for (int p : pivots) is_pivot[p] = 1;
		std::vector<DynamicBitset> result;
		result.reserve(std::size_t(vars) - pivots.size());
		for (int free = 0; free < vars; ++free) if (!is_pivot[free]) {
			DynamicBitset v(vars);
			v.set(free);
			for (int i = 0; i < int(pivots.size()); ++i) if (r(i, free)) v.set(pivots[i]);
			result.push_back(std::move(v));
		}
		return result;
	}
	std::pair<MatrixMod2, std::vector<int>> echelon(bool reduced) const {
		MatrixMod2 result = *this;
		auto pivots = result.reduce(w_, reduced);
		return {std::move(result), std::move(pivots)};
	}

  public:
	using value_type = bool;
	using word_type = Word;
	using Solution = LinearSolutionMod2;
	class reference {
		Word* word_;
		Word mask_;
		friend class MatrixMod2;
		reference(Word& word, int bit) : word_(&word), mask_(Word(1) << bit) {}
	  public:
		reference(const reference&) = default;
		operator bool() const { return (*word_ & mask_) != 0; }
		reference& operator=(bool value) { if (value) *word_ |= mask_; else *word_ &= ~mask_; return *this; }
		reference& operator=(const reference& other) { return *this = bool(other); }
		reference& flip() { *word_ ^= mask_; return *this; }
		reference& operator^=(bool value) { if (value) flip(); return *this; }
	};
	class Row {
		MatrixMod2* matrix_;
		int row_;
		friend class MatrixMod2;
		Row(MatrixMod2& matrix, int row) : matrix_(&matrix), row_(row) {}
	  public:
		reference operator[](int j) const { return (*matrix_)(row_, j); }
		int size() const { return matrix_->cols(); }
	};
	class ConstRow {
		const MatrixMod2* matrix_;
		int row_;
		friend class MatrixMod2;
		ConstRow(const MatrixMod2& matrix, int row) : matrix_(&matrix), row_(row) {}
	  public:
		bool operator[](int j) const { return (*matrix_)(row_, j); }
		int size() const { return matrix_->cols(); }
	};
	MatrixMod2() = default;
	explicit MatrixMod2(int n) : MatrixMod2(n, n) {}
	MatrixMod2(int rows, int columns, bool value = false) : h_(rows), w_(columns) {
		assert(rows >= 0 && columns >= 0);
		stride_ = word_count(columns);
		data_.assign(allocation(rows, stride_), value ? ~Word(0) : Word(0));
		trim();
	}
	MatrixMod2(std::initializer_list<std::initializer_list<int>> values)
		: MatrixMod2(int(values.size()), values.size() ? int(values.begin()->size()) : 0) {
		int i = 0;
		for (const auto& row : values) {
			assert(int(row.size()) == w_);
			int j = 0;
			for (int value : row) { assert(value == 0 || value == 1); (*this)(i, j++) = bool(value); }
			++i;
		}
	}
	explicit MatrixMod2(const std::vector<std::vector<int>>& values)
		: MatrixMod2(int(values.size()), values.empty() ? 0 : int(values[0].size())) {
		for (int i = 0; i < h_; ++i) {
			assert(int(values[i].size()) == w_);
			for (int j = 0; j < w_; ++j) { assert(values[i][j] == 0 || values[i][j] == 1); (*this)(i, j) = bool(values[i][j]); }
		}
	}
	explicit MatrixMod2(const std::vector<std::string>& values)
		: MatrixMod2(int(values.size()), values.empty() ? 0 : int(values[0].size())) {
		for (int i = 0; i < h_; ++i) set_row(i, values[i]);
	}
	explicit MatrixMod2(const std::vector<DynamicBitset>& values)
		: MatrixMod2(int(values.size()), values.empty() ? 0 : values[0].size()) {
		for (int i = 0; i < h_; ++i) set_row(i, values[i]);
	}
	MatrixMod2(const MatrixMod2&) = default;
	MatrixMod2& operator=(const MatrixMod2&) = default;
	MatrixMod2(MatrixMod2&& other) noexcept { swap(other); }
	MatrixMod2& operator=(MatrixMod2&& other) noexcept {
		if (this != &other) { MatrixMod2 next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(MatrixMod2& other) noexcept {
		std::swap(h_, other.h_), std::swap(w_, other.w_), std::swap(stride_, other.stride_);
		data_.swap(other.data_);
	}
	int rows() const { return h_; }
	int cols() const { return w_; }
	int words_per_row() const { return stride_; }
	bool empty() const { return !h_ || !w_; }
	bool operator()(int i, int j) const { check(i, j); return (raw_row(i)[j / 64] >> (j & 63)) & 1; }
	reference operator()(int i, int j) { check(i, j); return reference(raw_row(i)[j / 64], j & 63); }
	ConstRow operator[](int i) const { check_row(i); return ConstRow(*this, i); }
	Row operator[](int i) { check_row(i); return Row(*this, i); }
	void set(int i, int j, bool value = true) { (*this)(i, j) = value; }
	void flip(int i, int j) { (*this)(i, j).flip(); }
	void fill(bool value) { std::fill(data_.begin(), data_.end(), value ? ~Word(0) : Word(0)); trim(); }
	std::span<const Word> row_words(int i) const { check_row(i); return {raw_row(i), std::size_t(stride_)}; }
	static MatrixMod2 from_words(int rows, int columns, std::span<const Word> words) {
		MatrixMod2 result(rows, columns);
		assert(words.size() == result.data_.size());
		std::copy(words.begin(), words.end(), result.data_.begin());
		result.trim();
		return result;
	}
	DynamicBitset row(int i) const { return DynamicBitset::from_words(w_, row_words(i)); }
	void set_row(int i, const DynamicBitset& bits) {
		check_row(i);
		assert(bits.size() == w_);
		std::copy(bits.words().begin(), bits.words().end(), raw_row(i));
	}
	void set_row(int i, std::string_view text) {
		check_row(i);
		assert(text.size() == std::size_t(w_));
		if (stride_) std::fill_n(raw_row(i), stride_, Word(0));
		int j = 0;
		if constexpr ('0' == 0x30 && '1' == 0x31 && (std::endian::native == std::endian::little || std::endian::native == std::endian::big)) {
			for (; w_ - j >= 8; j += 8) {
				Word bytes;
				std::memcpy(&bytes, text.data() + j, 8);
				if constexpr (std::endian::native == std::endian::big) bytes = std::byteswap(bytes);
				if ((bytes & 0xfefefefefefefefeULL) != 0x3030303030303030ULL) throw std::invalid_argument("MatrixMod2");
				Word bits = ((bytes & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56;
				raw_row(i)[j / 64] |= bits << (j & 63);
			}
		}
		for (; j < w_; ++j) {
			if (text[j] != '0' && text[j] != '1') throw std::invalid_argument("MatrixMod2");
			if (text[j] == '1') raw_row(i)[j / 64] |= Word(1) << (j & 63);
		}
	}
	std::string row_string(int i) const {
		check_row(i);
		std::string result(w_, '0');
		int j = 0;
		if constexpr ('0' == 0x30 && '1' == 0x31 && (std::endian::native == std::endian::little || std::endian::native == std::endian::big)) {
			for (; w_ - j >= 8; j += 8) {
				Word bits = (raw_row(i)[j / 64] >> (j & 63)) & 255;
				bits = (bits | (bits << 28)) & 0x0000000f0000000fULL;
				bits = (bits | (bits << 14)) & 0x0003000300030003ULL;
				bits = (bits | (bits << 7)) & 0x0101010101010101ULL;
				bits += 0x3030303030303030ULL;
				if constexpr (std::endian::native == std::endian::big) bits = std::byteswap(bits);
				std::memcpy(result.data() + j, &bits, 8);
			}
		}
		for (; j < w_; ++j) if ((*this)(i, j)) result[j] = '1';
		return result;
	}
	bool operator==(const MatrixMod2&) const = default;
	static MatrixMod2 identity(int n) {
		MatrixMod2 result(n);
		for (int i = 0; i < n; ++i) result(i, i) = true;
		return result;
	}
	MatrixMod2 transpose() const {
		allocation(w_, word_count(h_));
		MatrixMod2 result;
		result.h_ = w_, result.w_ = h_, result.stride_ = word_count(h_);
		result.data_ = matrix_mod2_detail::transpose(data_, h_, w_);
		return result;
	}
	MatrixMod2 submatrix(int row_l, int row_r, int col_l, int col_r) const {
		assert(0 <= row_l && row_l <= row_r && row_r <= h_ && 0 <= col_l && col_l <= col_r && col_r <= w_);
		MatrixMod2 result(row_r - row_l, col_r - col_l);
		int first = col_l / 64, shift = col_l & 63;
		for (int i = 0; i < result.h_; ++i) for (int j = 0; j < result.stride_; ++j) {
			Word value = raw_row(row_l + i)[first + j] >> shift;
			if (shift && first + j + 1 < stride_) value |= raw_row(row_l + i)[first + j + 1] << (64 - shift);
			result.raw_row(i)[j] = value;
		}
		result.trim();
		return result;
	}
	MatrixMod2 operator-() const { return *this; }
	MatrixMod2& operator+=(const MatrixMod2& other) {
		assert(h_ == other.h_ && w_ == other.w_);
		for (std::size_t i = 0; i < data_.size(); ++i) data_[i] ^= other.data_[i];
		return *this;
	}
	MatrixMod2& operator-=(const MatrixMod2& other) { return *this += other; }
	MatrixMod2& operator*=(const MatrixMod2& other) { return *this = *this * other; }
	MatrixMod2& operator*=(bool value) { if (!value) fill(false); return *this; }
	friend MatrixMod2 operator+(MatrixMod2 a, const MatrixMod2& b) { return a += b; }
	friend MatrixMod2 operator-(MatrixMod2 a, const MatrixMod2& b) { return a -= b; }
	friend MatrixMod2 operator*(const MatrixMod2& a, const MatrixMod2& b) {
		assert(a.w_ == b.h_);
		allocation(a.h_, b.stride_);
		MatrixMod2 result;
		result.h_ = a.h_, result.w_ = b.w_, result.stride_ = b.stride_;
		result.data_ = matrix_mod2_detail::multiply(a.data_, b.data_, a.h_, a.w_, b.w_);
		return result;
	}
	friend MatrixMod2 operator*(MatrixMod2 a, bool b) { return a *= b; }
	friend MatrixMod2 operator*(bool a, MatrixMod2 b) { return b *= a; }
	DynamicBitset operator*(const DynamicBitset& v) const {
		assert(v.size() == w_);
		DynamicBitset result(h_);
		for (int i = 0; i < h_; ++i) {
			Word value = 0;
			for (int j = 0; j < stride_; ++j) value ^= raw_row(i)[j] & v.words()[j];
			if (std::popcount(value) & 1) result.set(i);
		}
		return result;
	}
	MatrixMod2 pow(std::uint64_t exponent) const {
		assert(h_ == w_);
		MatrixMod2 result = identity(h_);
		if (!exponent) return result;
		MatrixMod2 base = *this;
		while (exponent) {
			if (exponent & 1) result *= base;
			exponent >>= 1;
			if (exponent) base *= base;
		}
		return result;
	}
	bool trace() const {
		assert(h_ == w_);
		bool result = false;
		for (int i = 0; i < h_; ++i) result ^= (*this)(i, i);
		return result;
	}
	int rank() const {
		if (!h_ || !w_) return 0;
		if (w_ <= 64) {
			Word basis[64]{};
			int result = 0;
			for (int i = 0; i < h_; ++i) {
				Word value = raw_row(i)[0];
				while (value) {
					int pivot = std::countr_zero(value);
					if (basis[pivot]) value ^= basis[pivot];
					else { basis[pivot] = value; ++result; break; }
				}
				if (result == w_) return result;
			}
			return result;
		}
		MatrixMod2 work = *this;
		return int(work.reduce(w_, false).size());
	}
	bool det() const { assert(h_ == w_); return rank() == h_; }
	std::pair<MatrixMod2, std::vector<int>> row_echelon() const { return echelon(false); }
	std::pair<MatrixMod2, std::vector<int>> rref() const { return echelon(true); }
	std::optional<MatrixMod2> inverse() const {
		assert(h_ == w_ && w_ <= std::numeric_limits<int>::max() / 2);
		MatrixMod2 work(h_, w_ * 2);
		for (int i = 0; i < h_; ++i) {
			if (stride_) std::copy_n(raw_row(i), stride_, work.raw_row(i));
			work(i, w_ + i) = true;
		}
		if (int(work.reduce(w_, true).size()) != h_) return std::nullopt;
		return work.submatrix(0, h_, w_, w_ * 2);
	}
	std::vector<DynamicBitset> kernel() const {
		auto [work, pivots] = rref();
		return nullspace(work, pivots, w_);
	}
	std::optional<Solution> solve(const DynamicBitset& b) const {
		assert(b.size() == h_ && w_ < std::numeric_limits<int>::max());
		MatrixMod2 work(h_, w_ + 1);
		for (int i = 0; i < h_; ++i) {
			if (stride_) std::copy_n(raw_row(i), stride_, work.raw_row(i));
			if (b[i]) work(i, w_) = true;
		}
		auto pivots = work.reduce(w_, true);
		for (int i = int(pivots.size()); i < h_; ++i) if (work(i, w_)) return std::nullopt;
		Solution result{DynamicBitset(w_), nullspace(work, pivots, w_)};
		for (int i = 0; i < int(pivots.size()); ++i) if (work(i, w_)) result.particular.set(pivots[i]);
		return result;
	}
};

#endif
