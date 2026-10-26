#define PROBLEM "https://judge.yosupo.jp/problem/point_add_rectangle_sum"
#include "../data_structure/weighted_wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include "../components/radix_sort.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	struct Query { int type, x, y, r, u; std::uint32_t weight; };
	std::vector<Query> queries(q);
	std::vector<int> xs, values;
	std::vector<std::int64_t> weights;
	{
		struct Point { int x, y, id; std::uint32_t weight; };
		std::vector<Point> points(n);
		for (auto &p : points) input.read(p.x), input.read(p.y), input.read(p.weight), p.id = -1;
		for (int i = 0; i < q; ++i) {
			auto &z = queries[i];
			input.read(z.type); input.read(z.x); input.read(z.y);
			if (!z.type) input.read(z.weight), points.push_back({z.x, z.y, i, 0});
			else input.read(z.r), input.read(z.u);
		}
		stable_radix_sort<11>(points, [](const Point &p) { return (std::uint64_t(std::uint32_t(p.x)) << 32) | std::uint32_t(p.y); });
		xs.reserve(points.size()); values.reserve(points.size()); weights.reserve(points.size());
		int previous_x = -1, previous_y = -1;
		for (auto p : points) {
			if (!xs.empty() && p.x == previous_x && p.y == previous_y) weights.back() += p.weight;
			else xs.push_back(p.x), values.push_back(p.y), weights.push_back(p.weight);
			previous_x = p.x; previous_y = p.y;
			if (p.id >= 0) queries[p.id].x = int(weights.size()) - 1;
		}
	}
	{
		std::vector<std::pair<int, int>> bounds;
		for (int i = 0; i < q; ++i) if (queries[i].type) {
			bounds.emplace_back(queries[i].x, i * 2);
			bounds.emplace_back(queries[i].r, i * 2 + 1);
		}
		stable_radix_sort<11>(bounds, [](const auto &v) { return unsigned(v.first); });
		int p = 0;
		for (auto [x, id] : bounds) {
			while (p < int(xs.size()) && xs[p] < x) ++p;
			if (id & 1) queries[id / 2].r = p;
			else queries[id / 2].x = p;
		}
	}
	WeightedWaveletMatrix<int, std::int64_t, __int128_t> wm(values, weights, false);
	for (auto z : queries) {
		if (!z.type) wm.add_weight(z.x, z.weight);
		else output.println(wm.range_freq(z.x, z.r, z.y, z.u));
	}
}
