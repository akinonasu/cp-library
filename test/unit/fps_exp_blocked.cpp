#include "fps/fps.hpp"
#include "components/fps_exp_blocked.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>

[[maybe_unused]] static void require(bool value, int line = __builtin_LINE()) {
	if (!value) {
		std::fprintf(stderr, "line %d\n", line);
		std::abort();
	}
}

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (!fps_detail::NTT::vectorized()) return 0;
	std::mt19937 rng(20261019);
	for (int n = 0; n <= 257; ++n) {
		auto words = std::make_unique<unsigned[]>(n);
		for (int mode = 0; mode < 2; ++mode) {
			for (int i = 0; i < n; ++i) words[i] = (mode ? 2 : 1) * fps_mint::mod() - 1 - rng() % 1000;
			std::vector<fps_mint> out{5, 7};
			if (mode) fps_detail::append_fps_words<true>(out, n ? words.get() : nullptr, n);
			else fps_detail::append_fps_words<false>(out, n ? words.get() : nullptr, n);
			require(out.size() == unsigned(n + 2) && out[0] == fps_mint(5) && out[1] == fps_mint(7));
			for (int i = 0; i < n; ++i) require(out[i + 2] == (mode ? fps_mint::word(words[i]) : fps_mint::raw(words[i])));
		}
	}
	for (int n : {131071, 131072, 131073, 500000, 1048575}) {
		auto words = std::make_unique<unsigned[]>(n);
		for (int mode = 0; mode < 2; ++mode) {
			for (int i = 0; i < n; ++i) words[i] = (mode ? 2 : 1) * fps_mint::mod() - 1 - rng() % 1000;
			std::vector<fps_mint> out{5, 7};
			const auto first = out[0].v, second = out[1].v;
			fps_detail::prepare_fps_output(out, n + 2);
			require(out.size() == 2 && out[0].v == first && out[1].v == second);
			if (mode) fps_detail::append_fps_words<true>(out, words.get(), n);
			else fps_detail::append_fps_words<false>(out, words.get(), n);
			require(out.size() == unsigned(n + 2) && out[0].v == first && out[1].v == second);
			for (int i = 0; i < n; ++i) require(out[i + 2] == (mode ? fps_mint::word(words[i]) : fps_mint::raw(words[i])));
		}
	}
	for (int n : {1, 2, 31, 32, 33, 63, 64, 65, 66, 95, 96, 97, 127, 128, 129, 191, 192, 193, 255, 256, 257, 383, 384, 385, 511, 512, 513, 1023, 1024, 1025, 4097, 16385})
		for (int input : {0, 1, 7, 32, 63, 64, n / 3, n, n + 17}) {
			fps a(input);
			for (auto &x : a) x = fps_mint::word(2 * fps_mint::mod() - 1 - rng() % 1000);
			if (input) a[0] = 0;
			const fps saved = a;
			std::vector<fps_mint> g;
			fps_detail::blocked_poly::native_exp(a.data(), a.size(), g, n);
			require(g.size() == unsigned(n) && g[0] == fps_mint(1));
			fps actual(g), derivative = actual.diff(), expected = (actual * a.diff()).pre(n - 1);
			expected.resize(n);
			require(derivative == expected);
			for (unsigned i = 0; i < a.size(); ++i) require(a[i].v == saved[i].v);
			if (n <= 1025) {
				fps naive(n);
				naive[0] = 1;
				for (int i = 1; i < n; ++i) {
					for (int j = 1; j <= i && j < input; ++j) naive[i] += a[j] * j * naive[i - j];
					naive[i] /= fps_mint(i);
				}
				require(g == naive);
			}
			if (input) {
				a[0] = rng() % (fps_mint::mod() - 1) + 1;
				const fps saved_power = a;
				for (long long e : {1LL, 5LL, 998244353LL, 1000000000000000000LL}) {
					std::vector<fps_mint> h;
					fps_detail::blocked_poly::native_power(a.data(), a.size(), h, n, e % fps_mint::mod(), a[0].pow(e));
					fps f = a.pre(n);
					f.resize(n);
					fps hp(h);
					auto left = (hp.diff() * f).pre(n - 1), right = (f.diff() * hp).pre(n - 1);
					left.resize(n - 1);
					right.resize(n - 1);
					right *= fps_mint(e);
					require(left == right && h[0] == a[0].pow(e));
					for (unsigned i = 0; i < a.size(); ++i) require(a[i].v == saved_power[i].v);
				}
			}
		}
#endif
}
