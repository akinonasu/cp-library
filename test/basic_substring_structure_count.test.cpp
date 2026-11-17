#define PROBLEM "https://yukicoder.me/problems/no/430"
#include "string/basic_substring_structure.hpp"
#include "string/suffix_tree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	std::string text, pattern;
	int q = 0;
	input.read(text), input.read(q);
	SuffixTree tree(text);
	BasicSubstringStructure structure(text);
	std::uint64_t answer = 0;
	while (q--) {
		input.read(pattern);
		if (auto found = tree.find(pattern)) {
			int l = tree.label_range(found->node).first;
			auto position = structure.locate(l, l + int(pattern.size()));
			assert(position);
			answer += structure.occurrence_count(*position);
		}
	}
	output.println(answer);
}
