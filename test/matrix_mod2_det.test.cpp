#define PROBLEM "https://judge.yosupo.jp/problem/matrix_det_mod_2"
#include "fixtures/matrix_mod2_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	output.println(int(read_matrix_mod2(input, n, n).det()));
}
