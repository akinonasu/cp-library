#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_path_sum"
#include "tree/lca.hpp"
#include "components/fenwick_sum.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<long long> values(n);
	int vertex = 0;
	read_unsigned_records<1>(input, n, [&](unsigned value) { values[vertex++] = value; });
	LCA tree(n);
	read_unsigned_records<2>(input, n - 1, [&](unsigned u, unsigned v) { tree.add_edge(u, v); });
	tree.build();
	std::vector<int> first(n), last(n);
	std::vector<long long> difference(n);
	for (int v = 0; v < n; ++v) {
		auto [l, r] = tree.subtree(v);
		first[v] = l, last[v] = r;
		difference[l] += values[v];
		if (r < n) difference[r] -= values[v];
	}
	FenwickSum<long long> sums(difference);
	read_unsigned_records<3>(input, q, [&](unsigned type, unsigned u, unsigned x) {
		if (!type) {
			values[u] += x;
			if (last[u] < n) sums.add_pair(first[u], x, last[u], -static_cast<long long>(x));
			else sums.add(first[u], x);
		} else {
			int w = tree.lca(u, x);
			long long answer = sums.prefix(first[u] + 1) + sums.prefix(first[x] + 1) - 2 * sums.prefix(first[w] + 1) + values[w];
			output.println(answer);
		}
	});
}
