#define PROBLEM "https://judge.yosupo.jp/problem/matrix_rank_mod_2"
#include "fixtures/matrix_mod2_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n); input.read(m);
	output.println(read_matrix_mod2(input, n, m).rank());
}
