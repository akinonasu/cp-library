#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_totient_function"
#include "math/number_theory/totient_sum.hpp"
#include "math/mint.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint64_t n = 0;
	input.read(n);
	output.println(totient_sum<mint>(n).val());
}
