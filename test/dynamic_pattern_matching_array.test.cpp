#define PROBLEM "https://judge.yosupo.jp/problem/suffixarray"
#include "../string/dynamic_pattern_matching.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string text;
	in.read(text);
	DynamicPatternMatching matching(text);
	const auto &sa = matching.suffix_array();
	for (std::size_t i = 1; i < sa.size(); ++i) { if (i != 1) out.write(' '); out.write(sa[i]); }
	out.write('\n');
}
