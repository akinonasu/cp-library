#include "components/fast_io_output_values.hpp"
#include <cassert>
#include <charconv>
#include <cstdio>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

template<class UInt> void check(const std::vector<std::uint64_t> &input, char separator, char ending) {
	FILE *file = std::tmpfile();
	assert(file);
	std::string expected = "prefix\n";
	for (std::size_t i = 0; i < input.size(); ++i) {
		char digits[32];
		auto end = std::to_chars(digits, digits + sizeof(digits), UInt(input[i]));
		expected.append(digits, end.ptr);
		expected.push_back(i + 1 == input.size() ? ending : separator);
	}
	expected += "suffix";
	{
		FastOutput out(file);
		out.write("prefix\n");
		std::size_t calls = 0;
		write_unsigned_values<UInt>(out, input.size(), [&](std::size_t i) {
			assert(i == calls++);
			return UInt(input[i]);
		}, separator, ending);
		assert(calls == input.size());
		write_unsigned_values(out, 0, [](std::size_t) { assert(false); return std::uint64_t(0); });
		out.write("suffix");
		assert(out.flush());
	}
	std::rewind(file);
	std::string actual;
	char buffer[4096];
	for (std::size_t size; (size = std::fread(buffer, 1, sizeof(buffer), file));) actual.append(buffer, size);
	assert(actual == expected);
	std::fclose(file);
}
int main() {
	std::vector<std::uint64_t> values{0, 1, UINT32_MAX, UINT64_MAX};
	for (std::uint64_t x = 10;;) {
		values.insert(values.end(), {x - 1, x, x + 1});
		if (x > UINT64_MAX / 10) break;
		x *= 10;
	}
	std::mt19937_64 random(20261112);
	for (int i = 0; i < 20000; ++i) values.push_back(random());
	for (std::size_t n : {0,1,2,3,3120,3121,3122,5956,5957,5958,20000}) {
		std::vector<std::uint64_t> subset(values.begin(), values.begin() + n);
		check<std::uint8_t>(subset, ' ', '\n');
		check<std::uint16_t>(subset, ',', ';');
		check<std::uint32_t>(subset, '\t', '\n');
		check<std::uint64_t>(subset, ' ', '\n');
	}
}
