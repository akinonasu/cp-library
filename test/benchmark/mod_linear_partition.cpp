#include "math/partition_mod_of_linear.hpp"
#include "test/benchmark/mod_linear_partition_baseline.hpp"
#include "components/fast_io.hpp"
#include <string_view>

int main(int argc, char** argv) {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	std::uint64_t hash = 0;
	auto consume = [&](ModLinearSegment part) {
		for (std::uint64_t value : {part.x, part.y, part.dx, std::uint64_t(part.dy), part.count})
			hash = (hash ^ value) * 0x9e3779b97f4a7c15ULL;
	};
	std::string_view mode = argc > 1 ? argv[1] : "callback";
	for (int i = 0; i < q; ++i) {
		std::uint64_t n = 0, m = 0, a = 0, b = 0;
		input.read(n), input.read(m), input.read(a), input.read(b);
		if (mode == "baseline") {
			auto [dx, dy, parts] = sqrt_floor(n, m, a, b);
			for (auto [x, y, count] : parts) consume({std::uint64_t(x), std::uint64_t(y), std::uint64_t(dx), dy, std::uint64_t(count)});
		} else if (mode == "vector") {
			for (auto part : partition_mod_of_linear(n, m, a, b)) consume(part);
		} else partition_mod_of_linear(n, m, a, b, consume);
	}
	output.println(hash);
}
