#ifndef BIGINT_HPP
#define BIGINT_HPP

#include "../components/bigint_magnitude.hpp"
#include <algorithm>
#include <charconv>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

class BigInt {
	using Word = bigint_detail::Word;
	using Magnitude = bigint_detail::Magnitude;
	static constexpr Word base = bigint_detail::base;
	Magnitude digits_;
	int sign_ = 0;

	BigInt(Magnitude digits, int sign) : digits_(std::move(digits)), sign_(digits_.empty() ? 0 : sign) {}
	void add_magnitude(const Magnitude &other) { bigint_detail::add(digits_, other); }
	void subtract_magnitude(const Magnitude &other) {
		bigint_detail::subtract(digits_, other);
		if (digits_.empty()) sign_ = 0;
	}
	BigInt &add(const BigInt &other, int factor) {
		int sign = other.sign_ * factor;
		if (!sign) return *this;
		if (!sign_) {
			digits_ = other.digits_;
			sign_ = sign;
		} else if (sign_ == sign) add_magnitude(other.digits_);
		else {
			int order = bigint_detail::compare(digits_, other.digits_);
			if (!order) digits_.clear(), sign_ = 0;
			else if (order > 0) subtract_magnitude(other.digits_);
			else {
				BigInt result(other);
				result.subtract_magnitude(digits_);
				result.sign_ = sign;
				*this = std::move(result);
			}
		}
		return *this;
	}

public:
	BigInt() = default;
	BigInt(const BigInt &) = default;
	BigInt &operator=(const BigInt &) = default;
	BigInt(BigInt &&other) noexcept : digits_(std::move(other.digits_)), sign_(std::exchange(other.sign_, 0)) { other.digits_.clear(); }
	BigInt &operator=(BigInt &&other) noexcept {
		if (this != &other) {
			digits_ = std::move(other.digits_);
			sign_ = std::exchange(other.sign_, 0);
			other.digits_.clear();
		}
		return *this;
	}
	template <class T> requires (std::numeric_limits<T>::is_integer && sizeof(T) <= 16)
	BigInt(T value) {
		using U = std::conditional_t<(sizeof(T) <= 8), std::uint64_t, __uint128_t>;
		U magnitude = U(value);
		if constexpr (std::numeric_limits<T>::is_signed)
			if (value < 0) magnitude = U(0) - magnitude, sign_ = -1;
		while (magnitude) {
			digits_.push_back(Word(magnitude % base));
			magnitude /= base;
		}
		if (!digits_.empty() && !sign_) sign_ = 1;
	}
	explicit BigInt(std::string_view text) {
		if (text.empty()) throw std::invalid_argument("BigInt decimal string");
		std::size_t first = text.front() == '+' || text.front() == '-';
		if (first == text.size()) throw std::invalid_argument("BigInt decimal string");
		bool negative = text.front() == '-';
		while (first < text.size() && text[first] == '0') ++first;
		if (first == text.size()) return;
		digits_.reserve((text.size() - first - 1) / 9 + 1);
		for (std::size_t end = text.size(); end > first;) {
			std::size_t begin = end - first > 9 ? end - 9 : first;
			Word value = 0;
			for (std::size_t i = begin; i < end; ++i) {
				unsigned digit = unsigned(text[i] - '0');
				if (digit > 9) throw std::invalid_argument("BigInt decimal string");
				value = value * 10 + digit;
			}
			digits_.push_back(value);
			end = begin;
		}
		sign_ = negative ? -1 : 1;
	}
	int sign() const { return sign_; }
	explicit operator bool() const { return sign_ != 0; }
	std::string to_string() const {
		if (!sign_) return "0";
		char top[10];
		std::size_t length = std::to_chars(top, top + 10, digits_.back()).ptr - top;
		std::string result((sign_ < 0) + length + 9 * (digits_.size() - 1), '0');
		if (sign_ < 0) result[0] = '-';
		std::copy_n(top, length, result.begin() + (sign_ < 0));
		std::size_t end = result.size();
		for (std::size_t i = 0; i + 1 < digits_.size(); ++i) {
			Word value = digits_[i];
			for (int j = 0; j < 9; ++j) result[--end] = char('0' + value % 10), value /= 10;
		}
		return result;
	}
	BigInt operator+() const { return *this; }
	BigInt operator-() const {
		BigInt result(*this);
		result.sign_ = -result.sign_;
		return result;
	}
	BigInt &operator+=(const BigInt &other) { return add(other, 1); }
	BigInt &operator-=(const BigInt &other) { return add(other, -1); }
	BigInt &operator*=(const BigInt &other) { return *this = *this * other; }
	BigInt &operator/=(const BigInt &other) { return *this = divmod(*this, other).first; }
	BigInt &operator%=(const BigInt &other) { return *this = divmod(*this, other).second; }
	BigInt &operator++() { return *this += 1; }
	BigInt &operator--() { return *this -= 1; }
	BigInt operator++(int) { BigInt old(*this); ++*this; return old; }
	BigInt operator--(int) { BigInt old(*this); --*this; return old; }
	friend BigInt operator+(BigInt a, const BigInt &b) { a += b; return a; }
	friend BigInt operator-(BigInt a, const BigInt &b) { a -= b; return a; }
	friend BigInt operator*(const BigInt &a, const BigInt &b) {
		if (!a || !b) return {};
		return BigInt(bigint_detail::multiply(a.digits_, b.digits_), a.sign_ * b.sign_);
	}
	friend BigInt operator/(const BigInt &a, const BigInt &b) { return divmod(a, b).first; }
	friend BigInt operator%(const BigInt &a, const BigInt &b) { return divmod(a, b).second; }
	friend std::pair<BigInt, BigInt> divmod(const BigInt &a, const BigInt &b) {
		if (!b) throw std::domain_error("BigInt division by zero");
		auto [quotient, remainder] = bigint_detail::divide(a.digits_, b.digits_);
		return {BigInt(std::move(quotient), a.sign_ * b.sign_), BigInt(std::move(remainder), a.sign_)};
	}
	friend BigInt abs(BigInt value) { if (value.sign_ < 0) value.sign_ = 1; return value; }
	friend bool operator==(const BigInt &a, const BigInt &b) { return a.sign_ == b.sign_ && a.digits_ == b.digits_; }
	friend std::strong_ordering operator<=>(const BigInt &a, const BigInt &b) {
		if (a.sign_ != b.sign_) return a.sign_ <=> b.sign_;
		return (a.sign_ * bigint_detail::compare(a.digits_, b.digits_)) <=> 0;
	}
	friend std::ostream &operator<<(std::ostream &out, const BigInt &value) { return out << value.to_string(); }
	friend std::istream &operator>>(std::istream &in, BigInt &value) {
		std::string text;
		if (!(in >> text)) return in;
		try { value = BigInt(text); }
		catch (const std::invalid_argument &) { in.setstate(std::ios::failbit); }
		return in;
	}
};

#endif
