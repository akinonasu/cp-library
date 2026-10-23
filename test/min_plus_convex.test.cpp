#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_arbitrary"
#include "../convolution/min_plus_convolution.hpp"
#include "../components/fast_io_bulk.hpp"
#include <array>
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	std::vector<std::int64_t> a(n), b(m);
	auto read = [&](std::vector<std::int64_t> &values) {
		std::size_t offset = 0;
		read_unsigned_blocks(input, values.size(), [&](const std::uint32_t *block, std::size_t count) {
			for (std::size_t i = 0; i < count; ++i) values[offset + i] = block[i];
			offset += count;
		});
	};
	read(a), read(b);
	const auto result = min_plus_convolution_convex(a, b);
	write_unsigned_records<1>(output, result.size(), [&](std::size_t i) {
		return std::array{std::uint32_t(result[i])};
	});
}
