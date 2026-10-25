#define PROBLEM "https://judge.yosupo.jp/problem/point_add_rectangle_sum"
#include "../data_structure/weighted_wavelet_matrix.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	struct Point { int x, y; std::int64_t weight; };
	struct Query { int type, x, y, r, u; std::int64_t weight; };
	std::vector<Point> points(n);
	for (auto &p : points) input.read(p.x), input.read(p.y), input.read(p.weight);
	std::vector<Query> queries(q);
	for (auto &z : queries) {
		input.read(z.type); input.read(z.x); input.read(z.y);
		if (!z.type) input.read(z.weight), points.push_back({z.x, z.y, 0});
		else input.read(z.r), input.read(z.u);
	}
	std::sort(points.begin(), points.end(), [](auto a, auto b) { return std::pair(a.x, a.y) < std::pair(b.x, b.y); });
	std::vector<std::pair<int, int>> coordinates;
	std::vector<int> values;
	std::vector<std::int64_t> weights;
	for (auto p : points) {
		if (!coordinates.empty() && coordinates.back() == std::pair(p.x, p.y)) weights.back() += p.weight;
		else coordinates.emplace_back(p.x, p.y), values.push_back(p.y), weights.push_back(p.weight);
	}
	WeightedWaveletMatrix<int, std::int64_t, __int128_t> wm(values, weights, false);
	for (auto z : queries) {
		if (!z.type) {
			int p = std::lower_bound(coordinates.begin(), coordinates.end(), std::pair(z.x, z.y)) - coordinates.begin();
			wm.add_weight(p, z.weight);
		} else {
			int l = std::lower_bound(coordinates.begin(), coordinates.end(), std::pair(z.x, -1)) - coordinates.begin();
			int r = std::lower_bound(coordinates.begin(), coordinates.end(), std::pair(z.r, -1)) - coordinates.begin();
			output.println(wm.range_freq(l, r, z.y, z.u));
		}
	}
}
