#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_5_D"
#include "sequence/inversion_count.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<int> values(n);
	for (int &value : values) input.read(value);
	output.println(inversion_count(values));
}
