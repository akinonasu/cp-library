#define PROBLEM "https://judge.yosupo.jp/problem/characteristic_polynomial"
#include "matrix_common.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	auto p = read_matrix(in, n, n).characteristic_polynomial();
	for (int i = 0; i <= n; ++i) { out.write(p[i].val()); out.write(i == n ? '\n' : ' '); }
}
