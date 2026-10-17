#define PROBLEM "https://judge.yosupo.jp/problem/matrix_rank"
#include "matrix_common.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n); in.read(m);
	out.println(read_matrix(in, n, m).rank());
}
