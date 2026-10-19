#include "components/fps_composition.hpp"
#include <cstdlib>
#include <random>

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	using namespace fps_detail::composition_poly;
	if (!__builtin_cpu_supports("avx2")) return 0;
	std::mt19937 random(292831);
	for (std::size_t h : {4u, 8u, 32u, 256u, 1024u, 2048u, 4096u, 8192u}) {
		Arena arena(2 * Arena::footprint(32 * h) + Transform::words(15));
		const Transform t(arena, 15);
		auto a = arena.take(32 * h), b = arena.take(32 * h);
		for (std::size_t width = 1; width < h; width *= 2) {
			for (int inverse : {0, 1}) {
				const std::uint32_t bound = (inverse ? 2u : 4u) * kModulus;
				for (std::size_t i = 0; i < a.size(); ++i) a[i] = i % 3 == 0 ? bound - 1 : i % 3 == 1 ? 0 : random() % bound;
				std::copy(a.begin(), a.end(), b.begin());
				const auto* roots = inverse ? t.inverse_roots() : t.roots();
				auto* v = reinterpret_cast<__m256i*>(a.data());
				if (inverse) {
					if (width == 1) kernels::inverse_even_columns(v, h, roots, roots);
					else kernels::inverse_columns(v, h, width, roots, roots);
					detail::inverse_zero_columns(b.data(), h, width, roots);
				} else {
					if (width == 1) kernels::forward_even_columns(v, h, roots, roots);
					else kernels::forward_columns(v, h, width, roots, roots);
					detail::forward_zero_columns(b.data(), h, width, roots);
				}
				for (std::size_t i = 0; i < a.size(); ++i) {
					if (b[i] >= bound || a[i] % kModulus != b[i] % kModulus) std::abort();
					if ((i % (8 * h)) / (8 * width) % 2 && a[i] != b[i]) std::abort();
				}
			}
		}
	}
#endif
}
