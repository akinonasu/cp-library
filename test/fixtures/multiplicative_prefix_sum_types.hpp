#ifndef MULTIPLICATIVE_PREFIX_SUM_TYPES_HPP
#define MULTIPLICATIVE_PREFIX_SUM_TYPES_HPP

#include <cstdint>

template<unsigned Mod> struct MultiplicativeSumModint {
	unsigned v = 0;
	MultiplicativeSumModint() = default;
	MultiplicativeSumModint(long long x) { auto r = x % Mod; v = r < 0 ? r + Mod : r; }
	unsigned val() const { return v; }
	MultiplicativeSumModint& operator+=(MultiplicativeSumModint b) { v += b.v; if (v >= Mod) v -= Mod; return *this; }
	MultiplicativeSumModint& operator-=(MultiplicativeSumModint b) { v = v >= b.v ? v - b.v : v + Mod - b.v; return *this; }
	MultiplicativeSumModint& operator*=(MultiplicativeSumModint b) { v = std::uint64_t(v) * b.v % Mod; return *this; }
	friend MultiplicativeSumModint operator+(MultiplicativeSumModint a, MultiplicativeSumModint b) { return a += b; }
	friend MultiplicativeSumModint operator-(MultiplicativeSumModint a, MultiplicativeSumModint b) { return a -= b; }
	friend MultiplicativeSumModint operator*(MultiplicativeSumModint a, MultiplicativeSumModint b) { return a *= b; }
	friend bool operator==(MultiplicativeSumModint a, MultiplicativeSumModint b) { return a.v == b.v; }
};

#endif
