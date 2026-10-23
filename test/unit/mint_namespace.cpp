namespace fps_detail {
struct Mint {};
}

#include "math/mint.hpp"
#include <cassert>
#include <sstream>
#include <type_traits>

#ifdef CP_FPS_BACKEND_HPP
#error "mint must not include FPS"
#endif

static_assert(std::is_same_v<mint, mint_detail::Mint>);
static_assert(std::is_same_v<mint, fps_mint>);
static_assert(!std::is_same_v<mint, fps_detail::Mint>);

#include "fps/fps.hpp"

int main() {
	std::stringstream stream("-1 998244354");
	mint a, b;
	stream >> a >> b;
	assert(a == mint(-1) && b == mint(1));
	stream.str("");
	stream.clear();
	stream << a + b << ' ' << a * b;
	assert(stream.str() == "0 998244352");
	fps f{1, 2};
	assert(f * f == fps({1, 4, 4}));
}
