#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_2_C"
#include "../data_structure/wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	struct Point { std::int64_t x, y; int id; };
	std::vector<Point> points(n);
	for (int i = 0; i < n; ++i) in.read(points[i].x), in.read(points[i].y), points[i].id = i;
	std::sort(points.begin(), points.end(), [](auto a, auto b) { return a.x < b.x; });
	std::vector<std::int64_t> xs(n), ys(n);
	std::vector<int> ids(n);
	for (int i = 0; i < n; ++i) xs[i] = points[i].x, ys[i] = points[i].y, ids[i] = points[i].id;
	const WaveletMatrix<std::int64_t> wm(ys);
	int q = 0;
	in.read(q);
	while (q--) {
		std::int64_t sx = 0, tx = 0, sy = 0, ty = 0;
		in.read(sx); in.read(tx); in.read(sy); in.read(ty);
		int l = int(std::lower_bound(xs.begin(), xs.end(), sx) - xs.begin());
		int r = int(std::upper_bound(xs.begin(), xs.end(), tx) - xs.begin());
		std::vector<int> answer;
		auto last = wm.prev_value(l, r, ty + 1);
		if (last && *last >= sy) {
			auto value = wm.next_value(l, r, sy);
			while (value) {
				int first = wm.rank(*value, l), end = wm.rank(*value, r);
				for (int k = first; k < end; ++k) answer.push_back(ids[wm.select(*value, k)]);
				if (*value == *last) break;
				value = wm.next_value(l, r, *value + 1);
			}
		}
		std::sort(answer.begin(), answer.end());
		for (int id : answer) out.println(id);
		out.write('\n');
	}
}
