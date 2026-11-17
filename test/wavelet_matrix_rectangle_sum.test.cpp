#define PROBLEM "https://judge.yosupo.jp/problem/rectangle_sum"
#include "../data_structure/wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <tuple>
#include <vector>

struct Value {
	int y, id;
	std::int64_t weight;
	explicit operator std::int64_t() const { return weight; }
	bool operator<(const Value &other) const { return std::tie(y, id, weight) < std::tie(other.y, other.id, other.weight); }
	bool operator==(const Value &) const = default;
};

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	struct Point { int x; Value value; };
	std::vector<Point> points(n);
	for (int i = 0; i < n; ++i) {
		in.read(points[i].x); in.read(points[i].value.y); in.read(points[i].value.weight);
		points[i].value.id = i;
	}
	std::sort(points.begin(), points.end(), [](const Point &a, const Point &b) { return a.x < b.x; });
	std::vector<int> xs(n);
	std::vector<Value> values(n);
	for (int i = 0; i < n; ++i) xs[i] = points[i].x, values[i] = points[i].value;
	const WaveletMatrix<Value, std::int64_t> wm(values, true);
	while (q--) {
		int l = 0, d = 0, r = 0, u = 0;
		in.read(l); in.read(d); in.read(r); in.read(u);
		int first = int(std::lower_bound(xs.begin(), xs.end(), l) - xs.begin());
		int last = int(std::lower_bound(xs.begin(), xs.end(), r) - xs.begin());
		out.println(wm.range_sum(first, last, Value{d, -1, 0}, Value{u, -1, 0}));
	}
}
