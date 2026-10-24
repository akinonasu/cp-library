#define PROBLEM "https://judge.yosupo.jp/problem/rectangle_sum"
#include "../data_structure/offline_rectangle_sum.hpp"
#include "../components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	OfflineRectangleSum<int, int, std::int64_t> solver(n, q);
	read_unsigned_records<3>(input, n, [&](std::uint32_t x, std::uint32_t y, std::uint32_t w) { solver.add_point(x, y, w); });
	read_unsigned_records<4>(input, q, [&](std::uint32_t l, std::uint32_t d, std::uint32_t r, std::uint32_t u) { solver.add_query(l, d, r, u); });
	for (auto answer : solver.solve()) output.println(answer);
}
