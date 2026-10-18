#include "fps/compose.hpp"
#include <cstdlib>

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (!fps_detail::NTT::vectorized()) return 0;
	using namespace fps_detail::composition_poly;
	const auto require = [](bool ok) { if (!ok) std::abort(); };
	for (std::size_t m : {256u, 512u, 1024u, 4096u, 2097152u}) {
		const int lg = std::countr_zero(m) + 2;
		const auto words = detail::Tables::words(m);
		Arena arena(Transform::words(lg) + Transform::words(lg - 1) + 3 * Arena::footprint(words));
		const Transform t(arena, lg), smaller(arena, lg - 1);
		auto own_memory = arena.take(words), shared_memory = arena.take(words), fallback_memory = arena.take(words);
		std::fill(shared_memory.begin(), shared_memory.end(), 0xfdfdfdfdu);
		const detail::Tables own(own_memory, m), shared(shared_memory.first(0), m, &t), fallback(fallback_memory, m, &smaller);
		require(own.roots != t.roots() && shared.roots == t.roots());
		require(shared.inverse_roots == t.inverse_roots());
		require(fallback.roots != smaller.roots() && fallback.inverse_roots != smaller.inverse_roots());
		const auto count = fps_detail::blocked_ntt::detail::table_words(lg);
		require(std::equal(own.roots, own.roots + count, shared.roots));
		require(std::equal(own.inverse_roots, own.inverse_roots + count, shared.inverse_roots));
		require(std::equal(own.roots, own.roots + count, fallback.roots));
		require(std::equal(own.inverse_roots, own.inverse_roots + count, fallback.inverse_roots));
		require(std::all_of(shared_memory.begin(), shared_memory.end(), [](auto x) { return x == 0xfdfdfdfdu; }));
	}
#endif
}
