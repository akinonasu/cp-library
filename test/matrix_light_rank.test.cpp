#define PROBLEM "https://judge.yosupo.jp/problem/matrix_rank"
#include "fixtures/matrix_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	output.println(read_light_matrix(input, n, m).rank());
}
