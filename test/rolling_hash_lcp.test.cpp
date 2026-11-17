#define PROBLEM "https://judge.yosupo.jp/problem/zalgorithm"
#include "string/rolling_hash.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	RollingHash hash(s);
	for (int i = 0; i < hash.size(); ++i) {
		if (i) output.write(' ');
		output.write(hash.lcp(0, hash, i));
	}
	output.write('\n');
}
