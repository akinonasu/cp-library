#include "string/eertree.hpp"
#include "components/fast_io.hpp"
#include "eertree_baseline.hpp"
#include <string>

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
#if EERTREE_DENSE
	DenseEertree tree(s);
#else
	Eertree tree(s);
#endif
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
