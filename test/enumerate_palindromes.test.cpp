#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_palindromes"
#include "string/manacher.hpp"
#include "components/fast_io_output_values.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	auto lengths = palindrome_lengths(s);
	if (lengths.empty()) output.write('\n');
	else write_unsigned_values<unsigned>(output, lengths.size(), [&](std::size_t i) { return unsigned(lengths[i]); });
}
