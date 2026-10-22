#define PROBLEM "https://judge.yosupo.jp/problem/matrix_det"
#include "fixtures/matrix_common.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	out.println(read_matrix(in, n, n).det().val());
}
