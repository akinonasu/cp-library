#define PROBLEM "https://judge.yosupo.jp/problem/multiplication_of_big_integers"
#include "math/bigint.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		BigInt a, b;
		input.read(a), input.read(b);
		output.println((a * b).to_string());
	}
}
