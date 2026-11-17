#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_A"
#include "geometry/circle.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	geometry::Circle a, b;
	std::cin >> a.center >> a.radius >> b.center >> b.radius;
	auto relation = geometry::relation(a, b);
	int answer = 0;
	if (relation == geometry::CircleRelation::separate) answer = 4;
	if (relation == geometry::CircleRelation::external_tangent) answer = 3;
	if (relation == geometry::CircleRelation::intersecting) answer = 2;
	if (relation == geometry::CircleRelation::internal_tangent) answer = 1;
	std::cout << answer << '\n';
}
