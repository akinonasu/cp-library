#define PROBLEM "https://judge.yosupo.jp/problem/eertree"
#include "string/eertree.hpp"
#include "components/fast_io.hpp"
#include <string>

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	Eertree tree(s);
	auto number = [](int v) { return v - 1; };
	output.println(tree.distinct_count());
	for (int v = 2; v < tree.node_count(); ++v) {
		output.write(number(tree.parent(v))), output.write(' '), output.println(number(tree.suffix_link(v)));
	}
	const auto &prefix = tree.prefix_nodes();
	for (int i = 0; i < tree.size(); ++i) {
		if (i) output.write(' ');
		output.write(number(prefix[i]));
	}
	output.write('\n');
}
