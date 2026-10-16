#include "../../math/bigint.hpp"
#include <cassert>
#include <iterator>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

std::string remaining(std::istream &in) {
	in.clear();
	return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void compare(const std::string &text, bool skip = true) {
	std::istringstream expected(text), actual(text);
	if (!skip) {
		expected >> std::noskipws;
		actual >> std::noskipws;
	}
	int integer = 987;
	BigInt bigint = 987;
	expected >> integer;
	auto &result = actual >> bigint;
	assert(&result == &actual);
	assert(bigint == integer);
	assert(actual.rdstate() == expected.rdstate());
	assert(remaining(actual) == remaining(expected));
}

class InputBuffer : public std::streambuf {
	std::string text;
	std::size_t position = 0, failure;
	char current = 0;
	int_type underflow() override {
		if (position == failure) throw std::runtime_error("input");
		if (position == text.size()) return traits_type::eof();
		current = text[position++];
		setg(&current, &current, &current + 1);
		return traits_type::to_int_type(current);
	}
public:
	explicit InputBuffer(std::string text, std::size_t failure = std::string::npos)
		: text(std::move(text)), failure(failure) {}
};

class OutputBuffer : public std::stringbuf {
	int sync() override { ++flushes; return 0; }
public:
	int flushes = 0;
};

int main() {
	for (const std::string text : {"", " ", "\t\n", "0", "-0", "+000", "123", "-123", "123.45", "-12.75",
		"42abc", "0xFF", "12e3", ".5", "abc", "+", "-", "+x", "- 5", "+-12", "--12", "  +123,tail", "123 "}) {
		compare(text);
		compare(text, false);
	}
	compare(std::string("123\0tail", 8));
	compare(std::string(1, static_cast<char>(0xff)));
	compare("123" + std::string(1, static_cast<char>(0xff)));
	std::mt19937 random(42);
	const std::string alphabet = "0123456789+-.a ,\t\n";
	for (int trial = 0; trial < 30000; ++trial) {
		std::string text;
		for (unsigned i = 0, n = random() % 10; i < n; ++i) text += alphabet[random() % alphabet.size()];
		compare(text, (trial & 1) != 0);
	}
	for (int length : {1, 8, 17, 18, 19, 36, 37, 1000, 200003}) {
		std::string digits(length, '0');
		for (char &digit : digits) digit = static_cast<char>('0' + random() % 10);
		digits[0] = '7';
		for (const std::string sign : {"", "+", "-"}) {
			std::istringstream in(" \t" + sign + digits + ".tail");
			BigInt value;
			in >> value;
			assert(value.str() == (sign == "-" ? "-" : "") + digits);
			assert(in.good());
			assert(remaining(in) == ".tail");
		}
	}
	{
		std::istringstream in("123.45 -6+7");
		BigInt a, b, c, d;
		char point = 0;
		in >> a >> point >> b >> c >> d;
		assert(a == 123 && point == '.' && b == 45 && c == -6 && d == 7);
		assert(in.rdstate() == std::ios_base::eofbit);
	}
	for (auto state : {std::ios_base::failbit, std::ios_base::badbit, std::ios_base::eofbit}) {
		std::istringstream expected("123"), actual("123");
		expected.setstate(state);
		actual.setstate(state);
		int integer = 987;
		BigInt bigint = 987;
		expected >> integer;
		actual >> bigint;
		assert(bigint == integer);
		assert(actual.rdstate() == expected.rdstate());
		assert(remaining(actual) == remaining(expected));
	}
	for (auto mask : {std::ios_base::failbit, std::ios_base::eofbit, std::ios_base::badbit,
		std::ios_base::failbit | std::ios_base::eofbit | std::ios_base::badbit}) {
		for (const std::string text : {"", " ", "123", "123.", ".5", "-", "+x"}) {
			std::istringstream expected(text), actual(text);
			expected.exceptions(mask);
			actual.exceptions(mask);
			int integer = 987;
			BigInt bigint = 987;
			bool expected_throw = false, actual_throw = false;
			try { expected >> integer; } catch (const std::ios_base::failure &) { expected_throw = true; }
			try { actual >> bigint; } catch (const std::ios_base::failure &) { actual_throw = true; }
			assert(actual_throw == expected_throw);
			assert(bigint == integer);
			assert(actual.rdstate() == expected.rdstate());
			actual.exceptions(std::ios_base::goodbit);
			expected.exceptions(std::ios_base::goodbit);
			assert(remaining(actual) == remaining(expected));
		}
	}
	{
		InputBuffer buffer(" -123456789012345678901234567890.xyz");
		std::istream in(&buffer);
		BigInt value;
		in >> value;
		assert(value.str() == "-123456789012345678901234567890");
		assert(in.good());
		assert(remaining(in) == ".xyz");
	}
	for (bool throwing : {false, true}) {
		InputBuffer buffer("12345", 3);
		std::istream in(&buffer);
		if (throwing) in.exceptions(std::ios_base::badbit);
		BigInt value = 987;
		bool caught = false;
		try { in >> value; } catch (const std::runtime_error &error) {
			caught = true;
			assert(std::string(error.what()) == "input");
		}
		assert(caught == throwing);
		assert(in.rdstate() == std::ios_base::badbit);
		assert(value == 987);
	}
	{
		OutputBuffer buffer;
		std::ostream out(&buffer);
		std::istringstream in("123456789012345678901234567890.");
		in.tie(&out);
		BigInt value;
		in >> value;
		assert(value.str() == "123456789012345678901234567890");
		assert(buffer.flushes == 1);
	}
}
