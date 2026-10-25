#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_floor_of_linear"
#include "../math/floor_sum.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int t;
	in.read(t);
	while (t--) {
		std::uint64_t n, m, a, b;
		in.read(n);
		in.read(m);
		in.read(a);
		in.read(b);
		out.write(floor_sum(n, m, a, b));
		out.write('\n');
	}
}
