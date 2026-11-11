#ifndef GEOMETRY_RATIONAL_HPP
#define GEOMETRY_RATIONAL_HPP

#include "point.hpp"
#include <boost/multiprecision/cpp_int.hpp>

namespace geometry {
using Integer = boost::multiprecision::cpp_int;
using Rational = boost::multiprecision::cpp_rational;

namespace point_detail {
template<> struct Scalar<Integer> {
	static constexpr bool valid = true, exact = true;
	using Wide = Integer;
};
template<> struct Scalar<Rational> {
	static constexpr bool valid = true, exact = true;
	using Wide = Rational;
};
}

using BigPoint = Point<Integer>;
using RPoint = Point<Rational>;

template<class T>
concept ExactCoordinate = point_detail::Scalar<T>::valid && point_detail::Scalar<T>::exact;

template<ExactCoordinate T>
RPoint to_rational(const Point<T> &p) { return {Rational(p.x), Rational(p.y)}; }
}

#endif
