#include "components/fps_output.hpp"
#include <cstdlib>
#include <new>
static std::size_t fail_bytes = 0, failures = 0;
void *operator new(std::size_t n) {
	if (fail_bytes && n == fail_bytes) { fail_bytes = 0; ++failures; throw std::bad_alloc(); }
	if (void *p = std::malloc(n ? n : 1)) return p;
	throw std::bad_alloc();
}
void operator delete(void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
static void require(bool value) { if (!value) std::abort(); }
int main() {
	constexpr std::size_t extra = 2u * 1024 * 1024 / sizeof(mint), n = 1u << 19;
	for (bool fail : {false, true}) {
		std::vector<mint> out = {mint::word(0), mint::word(1234), mint::word(2 * mint::mod() - 1)};
		const auto prefix = out;
		if (fail) fail_bytes = (n + extra) * sizeof(mint);
		fps_detail::prepare_fps_input_output(out, n);
		require(out.size() == prefix.size() && out.capacity() >= n);
		if (fail) require(failures == 1);
		else require(out.capacity() >= n + extra);
		for (std::size_t i = 0; i < prefix.size(); ++i) require(out[i].v == prefix[i].v);
		bool length = false;
		try { fps_detail::prepare_fps_input_output(out, out.max_size() + 1); }
		catch (const std::length_error &) { length = true; }
		require(length && out.size() == prefix.size());
		for (std::size_t i = 0; i < prefix.size(); ++i) require(out[i].v == prefix[i].v);
	}
}
