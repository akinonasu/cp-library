#define PROBLEM "https://judge.yosupo.jp/problem/matrix_product"
#include "matrix_common.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0, k = 0;
	in.read(n); in.read(m); in.read(k);
	auto a = read_matrix(in, n, m), b = read_matrix(in, m, k);
	write_matrix(out, a * b);
}
