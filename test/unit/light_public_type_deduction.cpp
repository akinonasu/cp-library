#include "fps/fps_light.hpp"
#include "convolution/online_convolution_light.hpp"
#include <cassert>
#include <type_traits>

int main() {
	FormalPowerSeries empty;
	FormalPowerSeries f(fps{1, 2, 1});
	static_assert(std::is_same_v<decltype(empty), fps>);
	static_assert(std::is_same_v<decltype(f), fps>);
	static_assert(std::is_same_v<decltype(f * f), fps>);
	assert(empty.empty() && f == fps({1, 2, 1}));
	OnlineConvolution product(3);
	static_assert(std::is_same_v<decltype(product), OnlineConvolution<mint>>);
	assert(product.push(1, 4) == mint(4));
	assert(product.push(2, 5) == mint(13));
	assert(product.push(3, 6) == mint(28));
}
