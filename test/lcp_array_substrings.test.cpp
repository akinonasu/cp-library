#define PROBLEM "https://judge.yosupo.jp/problem/number_of_substrings"
#include "string/suffix_array.hpp"
#include "string/lcp_array.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	auto lcp = lcp_array(s, suffix_array(s));
	std::uint64_t answer = std::uint64_t(s.size()) * (s.size() + 1) / 2;
	for (int length : lcp) answer -= length;
	output.println(answer);
}
