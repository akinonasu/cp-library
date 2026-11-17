#define PROBLEM "https://yukicoder.me/problems/no/273"
#include "string/eertree.hpp"
#include "components/fast_io.hpp"
#include <algorithm>

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	std::string_view text(s);
	Eertree tree(text.substr(0, text.size() - 1));
	int answer = tree.length(tree.longest_palindrome());
	tree.clear();
	tree.append(text.substr(1));
	output.println(std::max(answer, tree.length(tree.longest_palindrome())));
}
