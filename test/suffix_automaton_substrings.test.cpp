#define PROBLEM "https://judge.yosupo.jp/problem/number_of_substrings"
#include "string/suffix_automaton.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	SuffixAutomaton automaton(s);
	output.println(automaton.distinct_count());
}
