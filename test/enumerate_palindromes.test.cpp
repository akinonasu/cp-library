#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_palindromes"
#include "string/manacher.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	auto lengths = Manacher(s).lengths();
	for (std::size_t i = 0; i < lengths.size(); ++i) {
		if (i) output.write(' ');
		output.write(lengths[i]);
	}
	output.write('\n');
}
