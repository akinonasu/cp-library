#define PROBLEM "https://judge.yosupo.jp/problem/adjugate_matrix"
#include "fixtures/matrix_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	write_light_matrix(output, read_light_matrix(input, n, n).adjugate());
}
