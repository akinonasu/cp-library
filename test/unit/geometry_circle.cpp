#include "geometry/circle.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

using namespace geometry;

bool near(Real a, Real b, Real tolerance = 1e-9L) {
	return std::abs(a - b) <= tolerance * std::max({Real(1), std::abs(a), std::abs(b)});
}
bool near(FPoint a, FPoint b, Real tolerance = 1e-9L) { return near(a.x, b.x, tolerance) && near(a.y, b.y, tolerance); }

void primitive_cases() {
	Circle c{{0, 0}, 1};
	assert(near(area(c), pi) && near(circumference(c), 2 * pi));
	assert(contains(c, IPoint{0, 0}) == Location::inside);
	assert(contains(c, IPoint{1, 0}) == Location::boundary);
	assert(contains(c, IPoint{2, 0}) == Location::outside);
	assert(contains(Circle{{0, 0}, 0}, IPoint{0, 0}, 0) == Location::boundary);
	auto points = intersection(c, Line<std::int64_t>{{-2, 0}, {2, 0}});
	assert(points.size() == 2 && near(points[0], {-1, 0}) && near(points[1], {1, 0}));
	points = intersection(c, Line<Real>{{-2, 1}, {2, 1}});
	assert(points.size() == 1 && near(points[0], {0, 1}));
	assert(intersection(c, Line<Real>{{-2, 2}, {2, 2}}).empty());
	assert(intersection(c, Line<Real>{{0, 0}, {0, 0}}).empty());
	assert(intersection(c, Line<Real>{{1, 0}, {1, 0}}).size() == 1);
	assert(intersection(c, Segment<Real>{{0, 0}, {0.5L, 0}}).empty());
	points = intersection(c, Segment<Real>{{0, 0}, {2, 0}});
	assert(points.size() == 1 && near(points[0], {1, 0}));
	points = intersection(c, Segment<Real>{{2, 0}, {-2, 0}});
	assert(points.size() == 2 && near(points[0], {-1, 0}) && near(points[1], {1, 0}));
	assert(intersection(c, Segment<Real>{{1, 0}, {1, 0}}).size() == 1);
	assert(!intersection(c, c));
	auto coincident_points = intersection(Circle{{0, 0}, 0}, Circle{{0, 0}, 0});
	assert(coincident_points && coincident_points->size() == 1 && near((*coincident_points)[0], {0, 0}));
	auto tangent = intersection(c, Circle{{2, 0}, 1});
	assert(tangent && tangent->size() == 1 && near((*tangent)[0], {1, 0}));
	auto crossing = intersection(c, Circle{{1, 0}, 1});
	assert(crossing && crossing->size() == 2 && near((*crossing)[0], {0.5L, -std::sqrt(Real(3)) / 2}));
	assert(intersection(c, Circle{{3, 0}, 1})->empty());
	assert(intersection(c, Circle{{0, 0}, 0.5L})->empty());
	assert(!common_tangents(c, c));
	assert(!common_tangents(Circle{{0, 0}, 0}, Circle{{0, 0}, 0}));
	assert(common_tangents(Circle{{0, 0}, 0}, Circle{{2, 0}, 0})->size() == 1);
	assert(common_tangents(c, Circle{{0, 0}, 0})->empty());
	assert(common_tangents(c, Circle{{1, 0}, 0})->size() == 1);
	assert(common_tangents(c, Circle{{2, 0}, 0})->size() == 2);
	assert(tangent_points(c, FPoint{0, 0}).empty());
	assert(tangent_points(c, FPoint{1, 0}).size() == 1);
	assert(tangent_points(c, FPoint{2, 0}).size() == 2);
	assert(tangent_points(Circle{{0, 0}, 0}, FPoint{0, 0}).size() == 1);
	auto outer = circumcircle(IPoint{0, 0}, IPoint{4, 0}, IPoint{0, 3});
	auto inner = incircle(IPoint{0, 0}, IPoint{4, 0}, IPoint{0, 3});
	assert(outer && near(outer->center, {2, 1.5L}) && near(outer->radius, 2.5L));
	assert(inner && near(inner->center, {1, 1}) && near(inner->radius, 1));
	assert(!circumcircle(IPoint{0, 0}, IPoint{1, 0}, IPoint{2, 0}));
	assert(!incircle(IPoint{0, 0}, IPoint{1, 0}, IPoint{2, 0}));
	Circle small{{0, 0}, 1}, large{{1e12L, 0}, 1e12L};
	auto unbalanced = intersection(small, large, 0);
	assert(unbalanced && unbalanced->size() == 2 && near((*unbalanced)[0], {5e-13L, -1}, 1e-10L));
	assert(near(overlap_area(small, large), pi / 2, 1e-10L));
	Circle huge{{0, 0}, 1e12L};
	unbalanced = intersection(huge, Circle{{1, 0}, 1e12L}, 0);
	assert(unbalanced && unbalanced->size() == 2 && near((*unbalanced)[0].x, Real(0.5L)));
}

CircleRelation exact_relation(IPoint a, int ra, IPoint b, int rb) {
	auto d2 = distance_squared(a, b);
	if (!d2 && ra == rb) return CircleRelation::coincident;
	if (d2 > (ra + rb) * (ra + rb)) return CircleRelation::separate;
	if (d2 < (ra - rb) * (ra - rb)) return CircleRelation::contained;
	if (d2 == (ra + rb) * (ra + rb)) return CircleRelation::external_tangent;
	if (d2 == (ra - rb) * (ra - rb)) return CircleRelation::internal_tangent;
	return CircleRelation::intersecting;
}

void random_circle_cases() {
	std::mt19937 random(931752);
	for (int iteration = 0; iteration < 10000; ++iteration) {
		auto point = [&] { return IPoint{int(random() % 41) - 20, int(random() % 41) - 20}; };
		IPoint p = point(), q = point();
		int ra = random() % 21, rb = random() % 21;
		Circle a{to_real(p), Real(ra)}, b{to_real(q), Real(rb)};
		auto type = exact_relation(p, ra, q, rb);
		assert(relation(a, b, 0) == type && relation(b, a, 0) == type);
		auto crossings = intersection(a, b, 0);
		if (type == CircleRelation::coincident && ra) assert(!crossings);
		else {
			int count = type == CircleRelation::intersecting ? 2 : type == CircleRelation::external_tangent || type == CircleRelation::internal_tangent || type == CircleRelation::coincident ? 1 : 0;
			assert(crossings && int(crossings->size()) == count);
			for (auto v : *crossings) assert(near(distance(a.center, v), a.radius) && near(distance(b.center, v), b.radius));
			auto reverse = intersection(b, a, 0);
			assert(reverse && reverse->size() == crossings->size());
			for (int i = 0; i < count; ++i) assert(near((*reverse)[i], (*crossings)[i]));
		}
		auto lines = common_tangents(a, b, 0);
		if (type == CircleRelation::coincident) assert(!lines);
		else {
			int count = !ra && !rb ? 1 : !ra || !rb ? type == CircleRelation::separate ? 2 : type == CircleRelation::contained ? 0 : 1
				: type == CircleRelation::separate ? 4 : type == CircleRelation::external_tangent ? 3 : type == CircleRelation::intersecting ? 2 : type == CircleRelation::internal_tangent ? 1 : 0;
			assert(lines && int(lines->size()) == count);
			for (auto line : *lines) {
				assert(line.a != line.b);
				assert(near(distance(line, a.center), a.radius, 1e-8L));
				assert(near(distance(line, b.center), b.radius, 1e-8L));
			}
		}
		auto contacts = tangent_points(a, q, 0);
		for (auto contact : contacts) {
			assert(near(distance(contact, a.center), a.radius));
			assert(near(dot(contact - a.center, to_real(q) - contact), 0, 1e-8L));
		}
	}
}

void areas() {
	Circle c{{0, 0}, 1};
	assert(near(overlap_area(c, c), pi));
	assert(near(overlap_area(c, Circle{{1, 0}, 1}), 2 * pi / 3 - std::sqrt(Real(3)) / 2));
	assert(overlap_area(c, Circle{{2, 0}, 1}) == 0);
	assert(near(overlap_area(c, Circle{{0, 0}, 2}), pi));
	Real gap = 1e-8L, sliver = overlap_area(c, Circle{{2 - gap, 0}, 1});
	assert(sliver > 0 && near(sliver / (gap * std::sqrt(gap)), Real(4) / 3, 1e-7L));
	std::vector<FPoint> square{ {-2, -2}, {2, -2}, {2, 2}, {-2, 2} };
	assert(near(intersection_area(c, square), pi));
	std::reverse(square.begin(), square.end());
	assert(near(intersection_area(c, square), pi));
	std::vector<FPoint> half{ {0, -2}, {2, -2}, {2, 2}, {0, 2} };
	assert(near(intersection_area(c, half), pi / 2));
	std::vector<FPoint> quadrant{ {0, 0}, {2, 0}, {2, 2}, {0, 2} };
	assert(near(intersection_area(c, quadrant), pi / 4));
	std::vector<FPoint> inside{ {-0.5L, -0.5L}, {0.5L, -0.5L}, {0.5L, 0.5L}, {-0.5L, 0.5L} };
	assert(near(intersection_area(c, inside), 1));
	inside.push_back(inside[0]);
	assert(near(intersection_area(c, inside), 1));
	std::vector<FPoint> tangent_rectangle{ {-2, 1}, {2, 1}, {2, 2}, {-2, 2} };
	for (int i = 0; i < 4; ++i) {
		assert(near(intersection_area(c, tangent_rectangle), 0));
		for (auto &p : tangent_rectangle) p = rotate90(p);
	}
	for (auto &p : square) p += FPoint{5, 7};
	assert(near(intersection_area(Circle{{5, 7}, 1}, square), pi));
	assert(intersection_area(Circle{{0, 0}, 0}, square) == 0);
	assert(intersection_area(c, std::vector<FPoint>{}) == 0);
	assert(near(intersection_area(Circle{{0, 0}, 5}, std::vector<IPoint>{ {1, 1}, {4, 1}, {5, 5} }), 4.639858417607L));
	assert(near(intersection_area(Circle{{0, 0}, 5}, std::vector<IPoint>{ {0, 0}, {-3, -6}, {1, -3}, {5, -4} }), 11.787686807576L));
	std::mt19937 random(291721);
	std::vector<FPoint> polygon(512);
	for (int iteration = 0; iteration < 100; ++iteration) {
		Circle a{{Real(int(random() % 11) - 5), Real(int(random() % 11) - 5)}, Real(random() % 5 + 1)};
		Circle b{{Real(int(random() % 11) - 5), Real(int(random() % 11) - 5)}, Real(random() % 5 + 1)};
		for (int i = 0; i < int(polygon.size()); ++i) {
			Real angle = 2 * pi * i / polygon.size();
			polygon[i] = b.center + FPoint{std::cos(angle), std::sin(angle)} * b.radius;
		}
		Real lens = overlap_area(a, b), approximation = intersection_area(a, polygon);
		Real missing = b.radius * b.radius * (pi - polygon.size() * std::sin(2 * pi / polygon.size()) / 2);
		assert(approximation <= lens + 1e-8L && lens - approximation <= missing + 1e-8L);
		assert(near(lens, overlap_area(b, a)));
	}
}

Real brute_radius(const std::vector<IPoint> &points) {
	Real best = std::numeric_limits<Real>::infinity();
	auto consider = [&](Circle candidate) {
		for (auto p : points) if (distance(candidate.center, to_real(p)) > candidate.radius + 1e-9L) return;
		best = std::min(best, candidate.radius);
	};
	for (int i = 0; i < int(points.size()); ++i) {
		consider({to_real(points[i]), 0});
		for (int j = 0; j < i; ++j) {
			FPoint center = (to_real(points[i]) + to_real(points[j])) / Real(2);
			consider({center, distance(center, to_real(points[i]))});
			for (int k = 0; k < j; ++k) if (auto c = circumcircle(points[i], points[j], points[k], 0)) consider(*c);
		}
	}
	return best;
}

void enclosing() {
	assert(!minimum_enclosing_circle(std::vector<IPoint>{}));
	std::vector<IPoint> equal(100, IPoint{5, -7});
	auto result = minimum_enclosing_circle(equal);
	assert(result && result->center == FPoint(5, -7) && result->radius == 0);
	std::vector<IPoint> line;
	for (int i = -20; i <= 20; ++i) line.emplace_back(i, 3 * i + 5);
	result = minimum_enclosing_circle(line);
	assert(result && near(result->center, {0, 5}) && near(result->radius, std::sqrt(Real(4000))));
	std::mt19937 random(712936);
	for (int iteration = 0; iteration < 800; ++iteration) {
		int n = random() % 11 + 1;
		std::vector<IPoint> points(n);
		for (auto &p : points) p = {int(random() % 21) - 10, int(random() % 21) - 10};
		auto circle = minimum_enclosing_circle(points, iteration + 1);
		assert(circle && near(circle->radius, brute_radius(points), 1e-8L));
		for (auto p : points) assert(contains(*circle, p, 1e-9L) != Location::outside);
		auto other = minimum_enclosing_circle(points, iteration + 17);
		assert(other && near(circle->radius, other->radius, 1e-8L) && near(circle->center, other->center, 1e-8L));
	}
}

int main() {
	primitive_cases();
	random_circle_cases();
	areas();
	enclosing();
}
