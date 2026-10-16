#define PROBLEM "https://yukicoder.me/problems/no/2362"
#include "../math/generalized_floor_sum.hpp"
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
		auto x = generalized_floor_sum_moments<2>(n, m, a, b);
		auto y = generalized_floor_sum_moments<2>(n, m, a, 0);
		out.write(static_cast<long long>(2 * x[1][1] - (n - 1) * x[0][1] + y[1][1] - n * y[0][1]));
		out.write('\n');
	}
}
