#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod_1000000007"
#include "../convolution/convolution_mod.hpp"
#include "../components/fast_io_bulk.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n);
	input.read(m);
	std::vector<std::uint32_t> a(n), b(m);
	for (auto *values : {&a, &b}) {
		std::size_t offset = 0;
		read_unsigned_blocks(input, values->size(), [&](const std::uint32_t *block, std::size_t count) {
			std::copy(block, block + count, values->data() + offset);
			offset += count;
		});
	}
	auto result = convolution_mod(a, b, 1000000007);
	write_unsigned_records<1>(output, result.size(), [&](std::size_t i) {
		return std::array<std::uint32_t, 1>{result[i]};
	});
}
