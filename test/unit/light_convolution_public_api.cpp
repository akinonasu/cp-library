#include "convolution/convolution_light.hpp"
#include "convolution/cyclic_convolution_light.hpp"
#include "convolution/multiplicative_convolution_light.hpp"
#include "convolution/ordered_convolution_light.hpp"
#include "convolution/max_min_convolution_light.hpp"
#include "convolution/multivariate_convolution_light.hpp"
#include "convolution/online_convolution_light.hpp"
#include <cassert>
#include <type_traits>

static_assert(&convolution<mint> == &light_convolution::convolution<mint>);
static_assert(&cyclic_convolution<false, mint> == &light_convolution::cyclic_convolution<false, mint>);
static_assert(&multiplicative_convolution<mint> == &light_convolution::multiplicative_convolution<mint>);
static_assert(&ordered_convolution<mint> == &light_convolution::ordered_convolution<mint>);
static_assert(&multivariate_convolution<mint> == &light_convolution::multivariate_convolution<mint>);
static_assert(&max_min_convolution<int> == &light_convolution::max_min_convolution<int>);
static_assert(&min_max_convolution<int> == &light_convolution::min_max_convolution<int>);
static_assert(std::is_base_of_v<LightOnlineConvolution<>, OnlineConvolution<>>);

int main() {
	const std::vector<mint> a{1, 2, 3}, b{4, 5, 6};
	assert(convolution(a, b) == std::vector<mint>({4, 13, 28, 27, 18}));
	assert(cyclic_convolution(a, b, 3) == std::vector<mint>({31, 31, 28}));
	assert(cyclic_convolution<true>(a, b, 3) == std::vector<mint>({-23, -5, 28}));
	assert(ordered_convolution(a, b) == std::vector<mint>({0, 5, 6, 12, 0}));
	assert(multiplicative_convolution(a, b) == std::vector<mint>({35, 28, 27}));
	assert(multivariate_convolution(a, b, {3}) == std::vector<mint>({4, 13, 28}));
	assert(max_min_convolution(std::vector<int>{1, 4}, std::vector<int>{3, 2}) == std::vector<int>({1, 3, 2}));
	assert(min_max_convolution(std::vector<int>{1, 4}, std::vector<int>{3, 2}) == std::vector<int>({3, 2, 4}));
	OnlineConvolution online(3);
	assert(online.push(1, 4) == mint(4));
	assert(online.push(2, 5) == mint(13));
	assert(online.push(3, 6) == mint(28));
	assert(online.size() == 3 && online[2] == mint(28));
}
