#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_14_D"
#include "string/suffix_automaton.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string text, pattern;
	int q = 0;
	input.read(text), input.read(q);
	SuffixAutomaton automaton(text);
	while (q--) {
		input.read(pattern);
		output.println(automaton.contains(pattern) ? 1 : 0);
	}
}
