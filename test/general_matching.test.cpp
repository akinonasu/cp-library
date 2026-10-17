#define PROBLEM "https://judge.yosupo.jp/problem/general_matching"
#include "../graph/general_matching.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n);
	in.read(m);
	GeneralMatching g(n);
	g.reserve(m);
	for (int i = 0, u = 0, v = 0; i < m; ++i) {
		in.read(u);
		in.read(v);
		g.add_edge(u, v);
	}
	out.write(g.build());
	out.write('\n');
	for (auto [u, v] : g.edges()) {
		out.write(u);
		out.write(' ');
		out.write(v);
		out.write('\n');
	}
}
