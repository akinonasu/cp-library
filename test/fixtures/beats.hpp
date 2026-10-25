#ifndef TEST_BEATS_HPP
#define TEST_BEATS_HPP

#include "../../data_structure/segment_tree/segtree_beats.hpp"
#include <cstdint>

namespace division_beats {
struct S { long long sum, lo, hi; int len; };
struct F { long long value; unsigned long long divisor; };
inline S e() { return {0, 0, 0, 0}; }
inline S leaf(long long value) { return {value, value, value, 1}; }
inline S op(const S &a, const S &b) {
	if (!a.len) return b;
	if (!b.len) return a;
	return {a.sum + b.sum, std::min(a.lo, b.lo), std::max(a.hi, b.hi), a.len + b.len};
}
inline F id() { return {-1, 1}; }
inline F composition(const F &f, const F &g) {
	if (f.value >= 0) return f;
	if (g.value >= 0) return {static_cast<long long>(g.value / f.divisor), 1};
	__uint128_t d = __uint128_t(f.divisor) * g.divisor;
	return {-1, d > ~0ULL ? ~0ULL : static_cast<unsigned long long>(d)};
}
inline bool mapping(const F &f, S &a) {
	if (!a.len) return true;
	if (f.value < 0 && f.divisor == 1) return true;
	long long lo = f.value < 0 ? a.lo / f.divisor : f.value;
	long long hi = f.value < 0 ? a.hi / f.divisor : f.value;
	if (lo != hi) return false;
	a.sum = lo * a.len;
	a.lo = a.hi = lo;
	return true;
}
using Tree = SegtreeBeats<S, op, e, F, mapping, composition, id>;
}

namespace bitwise_beats {
struct S { unsigned long long sum; unsigned lo, hi, bit_and, bit_or; int len; };
struct F { unsigned bit_and, bit_or; };
inline S e() { return {0, ~0U, 0, ~0U, 0, 0}; }
inline S leaf(unsigned value) { return {value, value, value, value, value, 1}; }
inline S op(const S &a, const S &b) {
	return {a.sum + b.sum, std::min(a.lo, b.lo), std::max(a.hi, b.hi),
		a.bit_and & b.bit_and, a.bit_or | b.bit_or, a.len + b.len};
}
inline F id() { return {~0U, 0}; }
inline F composition(const F &f, const F &g) {
	return {g.bit_and & f.bit_and, (g.bit_or & f.bit_and) | f.bit_or};
}
inline bool mapping(const F &f, S &a) {
	if (!a.len) return true;
	if ((a.bit_and ^ a.bit_or) & (~f.bit_and | f.bit_or)) return false;
	a.sum -= static_cast<unsigned long long>(a.bit_and) * a.len;
	a.bit_and = (a.bit_and & f.bit_and) | f.bit_or;
	a.sum += static_cast<unsigned long long>(a.bit_and) * a.len;
	a.bit_or = (a.bit_or & f.bit_and) | f.bit_or;
	a.lo = (a.lo & f.bit_and) | f.bit_or;
	a.hi = (a.hi & f.bit_and) | f.bit_or;
	return true;
}
using Tree = SegtreeBeats<S, op, e, F, mapping, composition, id>;
}

namespace clamp_beats {
inline constexpr long long inf = 1LL << 60;
struct S {
	long long sum, lo, hi, lo2, hi2;
	int nlo, nhi, len;
};
struct F { long long add, lo, hi; };
inline S e() { return {0, inf, -inf, inf, -inf, 0, 0, 0}; }
inline S leaf(long long value) { return {value, value, value, inf, -inf, 1, 1, 1}; }
inline S op(const S &a, const S &b) {
	if (!a.len) return b;
	if (!b.len) return a;
	S c;
	c.sum = a.sum + b.sum;
	c.len = a.len + b.len;
	if (a.lo < b.lo) c.lo = a.lo, c.lo2 = std::min(a.lo2, b.lo), c.nlo = a.nlo;
	else if (a.lo > b.lo) c.lo = b.lo, c.lo2 = std::min(a.lo, b.lo2), c.nlo = b.nlo;
	else c.lo = a.lo, c.lo2 = std::min(a.lo2, b.lo2), c.nlo = a.nlo + b.nlo;
	if (a.hi > b.hi) c.hi = a.hi, c.hi2 = std::max(a.hi2, b.hi), c.nhi = a.nhi;
	else if (a.hi < b.hi) c.hi = b.hi, c.hi2 = std::max(a.hi, b.hi2), c.nhi = b.nhi;
	else c.hi = a.hi, c.hi2 = std::max(a.hi2, b.hi2), c.nhi = a.nhi + b.nhi;
	return c;
}
inline F id() { return {0, -inf, inf}; }
inline F composition(const F &f, const F &g) {
	auto shift = [&](long long x) { return x == -inf || x == inf ? x : x + f.add; };
	return {g.add + f.add, std::clamp(shift(g.lo), f.lo, f.hi), std::clamp(shift(g.hi), f.lo, f.hi)};
}
inline bool mapping(const F &f, S &a) {
	if (!a.len) return true;
	long long lo = std::clamp(a.lo + f.add, f.lo, f.hi), hi = std::clamp(a.hi + f.add, f.lo, f.hi);
	if (lo == hi) {
		a = {lo * a.len, lo, lo, inf, -inf, a.len, a.len, a.len};
		return true;
	}
	if ((f.lo > a.lo + f.add && f.lo >= a.lo2 + f.add) ||
		(f.hi < a.hi + f.add && f.hi <= a.hi2 + f.add)) return false;
	a.sum += f.add * a.len;
	a.lo += f.add, a.hi += f.add;
	if (a.lo2 != inf) a.lo2 += f.add;
	if (a.hi2 != -inf) a.hi2 += f.add;
	if (a.hi > f.hi) {
		a.sum += (f.hi - a.hi) * a.nhi;
		if (a.lo2 == a.hi) a.lo2 = f.hi;
		a.hi = f.hi;
	}
	if (a.lo < f.lo) {
		a.sum += (f.lo - a.lo) * a.nlo;
		if (a.hi2 == a.lo) a.hi2 = f.lo;
		a.lo = f.lo;
	}
	return true;
}
using Tree = SegtreeBeats<S, op, e, F, mapping, composition, id>;
}

#endif
