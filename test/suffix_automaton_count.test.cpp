#define PROBLEM "https://yukicoder.me/problems/no/430"
#include "string/suffix_automaton.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	std::string text, pattern;
	int q = 0;
	input.read(text), input.read(q);
	SuffixAutomaton automaton(text);
	std::uint64_t answer = 0;
	while (q--) {
		input.read(pattern);
		answer += automaton.count(pattern);
	}
	output.println(answer);
}
