#define PROBLEM "https://judge.yosupo.jp/problem/pow_of_matrix"
#include "matrix_common.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	std::uint64_t k = 0;
	in.read(n); in.read(k);
	write_matrix(out, read_matrix(in, n, n).pow(k));
}
