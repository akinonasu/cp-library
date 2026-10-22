#include "components/fps_light_ntt.hpp"
#include <cstdio>

int main() {
	const std::size_t n = std::size_t(1) << 23;
	std::vector<mint> a(n);
	for (std::size_t i = 0; i < n; ++i) a[i] = mint::raw((i * 735187U + 13) % mint::mod());
	fps_light_detail::transform(a);
	fps_light_detail::transform(a, true);
	for (std::size_t i = 0; i < n; ++i)
		if (a[i].val() != (i * 735187U + 13) % mint::mod()) return 1;
	std::puts("2^23 full roundtrip PASS");
}
