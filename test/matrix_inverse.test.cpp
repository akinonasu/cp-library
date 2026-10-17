#define PROBLEM "https://judge.yosupo.jp/problem/inverse_matrix"
#include "matrix_common.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	auto inverse = read_matrix(in, n, n).inverse();
	if (inverse) write_matrix(out, *inverse);
	else out.println(-1);
}
