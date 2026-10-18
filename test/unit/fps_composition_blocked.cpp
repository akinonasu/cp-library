#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/power_projection.hpp"
#include <cstdlib>
#include <random>

void require(bool ok) {
	if (!ok) std::abort();
}

int main() {
	std::mt19937 random(20261019);
	for (int n : {33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 511, 512, 513, 1025, 4097}) {
		require(power_projection(fps{}, n - 1) == fps(n));
		for (int size : {3, 9, n - 1, n}) {
			fps f(n), g(size), weights(n), identity(n);
			for (auto &x : f) x = random();
			for (auto &x : g) x = random();
			for (auto &x : weights) x = random();
			g[0] = 0;
			if (g[1] == mint(0)) g[1] = 1;
			const auto before_f = f, before_g = g, before_weights = weights;
			auto h = compose(f, g, n), projection = power_projection(g, weights, n);
			mint lhs = 0, rhs = 0;
			for (int i = 0; i < n; ++i) lhs += f[i] * projection[i], rhs += weights[i] * h[i];
			require(lhs == rhs);
			require(f == before_f && g == before_g && weights == before_weights);
			if (size == 3) {
				auto inverse = compositional_inverse(g, n);
				identity[1] = 1;
				require(compose(g, inverse, n) == identity);
				require(compose(inverse, g, n) == identity);
			}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
			if (fps_detail::NTT::vectorized()) {
				using namespace fps_detail::composition_poly;
				const int lg = std::max(compose_log(n), projection_log(n) + 1);
				const std::size_t total = std::max(compose_scratch(n), projection_scratch(n) + Arena::footprint(6 * detail::projection_length(n)));
				Arena arena(Transform::words(lg) + 3 * Arena::footprint(n) + Arena::footprint(total));
				const Transform t(arena, lg);
				auto a = arena.take(n), b = arena.take(n), c = arena.take(n), scratch = arena.take(total);
				for (int i = 0; i < n; ++i) a[i] = f[i].val(), b[i] = i < size ? g[i].val() : 0;
				std::fill(c.begin(), c.end(), 0xfdfdfdfdu);
				std::fill(scratch.begin(), scratch.end(), 0xfdfdfdfdu);
				fps_detail::composition_poly::compose(t, a, b, c, scratch);
				for (int i = 0; i < n; ++i) require(c[i] == h[i].val());
				auto unit = power_projection(g, n - 1);
				std::fill(scratch.begin(), scratch.end(), 0xfdfdfdfdu);
				fps_detail::composition_poly::power_projection(t, b, c, scratch.first(projection_scratch(n)));
				for (int i = 0; i < n; ++i) require(c[i] == unit[i].val());
				for (int i = 0; i < n; ++i) a[i] = weights[i].val();
				std::fill(scratch.begin(), scratch.end(), 0xfdfdfdfdu);
				fps_detail::composition_poly::weighted_projection(t, b, a, c, scratch);
				for (int i = 0; i < n; ++i) require(c[i] == projection[i].val());
			}
#endif
		}
	}
}
