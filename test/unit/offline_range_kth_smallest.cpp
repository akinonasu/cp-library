#include "data_structure/offline_range_kth_smallest.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

struct Key {
	int value;
	explicit Key(int x) : value(x) {}
	bool operator==(const Key &) const = default;
	bool operator<(const Key &other) const { return value < other.value; }
};

template <class T>
void check(const std::vector<T> &values, const std::vector<std::array<int, 3>> &queries) {
	std::vector<T> expected;
	for (auto [l, r, k] : queries) {
		std::vector<T> sorted(values.begin() + l, values.begin() + r);
		std::nth_element(sorted.begin(), sorted.begin() + k, sorted.end());
		expected.push_back(sorted[k]);
	}
	assert(offline_range_kth_smallest(values, queries) == expected);
	std::span<const std::array<int, 3>> view(queries);
	assert(offline_range_kth_smallest(values, view) == expected);
	if (!values.empty() && queries.size() <= 2000) {
		std::vector<T> sorted;
		std::vector<int> positions, offsets;
		int ordered = 0;
		auto codes = wavelet_detail::compress<T, false>(values, sorted, ordered, positions, offsets);
		offline_range_kth_detail::Frontier frontier(queries.size());
		for (std::size_t i = 0; i < queries.size(); ++i) {
			frontier.left[i] = unsigned(queries[i][0]); frontier.right[i] = unsigned(queries[i][1]); frontier.rank[i] = unsigned(queries[i][2]);
		}
		unsigned bits = std::bit_width(unsigned(sorted.size() - 1));
		if (bits) offline_range_kth_detail::run_scalar(frontier, codes, bits);
		for (std::size_t i = 0; i < queries.size(); ++i) assert(sorted[frontier.answer[i]] == expected[i]);
	}
}

int main() {
	check<int>({}, {});
	std::mt19937_64 random(9182379123);
	for (int n = 1; n <= 8; ++n) for (int trial = 0; trial < 30; ++trial) {
		std::vector<int> a(n);
		for (auto &value : a) value = int(random() % 7) - 3;
		std::vector<std::array<int, 3>> queries;
		for (int l = 0; l < n; ++l) for (int r = l + 1; r <= n; ++r) for (int k = 0; k < r - l; ++k) queries.push_back({l, r, k});
		check(a, queries);
	}
	for (int n : {1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 63, 64, 65, 1023, 1024, 1025, 32767, 32768, 32769, 40001, 65536, 65537}) {
		for (int mode = 0; mode < 4; ++mode) {
			std::vector<std::int64_t> a(n);
			for (auto &value : a) value = mode ? std::int64_t(random() % 9) - 4 : std::int64_t(random());
			if (mode == 2) std::sort(a.begin(), a.end());
			if (mode == 3) std::sort(a.rbegin(), a.rend());
			std::vector<std::array<int, 3>> queries;
			for (int i = 0; i < (n >= 32768 && mode < 2 ? 4200 : 100); ++i) {
				int l = int(random() % n), r = int(random() % n);
				if (l > r) std::swap(l, r);
				++r;
				if (n >= 32768) r = std::min(n, l + 1 + int(random() % 64));
				int k = int(random() % (r - l));
				if (i < 3) { l = 0; r = n; k = i == 0 ? 0 : i == 1 ? n - 1 : n / 2; }
				queries.push_back({l, r, k});
			}
			check(a, queries);
		}
	}
	check<std::uint64_t>({0, UINT64_MAX, UINT64_MAX - 1, 0, 1}, {{0, 5, 0}, {0, 5, 4}, {0, 5, 3}, {1, 4, 1}});
	check<std::string>({"abc", "", "ab", "abc", "q"}, {{0, 5, 0}, {0, 5, 4}, {0, 3, 1}, {2, 5, 1}});
	check<Key>({Key(3), Key(1), Key(1), Key(9)}, {{0, 4, 0}, {0, 4, 3}, {1, 4, 1}});
}
