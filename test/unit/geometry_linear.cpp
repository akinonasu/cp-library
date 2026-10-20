#include "geometry/linear.hpp"
#include <cassert>
#include <random>

using namespace geometry;
using L = Line<std::int64_t>;
using S = Segment<std::int64_t>;
bool close(Real a, Real b) { return std::abs(a - b) <= 1e-9L * (1 + std::abs(a) + std::abs(b)); }
S turned(S s) { return {rotate90(s.a), rotate90(s.b)}; }

int main() {
	L horizontal{IPoint{0, 0}, IPoint{2, 0}}, vertical{IPoint{1, -1}, IPoint{1, 1}};
	assert(orthogonal(horizontal, vertical) && !parallel(horizontal, vertical));
	assert(almost_equal(projection(horizontal, IPoint{3, 7}), FPoint{3, 0}));
	assert(almost_equal(reflection(horizontal, IPoint{3, 7}), FPoint{3, -7}));
	assert(almost_equal(*intersection(horizontal, vertical), FPoint{1, 0}));
	assert(!intersection(horizontal, horizontal));
	assert(distance(horizontal, IPoint{3, 7}) == 7);
	assert(distance(S{IPoint{0, 0}, IPoint{2, 0}}, IPoint{3, 4}) == std::sqrt(Real(17)));
	S a{IPoint{0, 0}, IPoint{4, 0}}, b{IPoint{2, 0}, IPoint{6, 0}};
	auto overlap = intersection(a, b);
	assert(overlap.size() == 2 && almost_equal(overlap[0], FPoint{2, 0}) && almost_equal(overlap[1], FPoint{4, 0}));
	S origin{IPoint{}, IPoint{}};
	assert(intersects(origin, a) && distance(origin, a) == 0);
	assert(intersection(origin, a).size() == 1);
	L point{IPoint{1, 2}, IPoint{1, 2}};
	assert(distance(point, horizontal) == 2 && distance(horizontal, point) == 2);
	assert(distance(point, a) == 2 && distance(a, point) == 2);
	assert(!on_line(point, IPoint{3, 2}) && on_line(point, point.a));
	assert(almost_equal(projection(point, IPoint{5, 6}), FPoint{1, 2}));
	assert(!perpendicular_bisector(IPoint{}, IPoint{}));
	auto bisector = *perpendicular_bisector(IPoint{0, 0}, IPoint{2, 0});
	assert(close(distance(bisector, FPoint{1, 100}), 0));
	constexpr std::int64_t big = 1000000000000000000LL;
	S wide{IPoint{-big, 0}, IPoint{big, 0}}, tall{IPoint{0, -big}, IPoint{0, big}};
	assert(intersects(wide, tall) && distance(wide, tall) == 0);
	assert(almost_equal(intersection(wide, tall)[0], FPoint{}));
	std::mt19937_64 random(1293);
	for (int i = 0; i < 30000; ++i) {
		auto p = [&] { return IPoint{std::int64_t(random() % 101) - 50, std::int64_t(random() % 101) - 50}; };
		S s{p(), p()}, t{p(), p()};
		bool hit = intersects(s, t);
		assert(hit == intersects(t, s) && hit == intersects(S{s.b, s.a}, t));
		assert(hit == intersects(turned(s), turned(t)));
		Real d = distance(s, t);
		assert(d >= 0 && close(d, distance(t, s)) && close(d, distance(turned(s), turned(t))));
		assert((d == 0) == hit);
		auto points = intersection(s, t);
		assert(points.empty() == !hit && points.size() <= 2);
		for (auto x : points) {
			assert(distance(Segment<Real>{to_real(s.a), to_real(s.b)}, x) < 1e-9L);
			assert(distance(Segment<Real>{to_real(t.a), to_real(t.b)}, x) < 1e-9L);
		}
		IPoint q = p();
		if (s.a != s.b) {
			auto x = projection(L{s.a, s.b}, q);
			auto v = to_real(s.b - s.a);
			assert(std::abs(dot(to_real(q) - x, v)) < 1e-8L);
			assert(close(distance(L{s.a, s.b}, q), distance(to_real(q), x)));
		}
	}
}
