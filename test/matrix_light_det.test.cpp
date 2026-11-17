#define PROBLEM "https://judge.yosupo.jp/problem/matrix_det"
#include "fixtures/matrix_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	output.println(read_light_matrix(input, n, n).det().val());
}
