#define PROBLEM "https://judge.yosupo.jp/problem/zalgorithm"
#include "string/z_algorithm.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	auto z = z_algorithm(s);
	for (int i = 0; i < int(z.size()); ++i) {
		if (i) output.write(' ');
		output.write(z[i]);
	}
	output.write('\n');
}
