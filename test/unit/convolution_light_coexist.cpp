#if LIGHT_FIRST
#include "convolution/convolution_light.hpp"
#include "convolution/cyclic_convolution_light.hpp"
#include "convolution/multiplicative_convolution_light.hpp"
#include "convolution/multivariate_convolution_light.hpp"
#include "convolution/ordered_convolution_light.hpp"
#include "convolution/online_convolution_light.hpp"
#include "convolution/max_min_convolution_light.hpp"
#endif
#include "convolution/convolution.hpp"
#include "convolution/cyclic_convolution.hpp"
#include "convolution/multiplicative_convolution.hpp"
#include "convolution/multivariate_convolution.hpp"
#include "convolution/ordered_convolution.hpp"
#include "convolution/online_convolution.hpp"
#include "convolution/max_min_convolution.hpp"
#if !LIGHT_FIRST
#include "convolution/convolution_light.hpp"
#include "convolution/cyclic_convolution_light.hpp"
#include "convolution/multiplicative_convolution_light.hpp"
#include "convolution/multivariate_convolution_light.hpp"
#include "convolution/ordered_convolution_light.hpp"
#include "convolution/online_convolution_light.hpp"
#include "convolution/max_min_convolution_light.hpp"
#endif
#include <cassert>

struct Ring {
	long long value = 0;
	Ring(long long x = 0) : value(x) {}
	Ring &operator+=(Ring x) { value += x.value; return *this; }
	Ring &operator-=(Ring x) { value -= x.value; return *this; }
	friend Ring operator*(Ring a, Ring b) { return a.value * b.value; }
	friend bool operator==(Ring, Ring) = default;
};
struct OtherMint {
	unsigned value = 0;
	OtherMint(unsigned x = 0) : value(x % mod()) {}
	static constexpr unsigned mod() { return 998244353; }
	static OtherMint raw(unsigned x) { return OtherMint(x); }
	unsigned val() const { return value; }
	friend bool operator==(OtherMint, OtherMint) = default;
};

int main() {
	std::vector<mint> a(97, mint(2)), b(97, mint(3));
	assert(convolution(a, b) == light_convolution::convolution(a, b));
	assert(cyclic_convolution(a, b, 19) == light_convolution::cyclic_convolution(a, b, 19));
	assert(multiplicative_convolution(a, b) == light_convolution::multiplicative_convolution(a, b));
	assert(ordered_convolution(a, b) == light_convolution::ordered_convolution(a, b));
	assert(multivariate_convolution(a, b, {97}) == light_convolution::multivariate_convolution(a, b, {97}));
	std::vector<Ring> x(97, Ring(2)), y(97, Ring(3));
	assert(convolution(x, y) == light_convolution::convolution(x, y));
	assert(cyclic_convolution(x, y, 19) == light_convolution::cyclic_convolution(x, y, 19));
	assert(multiplicative_convolution(x, y) == light_convolution::multiplicative_convolution(x, y));
	assert(ordered_convolution(x, y) == light_convolution::ordered_convolution(x, y));
	assert(multivariate_convolution(x, y, {97}) == light_convolution::multivariate_convolution(x, y, {97}));
	std::vector<OtherMint> p(127, OtherMint(2)), q(113, OtherMint(3));
	assert(convolution(p, q) == light_convolution::convolution(p, q));
	assert(convolution(p, p) == light_convolution::convolution(p, p));
	OnlineConvolution<> fast;
	LightOnlineConvolution<> light;
	for (int i = 0; i < 513; ++i) assert(fast.push(i + 1, i + 2) == light.push(i + 1, i + 2));
}
