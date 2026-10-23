#ifndef TEST_RECURRENCE_TYPES_HPP
#define TEST_RECURRENCE_TYPES_HPP

#include <cassert>
#include <cstdint>

template <std::uint32_t P>
struct RecurrenceModint {
	std::uint32_t x = 0;
	RecurrenceModint() = default;
	RecurrenceModint(long long v) : x((v % P + P) % P) {}
	static constexpr std::uint32_t mod() { return P; }
	static RecurrenceModint raw(std::uint32_t v) { RecurrenceModint a; a.x = v; return a; }
	std::uint32_t val() const { return x; }
	RecurrenceModint operator-() const { return raw(x ? P - x : 0); }
	RecurrenceModint &operator+=(RecurrenceModint b) { x += b.x; if (x >= P) x -= P; return *this; }
	RecurrenceModint &operator-=(RecurrenceModint b) { x = x >= b.x ? x - b.x : x + P - b.x; return *this; }
	RecurrenceModint &operator*=(RecurrenceModint b) { x = std::uint64_t(x) * b.x % P; return *this; }
	RecurrenceModint &operator/=(RecurrenceModint b) {
		assert(b.x);
		RecurrenceModint p = 1;
		for (auto k = P - 2; k; k >>= 1, b *= b) if (k & 1) p *= b;
		return *this *= p;
	}
	friend RecurrenceModint operator+(RecurrenceModint a, RecurrenceModint b) { return a += b; }
	friend RecurrenceModint operator-(RecurrenceModint a, RecurrenceModint b) { return a -= b; }
	friend RecurrenceModint operator*(RecurrenceModint a, RecurrenceModint b) { return a *= b; }
	friend RecurrenceModint operator/(RecurrenceModint a, RecurrenceModint b) { return a /= b; }
	bool operator==(const RecurrenceModint &) const = default;
};

template <class T>
struct RecurrenceGeneric {
	T value;
	RecurrenceGeneric(long long x = 0) : value(x) {}
	RecurrenceGeneric(T x) : value(x) {}
	RecurrenceGeneric operator-() const { return RecurrenceGeneric(-value); }
	RecurrenceGeneric &operator+=(RecurrenceGeneric b) { value += b.value; return *this; }
	RecurrenceGeneric &operator*=(RecurrenceGeneric b) { value *= b.value; return *this; }
	friend RecurrenceGeneric operator+(RecurrenceGeneric a, RecurrenceGeneric b) { return a += b; }
	friend RecurrenceGeneric operator*(RecurrenceGeneric a, RecurrenceGeneric b) { return a *= b; }
	friend RecurrenceGeneric operator/(RecurrenceGeneric a, RecurrenceGeneric b) { return RecurrenceGeneric(a.value / b.value); }
	bool operator==(const RecurrenceGeneric &) const = default;
};

#endif
