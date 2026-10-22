#include "fps/compose_light.hpp"
#include "fps/compositional_inverse_light.hpp"
#include "fps/power_projection_light.hpp"
#include <random>
#ifdef TEST_LIGHT_ACL
#include <atcoder/modint>
#endif

namespace A = light_fps_algorithms;

template<class T>
LightFormalPowerSeries<T> naive_compose(const LightFormalPowerSeries<T> &f, const LightFormalPowerSeries<T> &g, int n) {
	using F = LightFormalPowerSeries<T>;
	F result(n), power(n);
	if (n) power[0] = 1;
	for (int k = 0; k < std::min(n, int(f.size())); ++k) {
		for (int i = 0; i < n; ++i) result[i] += f[k] * power[i];
		F next(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < std::min(n - i, int(g.size())); ++j)
			next[i + j] += power[i] * g[j];
		power = std::move(next);
	}
	return result;
}

template<class T>
void check() {
	using F = LightFormalPowerSeries<T>;
	std::mt19937 random(397222);
	for (int n : {0, 1, 2, 3, 4, 5, 7, 8, 15, 16, 17, 31, 32, 33, 65}) {
		for (int trial = 0; trial < 8; ++trial) {
			int size = trial == 0 ? 0 : trial == 1 ? 1 : n + trial % 3;
			F f(size), g(size), w(n);
			for (auto *v : {&f, &g, &w}) for (auto &x : *v) x = random();
			if (size) g[0] = 0;
			if (trial == 2) for (int i = 1; i < size; i += 2) g[i] = 0;
			assert(A::compose(f, g, n) == naive_compose(f, g, n));
			assert(A::compose(f, g) == naive_compose(f, g, size));
			if (n) {
				F power(n), expected(n + 3);
				power[0] = 1;
				for (int i = 0; i < n + 3; ++i) {
					for (int j = 0; j < n; ++j) expected[i] += power[j] * w[j];
					power = F::multiply(power, f).pre(n);
					power.resize(n);
				}
				assert(A::power_projection(f, w, n + 3) == expected);
				assert(A::power_projection(f, w, 1) == F{w[0]});
				assert(A::power_projection(f, w, 0).empty());
			}
		}
	}
	assert(A::compose(F{}, F{}, 0).empty());
	assert(A::compositional_inverse(F{}, 0).empty());
	assert(A::compositional_inverse(F{0}, 1) == F{0});
	for (int n : {2, 3, 7, 16, 33, 65, 129, 257, 513}) {
		F f(n), g(n), w(n);
		for (auto *v : {&f, &g, &w}) for (auto &x : *v) x = random();
		g[0] = 0;
		auto composed = A::compose(f, g), projected = A::power_projection(g, w);
		T left = 0, right = 0;
		for (int i = 0; i < n; ++i) left += f[i] * projected[i], right += w[i] * composed[i];
		assert(left == right);
		g[1] = 7;
		auto inverse = A::compositional_inverse(g);
		F identity(n); identity[1] = 1;
		assert(A::compose(g, inverse) == identity);
		assert(A::compose(inverse, g) == identity);
		assert(A::compositional_inverse(F{0, 7}, n) == identity / T(7));
		F coefficient(n); coefficient.back() = 1;
		assert(A::power_projection(g, n - 1) == A::power_projection(g, coefficient));
	}
}

int main() {
	check<mint>();
#ifdef TEST_LIGHT_ACL
	check<atcoder::modint998244353>();
#endif
}
