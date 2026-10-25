#define PROBLEM "https://yukicoder.me/problems/no/2996"
#include "../math/generalized_floor_sum.hpp"
#include "../math/mint.hpp"
#include "../components/fast_io.hpp"

template <int D>
mint solve(int p, int q, std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b) {
	auto sums = generalized_floor_sum_moments<D, mint>(n + 1, m, a, b);
	return sums[p][q];
}

int main() {
	FastInput in;
	FastOutput out;
	int t = 0;
	in.read(t);
	while (t--) {
		int p = 0, q = 0;
		std::int64_t n = 0, m = 1, a = 0, b = 0;
		in.read(p); in.read(q); in.read(n); in.read(m); in.read(a); in.read(b);
		auto result = p <= 2 && q <= 2 ? solve<4>(p, q, n, m, a, b) : solve<20>(p, q, n, m, a, b);
		out.println(result.val());
	}
}
