#define PROBLEM "https://judge.yosupo.jp/problem/suffixarray"
#include "../string/suffix_array.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	for (int x : suffix_array(s)) { output.write(x); output.write(' '); }
	output.write('\n');
}
