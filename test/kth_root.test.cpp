#define PROBLEM "https://judge.yosupo.jp/problem/kth_root_integer"
#include "math/kth_root.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint64_t n = 0, k = 0;
		input.read(n), input.read(k);
		output.println(kth_root(n, k));
	}
}
