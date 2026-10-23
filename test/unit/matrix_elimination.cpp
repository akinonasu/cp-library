#include "components/matrix_elimination_modular.hpp"
#include <array>
#include <cassert>
#include <random>

template <std::uint32_t P>
struct Mod {
	std::uint32_t v = 0;
	Mod() = default;
	Mod(std::uint64_t x) : v(x % P) {}
	static constexpr std::uint32_t mod() { return P; }
	static Mod raw(std::uint32_t x) { Mod r; r.v = x; return r; }
	std::uint32_t val() const { return v; }
	friend Mod operator/(Mod a, Mod b) { Mod r = 1; for (auto e = P - 2; e; e >>= 1, b.v = std::uint64_t(b.v) * b.v % P) if (e & 1) r.v = std::uint64_t(r.v) * b.v % P; r.v = std::uint64_t(r.v) * a.v % P; return r; }
};

template <std::uint32_t P>
void check() {
	using Ops = matrix_detail::ModularField<Mod<P>>;
	using Engine = matrix_detail::ModularElimination<Ops>;
	std::mt19937 rng(42);
	for (int count = 1; count <= Engine::batch; ++count) for (int length : {0, 1, 7, 8, 15, 16, 17, 31, 32, 33, 64, 127, 129}) {
		std::vector<std::uint32_t> dst(length + 2, 17), expected(dst);
		std::array<std::vector<std::uint32_t>, Engine::batch> rows;
		std::array<const std::uint32_t *, Engine::batch> sources;
		std::array<std::uint32_t, Engine::batch> coefficients;
		for (int k = 0; k < count; ++k) {
			rows[k].resize(length + 2);
			for (auto &x : rows[k]) x = k & 1 ? P - 1 : rng() % P;
			sources[k] = rows[k].data();
			coefficients[k] = k & 1 ? P - 1 : rng() % P;
		}
		for (int j = 1; j <= length; ++j) {
			dst[j] = expected[j] = rng() % P;
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(rows[k][j]) * coefficients[k];
			expected[j] = sum % P;
		}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (matrix_detail::avx2()) {
			auto actual = dst;
			Engine::update256(actual.data() + 1, sources.data(), coefficients.data(), count, 1, length);
			assert(actual == expected);
		}
		if (matrix_detail::avx512_product()) {
			auto actual = dst;
			Engine::update512(actual.data() + 1, sources.data(), coefficients.data(), count, 1, length);
			assert(actual == expected);
		}
#endif
		Engine::update(dst.data() + 1, sources.data(), coefficients.data(), count, 1, length);
		assert(dst == expected);
	}
	for (int n : {0, 1, 2, 7, 8, 9, 15, 16, 17, 31, 32, 33, 65}) for (int m : {0, 1, 2, 7, 8, 9, 17, 31, 32, 33, 65, 129}) for (int extra : {0, 1, 7}) for (int kind = 0; kind < 3; ++kind) {
		int stride = m + extra;
		std::vector<std::uint32_t> a(std::size_t(n) * stride);
		for (int i = 0; i < n; ++i) for (int j = 0; j < stride; ++j) {
			if (kind == 0) a[std::size_t(i) * stride + j] = rng() % P;
			if (kind == 1) a[std::size_t(i) * stride + j] = (std::uint64_t(i) * 23 + j * 17) % P;
			if (kind == 2 && j > i) a[std::size_t(i) * stride + j] = P - 1;
		}
		for (bool reduced : {false, true}) {
			auto b = a, c = a;
			auto x = matrix_detail::eliminate<Ops, true>(b, n, stride, m, reduced, 0);
			auto y = Engine::run(c, n, stride, m, reduced);
			assert(x.pivots == y.pivots && x.determinant == y.determinant && b == c);
		}
	}
}
int main() { check<2>(); check<17>(); check<998244353>(); check<1000000007>(); check<2147483647>(); }
