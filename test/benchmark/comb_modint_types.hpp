#ifndef COMB_MODINT_TYPES_HPP
#define COMB_MODINT_TYPES_HPP

#include <cassert>
#include <cstdint>

namespace comb_test {
template<unsigned P, int Id = 0> struct Field {
	unsigned value = 0;
	inline static int inversions = 0;
	static constexpr unsigned mod() { return P; }
	Field() = default;
	Field(long long x) {
		long long r = x % P;
		value = r < 0 ? r + P : r;
	}
	unsigned val() const { return value; }
	Field operator*(Field other) const { return Field(std::uint64_t(value) * other.value % P); }
	Field inv() {
		assert(value);
		++inversions;
		Field a = *this, result(1);
		for (unsigned e = P - 2; e; e >>= 1, a = a * a) if (e & 1) result = result * a;
		return result;
	}
};

template<int Id = 0> struct DynamicField {
	unsigned value = 0;
	inline static unsigned modulus = 998244353;
	inline static int inversions = 0;
	static unsigned mod() { return modulus; }
	static void set_mod(unsigned p) { modulus = p; }
	DynamicField() = default;
	DynamicField(long long x) {
		long long r = x % mod();
		value = r < 0 ? r + mod() : r;
	}
	unsigned val() const { return value; }
	DynamicField operator*(DynamicField other) const { return DynamicField(std::uint64_t(value) * other.value % mod()); }
	DynamicField inv() {
		assert(value);
		++inversions;
		DynamicField a = *this, result(1);
		for (unsigned e = mod() - 2; e; e >>= 1, a = a * a) if (e & 1) result = result * a;
		return result;
	}
};
}

#endif
