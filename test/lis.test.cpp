#define PROBLEM "https://judge.yosupo.jp/problem/longest_increasing_subsequence"
#include "../sequence/lis.hpp"
#include "../components/fast_io_bulk.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	std::vector<int> a(n);
	std::size_t position = 0;
	read_unsigned_blocks(in, n, [&](const std::uint32_t *values, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) a[position++] = static_cast<int>(values[i]);
	});
	auto indices = lis(a);
	out.println(indices.size());
	write_unsigned_records<1>(out, indices.size(), [&](std::size_t i) {
		return std::array<std::uint32_t, 1>{static_cast<std::uint32_t>(indices[i])};
	});
}
