#include "components/dynamic_quad_vector.hpp"
#include "data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <tuple>
#include <type_traits>
#include <vector>

template<class V> void check_vector(const V& v, const std::vector<std::uint8_t>& a) {
	assert(v.size() == int(a.size()));
	int uniform = a.empty() ? -1 : a[0];
	for (auto value : a) if (value != uniform) uniform = -1;
	assert(v.uniform_digit() == uniform);
	for (int p : {0, int(a.size()) / 2, int(a.size())}) {
		std::array<int, 4> counts{};
		for (int i = 0; i < p; ++i) ++counts[a[i]];
		assert(v.ranks(p) == counts);
		for (int d = 0; d < 4; ++d) assert(v.rank(p, d) == counts[d]);
		if (p < int(a.size())) assert(v.access_rank(p) == std::pair(counts[a[p]], int(a[p])));
		assert(v.ranks(0, p) == std::pair(std::array<int, 4>{}, counts));
	}
	for (int d = 0; d < 4; ++d) {
		std::vector<int> positions;
		for (int i = 0; i < int(a.size()); ++i) if (a[i] == d) positions.push_back(i);
		assert(v.select(d, -1) == -1 && v.select(d, int(positions.size())) == -1);
		if (!positions.empty()) for (int k : {0, int(positions.size()) / 2, int(positions.size()) - 1}) assert(v.select(d, k) == positions[k]);
	}
}

template<class V> void vector_tests() {
	for (int n : {0, 1, 63, 64, 65, 1023, 1024, 1025, 32768}) for (int d = 0; d < 4; ++d) {
		std::vector<std::uint8_t> a(n, d);
		V v(a);
		check_vector(v, a);
		if (n) {
			for (int p : {0, n / 2, n - 1}) {
				assert(v.set_rank(p, d) == std::tuple(p, d, p));
				int next = (d + 1) % 4;
				assert(v.set_rank(p, next) == std::tuple(p, d, 0)); a[p] = next;
				check_vector(v, a);
				assert(v.set_rank(p, next) == std::tuple(0, next, 0));
				assert(v.set_rank(p, d) == std::tuple(0, next, p)); a[p] = d;
				check_vector(v, a);
			}
		}
	}
}

template<class W, class T> void check_query(const W& w, const std::vector<T>& a, int l, int r, T bound, T upper) {
	std::vector<T> sorted(a.begin() + l, a.begin() + r);
	std::sort(sorted.begin(), sorted.end());
	int p = int(std::lower_bound(sorted.begin(), sorted.end(), bound) - sorted.begin());
	assert(w.range_freq(l, r, bound) == p);
	assert(w.range_freq(l, r, bound, upper) == std::count_if(sorted.begin(), sorted.end(), [&](T x) { return bound <= x && x < upper; }));
	assert(w.count(l, r, bound) == std::count(sorted.begin(), sorted.end(), bound));
	assert(w.rank(bound, r) == std::count(a.begin(), a.begin() + r, bound));
	assert(w.prev_value(l, r, bound) == (p ? std::optional<T>(sorted[p - 1]) : std::nullopt));
	assert(w.next_value(l, r, bound) == (p < r - l ? std::optional<T>(sorted[p]) : std::nullopt));
	if (l < r) for (int k : {0, (r - l) / 2, r - l - 1}) {
		assert(w.kth_smallest(l, r, k) == sorted[k]);
		assert(w.kth_largest(l, r, k) == sorted[r - l - 1 - k]);
	}
	std::vector<int> positions;
	for (int i = 0; i < int(a.size()); ++i) if (a[i] == bound) positions.push_back(i);
	assert(w.select(bound, -1) == -1 && w.select(bound, int(positions.size())) == -1);
	if (!positions.empty()) for (int k : {0, int(positions.size()) / 2, int(positions.size()) - 1}) assert(w.select(bound, k) == positions[k]);
}

template<class T, int Bits> void uniform_matrix_tests() {
	using W = DynamicWaveletMatrix<T, Bits>;
	using U = std::make_unsigned_t<T>;
	constexpr U mask = std::numeric_limits<U>::max() >> (std::numeric_limits<U>::digits - Bits);
	std::mt19937_64 random(18228);
	for (U pattern : {U(0), U(0x5555555555555555ULL), U(0xaaaaaaaaaaaaaaaaULL), U(0xffffffffffffffffULL)}) {
		T value = T(pattern & mask);
		for (int n : {0, 1, 63, 64, 65, 1023, 1024, 1025, 32768}) {
			std::vector<T> a(n, value);
			W w(a);
			for (int trial = 0; trial < 8; ++trial) {
				int l = trial == 0 ? 0 : int(random() % (n + 1));
				int r = trial == 0 ? n : int(random() % (n + 1));
				if (l > r) std::swap(l, r);
				T lower = trial == 0 ? value : T(U(random()) & mask);
				T upper = trial == 0 ? std::numeric_limits<T>::max() : T(U(random()) & mask);
				check_query(w, a, l, r, lower, upper);
			}
			if (n) {
				int p = n / 2; T other = T(U(value) ^ U(0x1010101010101010ULL));
				w.set(p, value); assert(w[p] == value);
				w.set(p, other); a[p] = other;
				check_query(w, a, 0, n, value, other);
				check_query(w, a, p, p + 1, other, value);
				w.set(p, value); a[p] = value;
				check_query(w, a, 0, n, value, other);
			}
		}
	}
}

int main() {
	vector_tests<dynamic_wavelet_detail::QuadVector<>>();
	vector_tests<dynamic_wavelet_detail::QuadVector<32768, 4>>();
	uniform_matrix_tests<std::int64_t, 64>();
	uniform_matrix_tests<std::uint64_t, 63>();
	std::vector<unsigned> values{0x12, 0x96, 0x16, 0x92, 0x12, 0x92};
	DynamicWaveletMatrix<unsigned, 9> w(values);
	for (int l = 0; l <= int(values.size()); ++l) for (int r = l; r <= int(values.size()); ++r)
		for (unsigned bound = 0; bound <= 512; ++bound) check_query(w, values, l, r, bound, (bound + 17) % 513);
}
