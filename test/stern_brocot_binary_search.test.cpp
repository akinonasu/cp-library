#define PROBLEM "https://judge.yosupo.jp/problem/rational_approximation"
#include "../math/stern_brocot_tree.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int t;
	in.read(t);
	while (t--) {
		std::uint64_t bound, x, y;
		in.read(bound); in.read(x); in.read(y);
		auto [l, r] = SternBrocotTree<>::binary_search(bound, [&](auto a, auto b) { return __uint128_t(a) * y <= __uint128_t(x) * b; });
		if (__uint128_t(l.first) * y == __uint128_t(x) * l.second) r = l;
		out.write(l.first); out.write(' '); out.write(l.second); out.write(' ');
		out.write(r.first); out.write(' '); out.println(r.second);
	}
}
