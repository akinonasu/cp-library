#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_formal_power_series"
#include "fps/fps.hpp"
#include "fixtures/mint_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	fps denominator(n), numerator(n);
	read_mints(input, denominator);
	numerator[0] = 1;
	write_mints(output, numerator / denominator);
}
