#define PROBLEM "https://judge.yosupo.jp/problem/multivariate_convolution"
#include "../convolution/multivariate_convolution_light.hpp"
#include "../components/fast_io_bulk.hpp"
#include <array>
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int k = 0, n = 1;
	input.read(k);
	std::vector<int> dimensions(k);
	for (auto &d : dimensions) input.read(d), n *= d;
	std::vector<mint> a(n), b(n);
	for (auto *v : {&a, &b}) {
		std::size_t i = 0;
		read_unsigned_blocks(input, n, [&](const std::uint32_t *values, std::size_t count) {
			for (std::size_t j = 0; j < count; ++j) (*v)[i++] = mint::raw(values[j]);
		});
	}
	const auto result = light_convolution::multivariate_convolution(a, b, dimensions);
	write_unsigned_records<1>(output, result.size(), [&](std::size_t i) {
		return std::array{result[i].val()};
	});
}
