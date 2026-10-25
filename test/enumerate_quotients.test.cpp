#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_quotients"
#include "components/floor_quotients.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint64_t n = 0;
	input.read(n);
	FloorQuotientRange quotients(n);
	output.println(quotients.size());
	for (std::uint64_t value = 1; value <= quotients.root(); ++value) {
		output.write(std::uint32_t(value));
		output.write(value == quotients.root() && !quotients.large_size() ? '\n' : ' ');
	}
	for (auto i = quotients.root(); i < quotients.size(); ++i) {
		output.write(quotients[i]);
		output.write(i + 1 == quotients.size() ? '\n' : ' ');
	}
}
