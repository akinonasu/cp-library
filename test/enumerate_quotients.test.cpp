#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_quotients"
#include "components/floor_quotients.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint64_t n = 0;
	input.read(n);
	FloorQuotients quotients(n);
	output.println(quotients.size());
	const auto &values = quotients.values();
	for (auto i = values.size(); i--;) {
		output.write(values[i]);
		output.write(i ? ' ' : '\n');
	}
}
