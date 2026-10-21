#define PROBLEM "https://judge.yosupo.jp/problem/longest_common_substring"
#include "string/longest_common_substring.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s, t;
	input.read(s);
	input.read(t);
	auto [a, b, length] = longest_common_substring(s, t);
	output.write(a);
	output.write(' ');
	output.write(a + length);
	output.write(' ');
	output.write(b);
	output.write(' ');
	output.println(b + length);
}
