#define PROBLEM "https://judge.yosupo.jp/problem/minimum_enclosing_circle"
#include "geometry/exact_circle.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<geometry::IPoint> points(n);
	for (auto& point : points) input.read(point.x), input.read(point.y);
	auto circle = *geometry::minimum_enclosing_circle_exact(points);
	for (auto point : points)
		output.write(geometry::contains(circle, point) == geometry::Location::boundary ? '1' : '0');
	output.write('\n');
}
