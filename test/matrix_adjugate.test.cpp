#define PROBLEM "https://judge.yosupo.jp/problem/adjugate_matrix"
#include "fixtures/matrix_common.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	write_matrix(out, read_matrix(in, n, n).adjugate());
}
