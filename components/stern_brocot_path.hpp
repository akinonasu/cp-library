#ifndef STERN_BROCOT_PATH_HPP
#define STERN_BROCOT_PATH_HPP

#include <cassert>
#include <concepts>

namespace stern_brocot_detail {
template <std::unsigned_integral U>
class Path {
	U a, b;
public:
	Path(U numerator, U denominator) : a(numerator), b(denominator) { assert(a && b); }
	bool empty() const { return a == b; }
	bool right() const { return a > b; }
	U pop() {
		assert(!empty());
		if (right()) { U k = (a - 1) / b; a -= k * b; return k; }
		U k = (b - 1) / a; b -= k * a; return k;
	}
};
}

#endif
