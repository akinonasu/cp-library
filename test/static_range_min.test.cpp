#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include "components/static_range_min.hpp"
#include "components/fast_io_bulk.hpp"
#include <cstdint>
#include <utility>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::uint32_t> values(n);
	read_unsigned(input, values);
	StaticRangeMin range(std::move(values));
	read_unsigned_records<2>(input, q, [&](std::uint32_t l, std::uint32_t r) {
		output.println(range.prod(l, r));
	});
}
