#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_A"
#include "geometry/exact_circle.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	geometry::IPoint a, b;
	std::int64_t first_radius = 0, second_radius = 0;
	std::cin >> a >> first_radius >> b >> second_radius;
	auto first = geometry::make_exact_circle(a, geometry::Rational(first_radius));
	auto second = geometry::make_exact_circle(b, geometry::Rational(second_radius));
	auto relation = geometry::relation(first, second);
	int answer = 0;
	if (relation == geometry::CircleRelation::separate) answer = 4;
	if (relation == geometry::CircleRelation::external_tangent) answer = 3;
	if (relation == geometry::CircleRelation::intersecting) answer = 2;
	if (relation == geometry::CircleRelation::internal_tangent) answer = 1;
	std::cout << answer << '\n';
}
