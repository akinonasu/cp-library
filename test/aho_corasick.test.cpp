#define PROBLEM "https://judge.yosupo.jp/problem/aho_corasick"
#include "string/aho_corasick.hpp"
#include "components/fast_io.hpp"
#include <string>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n;
	input.read(n);
	std::vector<std::string> patterns(n);
	int length = 0;
	for (auto &s : patterns) input.read(s), length += static_cast<int>(s.size());
	AhoCorasick<> ac;
	ac.reserve(length, n);
	for (const auto &s : patterns) ac.add(s);
	ac.build();
	output.println(ac.node_count());
	for (int v = 1; v < ac.node_count(); ++v) {
		output.write(ac.parent(v)), output.write(' '), output.println(ac.suffix_link(v));
	}
	for (int i = 0; i < n; ++i) {
		if (i) output.write(' ');
		output.write(ac.terminal(i));
	}
	output.write('\n');
}
