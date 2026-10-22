#include "components/wavelet_compression.hpp"
#include <random>

template<class T> void check(const std::vector<T> &a) {
	wavelet_detail::ValueIndex<T> index;
	index.build(a);
	auto expected = a;
	std::sort(expected.begin(), expected.end());
	expected.erase(std::unique(expected.begin(), expected.end()), expected.end());
	assert(index.n == int(a.size()) && index.values == expected && index.original.size() == a.size());
	for (std::size_t i = 0; i < a.size(); ++i) {
		auto rank = std::lower_bound(expected.begin(), expected.end(), a[i]) - expected.begin();
		assert(index.original[i] == rank && index.code(a[i]) == rank);
	}
	for (T x : {std::numeric_limits<T>::lowest(), T(0), T(7), std::numeric_limits<T>::max()})
		assert(index.code(x) == std::lower_bound(expected.begin(), expected.end(), x) - expected.begin());
	index.build({});
	assert(index.values.empty() && index.original.empty() && index.n == 0 && index.code(T(1)) == 0);
	index.build(a);
	assert(index.values == expected);
}

int main() {
	std::mt19937_64 random(20261023);
	for (int n : {0, 1, 2, 63, 1023, 1024, 1025, 4096}) {
		std::vector<std::int64_t> a(n);
		for (int mode = 0; mode < 4; ++mode) {
			for (auto &x : a) x = mode == 0 ? std::int64_t(random()) : mode == 1 ? std::int64_t(random() % 31) - 15 : mode == 2 ? std::numeric_limits<std::int64_t>::max() - std::int64_t(random() % 1000000) : 7;
			check(a);
			std::sort(a.begin(), a.end());
			check(a);
			std::reverse(a.begin(), a.end());
			check(a);
		}
		std::vector<std::uint32_t> unsigned_values(n);
		for (auto &x : unsigned_values) x = random();
		check(unsigned_values);
		std::vector<double> real(n);
		for (auto &x : real) x = double(random() % 1000) / 3;
		check(real);
	}
}
