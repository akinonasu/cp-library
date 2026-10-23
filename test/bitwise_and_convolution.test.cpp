#define PROBLEM "https://judge.yosupo.jp/problem/bitwise_and_convolution"
#include "../convolution/bitwise_convolution.hpp"
#include "../components/fast_io_bulk.hpp"
#include "../math/mint.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int exponent = 0;
	input.read(exponent);
	int n = 1 << exponent;
	std::vector<mint> a(n), b(n);
	for (auto *values : {&a, &b}) {
		std::size_t offset = 0;
		read_unsigned_blocks(input, n, [&](const std::uint32_t *block, std::size_t count) {
			for (std::size_t i = 0; i < count; ++i) (*values)[offset + i] = mint::raw(block[i]);
			offset += count;
		});
	}
	auto result = and_convolution(std::move(a), std::move(b));
	write_unsigned_records<1>(output, result.size(), [&](std::size_t i) {
		return std::array<std::uint32_t, 1>{result[i].val()};
	});
}
