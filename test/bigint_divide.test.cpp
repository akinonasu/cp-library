#define PROBLEM "https://judge.yosupo.jp/problem/division_of_big_integers"
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
		auto [q, r] = divmod(a, b);
		output.write(q.to_string());
		output.write(' ');
		output.println(r.to_string());
	}
}
