#define PROBLEM "https://judge.yosupo.jp/problem/sort_points_by_argument"
#include "geometry/point.hpp"
#include "components/fast_io.hpp"
#include <algorithm>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	std::vector<geometry::IPoint> points(n);
	for (auto &p : points) input.read(p.x), input.read(p.y);
	std::sort(points.begin(), points.end(), geometry::PolarLess<std::int64_t>{});
	auto lower = std::find_if(points.begin(), points.end(), [](auto p) { return p.y < 0; });
	std::rotate(points.begin(), lower, points.end());
	for (auto p : points) output.write(p.x), output.write(' '), output.println(p.y);
}
