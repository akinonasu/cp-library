#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_quotients"
#include "components/floor_quotients.hpp"
#include "components/fast_io_output_values.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint64_t n = 0;
	input.read(n);
	FloorQuotientRange quotients(n);
	output.println(quotients.size());
	write_unsigned_values<std::uint32_t>(output, quotients.root(), [&](std::size_t i) { return i + 1; }, ' ', quotients.large_size() ? ' ' : '\n');
	write_unsigned_values(output, quotients.large_size(), [&](std::size_t i) { return quotients[quotients.root() + i]; });
}
