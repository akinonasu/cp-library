#define PROBLEM "https://yukicoder.me/problems/no/263"
#include "string/eertree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	std::string s, t;
	input.read(s), input.read(t);
	Eertree tree;
	tree.reserve(int(s.size() + t.size() + 1));
	tree.append(s);
	auto before = tree.occurrences();
	tree.push_back('#');
	tree.append(t);
	auto after = tree.occurrences();
	std::uint64_t answer = 0;
	for (int v = 2; v < int(before.size()); ++v) answer += before[v] * (after[v] - before[v]);
	output.println(answer);
}
