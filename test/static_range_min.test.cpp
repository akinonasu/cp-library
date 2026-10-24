#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include "components/static_range_min.hpp"
#include "components/fast_io_bulk.hpp"
#include <cstdint>
#include <array>
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
	std::array<std::pair<int, int>, 64> queries;
	std::size_t count = 0;
	auto flush = [&] {
		for (auto value : range.prod_many(std::span(queries.data(), count))) output.println(value);
		count = 0;
	};
	read_unsigned_records<2>(input, q, [&](std::uint32_t l, std::uint32_t r) {
		queries[count++] = {int(l), int(r)};
		if (count == queries.size()) flush();
	});
	if (count) flush();
}
