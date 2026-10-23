#define PROBLEM "https://judge.yosupo.jp/problem/zalgorithm"
#include "string/data_dependent_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	DataDependentTree tree;
	int root = tree.build(s);
	for (std::size_t i = 0; i < s.size(); ++i) {
		if (i) output.write(' ');
		output.write(tree.lcp(root, 0, root, i));
	}
	output.write('\n');
}
