#include "convolution/min_plus_convolution.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

template <class T, class Compare>
std::vector<T> naive(const std::vector<T> &a, const std::vector<T> &b, Compare compare) {
	if (a.empty() || b.empty()) return {};
	std::vector<T> result(a.size() + b.size() - 1);
	std::vector<bool> done(result.size());
	for (std::size_t i = 0; i < a.size(); ++i)
		for (std::size_t j = 0; j < b.size(); ++j) {
			T value = a[i] + b[j];
			if (!done[i + j] || compare(value, result[i + j])) result[i + j] = value;
			done[i + j] = true;
		}
	return result;
}

template <class T>
void check(const std::vector<T> &a, const std::vector<T> &b) {
	const auto saved_a = a, saved_b = b;
	assert(min_plus_convolution_convex(a, b) == naive(a, b, std::less<T>{}));
	assert(a == saved_a && b == saved_b);
	if constexpr (std::is_signed_v<T>) {
		std::vector<T> x = a, y = b;
		for (auto &value : x) value = -value;
		for (auto &value : y) value = -value;
		assert(max_plus_convolution_concave(x, y) == naive(x, y, std::greater<T>{}));
	}
}

template <class T>
void small_integers() {
	std::mt19937 rng(71651);
	for (int n : {0, 1, 2, 3, 15, 16, 17, 65, 130})
		for (int m : {0, 1, 2, 3, 15, 16, 17, 65, 140}) {
			std::vector<T> a(n), b(m);
			for (int i = std::max(0, n - 3); i < n; ++i) a[i] = (i - std::max(0, n - 3) + 1) * (i - std::max(0, n - 3) + 2) / 2;
			for (auto &value : b) value = T(rng() % 51);
			check(a, b);
			std::sort(b.begin(), b.end());
			for (int i = 1; i < m; ++i) b[i] = 0;
			check(a, b);
		}
}

void boundaries() {
	using I = std::int64_t;
	using U = std::uint64_t;
	std::mt19937_64 rng(934555);
	for (int repeat = 0; repeat < 12; ++repeat) {
		std::vector<I> a(130), b(140);
		for (int i = 0; i < int(a.size()); ++i) a[i] = I(i) * i;
		for (auto &value : b) value = I(rng() % 10001);
		I abase = repeat % 2 ? std::numeric_limits<I>::min() + 100000 : std::numeric_limits<I>::max() - 100000;
		I bbase = repeat % 2 ? std::numeric_limits<I>::max() - 100000 : std::numeric_limits<I>::min() + 100000;
		for (auto &value : a) value += abase;
		for (auto &value : b) value += bbase;
		check(a, b);
		std::vector<U> x(130), y(140);
		for (int i = 0; i < int(x.size()); ++i) x[i] = std::numeric_limits<U>::max() - 100000 + U(i) * i;
		for (auto &value : y) value = rng() % 10001;
		check(x, y);
	}
	check(std::vector<I>{std::numeric_limits<I>::min() + 10, -1, std::numeric_limits<I>::max() - 10}, std::vector<I>{0, 0, 0});
	check(std::vector<U>{0, 0, std::numeric_limits<U>::max() - 10}, std::vector<U>{0, 1});
	assert(min_plus_convolution_convex(std::vector<U>{std::numeric_limits<U>::max()}, std::vector<U>{1}) == std::vector<U>{0});
	std::vector<I> a(130), b(140);
	a.back() = I(std::numeric_limits<std::uint32_t>::max()) - 3;
	for (int i = 0; i < int(b.size()); ++i) b[i] = i & 1;
	check(a, b);
	a.back() = I(1) << 50;
	check(a, b);
#if defined(__SIZEOF_INT128__)
	assert((!min_plus_convolution_integral_detail::multiply<I, true>({std::numeric_limits<I>::max()}, {1})));
	assert((!min_plus_convolution_integral_detail::multiply<I, true>({std::numeric_limits<I>::min()}, {-1})));
	assert((!min_plus_convolution_integral_detail::multiply<U, true>({std::numeric_limits<U>::max()}, {1})));
#endif
}

struct Value {
	long long value = 0;
	friend Value operator+(Value a, Value b) { return {a.value + b.value}; }
	friend bool operator<(Value a, Value b) { return a.value < b.value; }
	friend bool operator>(Value a, Value b) { return b < a; }
	friend bool operator==(Value, Value) = default;
};

void generic() {
	std::vector<Value> a{{3}, {1}, {0}, {3}}, b{{5}, {1}, {3}, {3}, {2}};
	assert(min_plus_convolution_convex(a, b) == naive(a, b, std::less<Value>{}));
	auto compare = [](Value x, Value y) { return x < y; };
	assert(min_plus_convolution_detail::multiply(a, b, compare) == naive(a, b, compare));
	check(std::vector<double>{3, 1, 0, 3}, std::vector<double>{5, 1, 3, 3, 2});
	assert(min_plus_convolution_convex(std::vector<bool>(4, false), std::vector<bool>(5, true)) == std::vector<bool>(8, true));
#if defined(__SIZEOF_INT128__)
	static_assert(!min_plus_convolution_integral_detail::supported<__int128_t>);
	std::vector<__int128_t> x(130), y(140);
	for (int i = 0; i < int(x.size()); ++i) x[i] = (__int128_t(1) << 120) + i * i;
	for (int i = 0; i < int(y.size()); ++i) y[i] = -(__int128_t(1) << 120) + i % 7;
	assert(min_plus_convolution_convex(x, y) == naive(x, y, std::less<__int128_t>{}));
#endif
}

void kernel() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (!min_plus_convolution_integral_detail::available()) return;
	using K = min_plus_convolution_integral_detail::Kernel;
	using U = std::uint32_t;
	constexpr int n = 17, m = 19, capacity = 48;
	alignas(32) std::array<U, n + 2 * K::pad + 1> storage_a{};
	alignas(32) std::array<U, m + 1> storage_b{};
	alignas(32) std::array<U, capacity + 2> output{};
	storage_a.fill(std::numeric_limits<U>::max() - 10);
	output.fill(71651);
	U *a = storage_a.data() + K::pad + 1, *b = storage_b.data() + 1;
	for (int i = 0; i < n; ++i) a[i] = i * i;
	for (int i = 0; i < m; ++i) b[i] = i % 11;
	const auto saved_a = storage_a;
	const auto saved_b = storage_b;
	const auto saved_output = output;
	assert(!K::multiply(a, b, n, m, output.data() + 1, capacity, 0));
	assert(output == saved_output);
	assert(K::multiply(a, b, n, m, output.data() + 1, capacity, 16 * (n + m)));
	assert(output.front() == saved_output.front() && output.back() == saved_output.back());
	assert(storage_a == saved_a && storage_b == saved_b);
	for (int k = 0; k < n + m - 1; ++k) {
		U best = std::numeric_limits<U>::max();
		for (int i = 0; i < n; ++i) if (0 <= k - i && k - i < m) best = std::min(best, a[i] + b[k - i]);
		assert(output[k + 1] == best);
	}
	assert(!K::multiply(a, b, n, m, output.data() + 1, capacity - 1, 16 * (n + m)));
	assert(!K::multiply(a, b, std::numeric_limits<int>::max(), m, output.data() + 1, capacity, 16 * (n + m)));
#endif
}

int main() {
	static_assert(!min_plus_convolution_integral_detail::supported<bool>);
	small_integers<std::int8_t>();
	small_integers<std::uint8_t>();
	small_integers<std::int16_t>();
	small_integers<std::uint16_t>();
	small_integers<std::int32_t>();
	small_integers<std::uint32_t>();
	small_integers<std::int64_t>();
	small_integers<std::uint64_t>();
	boundaries();
	generic();
	kernel();
}
