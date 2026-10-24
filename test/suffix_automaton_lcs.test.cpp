#define PROBLEM "https://judge.yosupo.jp/problem/longest_common_substring"
#include "string/suffix_automaton.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s, t;
	input.read(s), input.read(t);
	SuffixAutomaton automaton(s);
	auto match = automaton.longest_common_substring(t);
	output.write(match.text_pos), output.write(' ');
	output.write(match.text_pos + match.length), output.write(' ');
	output.write(match.query_pos), output.write(' ');
	output.println(match.query_pos + match.length);
}
