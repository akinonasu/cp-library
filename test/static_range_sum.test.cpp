#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"
#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::int64_t> prefix(n + 1);
	for (int i = 0; i < n; ++i) { std::int64_t value = 0; input.read(value); prefix[i + 1] = prefix[i] + value; }
	while (q--) {
		int l = 0, r = 0;
		input.read(l), input.read(r);
		output.println(prefix[r] - prefix[l]);
	}
}
