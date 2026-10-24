#include "components/matrix_arithmetic.hpp"
#include "test/fixtures/recurrence_types.hpp"
#include <array>
#include <cassert>
#include <cfenv>
#include <limits>
#include <random>

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
template <std::uint32_t P>
__attribute__((target("avx512f,avx512dq"))) void reduce_batch(const std::int64_t *source, std::uint64_t *result) {
	_mm512_storeu_si512(result, matrix_detail::reduce_product_avx512<P>(_mm512_loadu_si512(source)));
}
#endif

template <std::uint32_t P>
void check() {
	std::mt19937_64 random(P);
	using T = RecurrenceModint<P>;
	for (const auto &[n, inner, m] : std::array<std::array<int, 3>, 22>{
		std::array{0, 16, 32}, {4, 0, 32}, {4, 16, 0}, {1, 1, 1}, {3, 15, 7},
		{4, 16, 8}, {5, 17, 9}, {7, 31, 15}, {8, 32, 16}, {9, 33, 17},
		{4, 63, 31}, {5, 64, 32}, {7, 65, 33}, {8, 127, 63}, {9, 128, 64},
		{3, 129, 65}, {5, 255, 127}, {7, 256, 128}, {9, 257, 129}, {17, 200, 200}, {9, 4097, 17}, {4, 4096, 8}}) {
		for (int kind = 0; kind < 5; ++kind) {
			std::vector<std::uint32_t> a(std::size_t(n) * inner), b(std::size_t(inner) * m), expected(std::size_t(n) * m);
			for (auto &x : a) x = kind == 0 ? 0 : kind == 1 ? P - 1 : kind == 2 ? P / 2 : random() % P;
			for (auto &x : b) x = kind == 0 ? P - 1 : kind == 1 ? P - 1 : kind == 2 ? (P - 1) / 2 + 1 : random() % P;
			for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
				std::uint32_t sum = 0;
				for (int k = 0; k < inner; ++k) sum = (sum + std::uint64_t(a[std::size_t(i) * inner + k]) * b[std::size_t(k) * m + j]) % P;
				expected[std::size_t(i) * m + j] = sum;
			}
			std::vector<T> x, y;
			for (auto z : a) x.push_back(T::raw(z));
			for (auto z : b) y.push_back(T::raw(z));
			auto result = matrix_detail::multiply(x, y, n, inner, m);
			assert(result.size() == expected.size());
			for (std::size_t i = 0; i < result.size(); ++i) assert(result[i].val() == expected[i]);
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
			if (matrix_detail::avx512_product() && n && inner && m) {
				std::vector<std::int32_t> centered_a(a.size()), packed_b(b.size());
				auto center = [](std::uint32_t value) { return value <= (P - 1) / 2 ? std::int32_t(value) : std::int32_t(std::int64_t(value) - P); };
				for (std::size_t i = 0; i < a.size(); ++i) centered_a[i] = center(a[i]);
				for (int j = 0; j < m; j += 16) {
					int width = std::min(16, m - j);
					for (int k = 0; k < inner; ++k) for (int s = 0; s < width; ++s)
						packed_b[std::size_t(j) * inner + std::size_t(k) * width + s] = center(b[std::size_t(k) * m + j + s]);
				}
				std::vector<std::uint32_t> actual(expected.size());
				matrix_detail::product_packed_avx512<P>(centered_a.data(), packed_b.data(), actual.data(), n, inner, m);
				assert(actual == expected);
			}
			if (matrix_detail::avx2() && n && inner && m) {
				std::vector<std::int32_t> centered_a(a.size()), packed_b(b.size());
				auto center = [](std::uint32_t value) { return value <= (P - 1) / 2 ? std::int32_t(value) : std::int32_t(std::int64_t(value) - P); };
				for (std::size_t i = 0; i < a.size(); ++i) centered_a[i] = center(a[i]);
				for (int j = 0; j < m; j += 8) {
					int width = std::min(8, m - j);
					for (int k = 0; k < inner; ++k) for (int q = 0; q < width; ++q)
						packed_b[std::size_t(j) * inner + std::size_t(k) * width + q] = center(b[std::size_t(k) * m + j + q]);
				}
				std::vector<std::uint32_t> actual(expected.size());
				matrix_detail::product_packed_avx2<P>(centered_a.data(), packed_b.data(), actual.data(), n, inner, m);
				assert(actual == expected);
			}
#endif
		}
	}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (matrix_detail::avx512_product()) {
		constexpr std::int64_t radius = P / 2;
		constexpr int step = int(std::min<std::int64_t>(32, (std::numeric_limits<std::int64_t>::max() - (P - 1)) / (radius * radius)));
		constexpr std::uint64_t bound = std::uint64_t(step) * radius * radius;
		const int previous = std::fegetround();
		for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
			assert(!std::fesetround(mode));
			for (int trial = 0; trial < 2000; ++trial) {
				std::array<std::int64_t, 8> source;
				std::array<std::uint64_t, 8> result;
				for (auto &x : source) {
					x = random() % bound;
					if (trial % 3 == 0) x = x / P * P + std::int64_t(random() % 3) - 1;
					if (trial % 3 == 1) x = bound - random() % 16;
					if (random() & 1) x = -x;
				}
				reduce_batch<P>(source.data(), result.data());
				for (int i = 0; i < 8; ++i) {
					auto expected = source[i] % std::int64_t(P);
					if (expected < 0) expected += P;
					assert(result[i] == std::uint64_t(expected));
				}
			}
		}
		assert(!std::fesetround(previous));
	}
#endif
}

int main() {
	check<2>();
	check<3>();
	check<4>();
	check<97>();
	check<65536>();
	check<998244353>();
	check<1000000007>();
	check<1073741824>();
	check<2147483646>();
	check<2147483647>();
}
