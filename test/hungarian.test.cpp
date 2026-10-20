#define PROBLEM "https://judge.yosupo.jp/problem/assignment"
#include "../graph/matching/hungarian.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n;
	in.read(n);
	std::vector<int> a(std::size_t(n) * n);
	for (auto &x : a) in.read(x);
	Hungarian res(n, n, a.data());
	out.write(res.cost);
	out.write('\n');
	for (int i = 0; i < n; ++i) {
		if (i) out.write(' ');
		out.write(res.row_to_col[i]);
	}
	out.write('\n');
}
