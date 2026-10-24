#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"
#include "components/fast_io_bulk.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::int64_t> prefix(n + 1);
	std::size_t position = 1;
	std::int64_t sum = 0;
	read_unsigned_blocks(input, n, [&](const std::uint32_t *values, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) prefix[position++] = sum += values[i];
	});
	read_unsigned_records<2>(input, q, [&](std::uint32_t l, std::uint32_t r) {
		output.println(prefix[r] - prefix[l]);
	});
}
