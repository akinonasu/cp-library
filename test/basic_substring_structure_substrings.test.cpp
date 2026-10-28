#define PROBLEM "https://judge.yosupo.jp/problem/number_of_substrings"
#include "../string/basic_substring_structure.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string s;
	in.read(s);
	BasicSubstringStructure structure(s);
	out.println(structure.distinct_substrings());
}
