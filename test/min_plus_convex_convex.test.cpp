#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_convex"
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
	std::vector<std::uint32_t> a(n), b(m);
	read_unsigned(input, a), read_unsigned(input, b);
	const auto result = min_plus_convolution_convex(a, b);
	write_unsigned_records<1>(output, result.size(), [&](std::size_t i) {
		return std::array{result[i]};
	});
}
