#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_3_C"
#include "geometry/polygon.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	std::vector<geometry::IPoint> polygon(n);
	for (auto &p : polygon) input.read(p.x), input.read(p.y);
	int q = 0; input.read(q);
	while (q--) {
		geometry::IPoint p;
		input.read(p.x), input.read(p.y);
		output.println(static_cast<int>(geometry::contains(polygon, p)) + 1);
	}
}
