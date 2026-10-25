#include "data_structure/mo_order.hpp"
#include <numeric>
#include <random>

void check(const std::vector<std::pair<int, int>> &ranges, int block_size) {
	auto original = ranges;
	auto actual = mo_order(ranges, block_size);
	assert(original == ranges);
	std::vector<int> expected(ranges.size());
	std::iota(expected.begin(), expected.end(), 0);
	if (ranges.empty()) {
		assert(actual.empty());
		return;
	}
	int min_l = ranges[0].first, max_l = min_l, min_r = ranges[0].second, max_r = min_r;
	for (auto [l, r] : ranges) {
		min_l = std::min(min_l, l), max_l = std::max(max_l, l);
		min_r = std::min(min_r, r), max_r = std::max(max_r, r);
	}
	if (!block_size) block_size = std::max(1, int(std::sqrt((double(max_l) - min_l + 1) * (double(max_r) - min_r + 1) / ranges.size())));
	std::sort(expected.begin(), expected.end(), [&](int a, int b) {
		int ba = (ranges[a].first - min_l) / block_size, bb = (ranges[b].first - min_l) / block_size;
		if (ba != bb) return ba < bb;
		if (ranges[a].second != ranges[b].second)
			return ba & 1 ? ranges[a].second > ranges[b].second : ranges[a].second < ranges[b].second;
		return a < b;
	});
	assert(actual == expected);
}

int main() {
	std::mt19937_64 rng(20261020);
	for (int size : {0, 1, 16, 255, 256, 511, 512, 1023, 1024, 2047, 2048, 2049, 8192, 200000}) {
		for (int shape = 0; shape < 4; ++shape) {
			std::vector<std::pair<int, int>> ranges;
			for (int i = 0; i < size; ++i) {
				int l = rng() % std::numeric_limits<int>::max(), r = rng() % std::numeric_limits<int>::max();
				if (shape == 1) l %= 16, r %= 16;
				if (shape == 2) l = 0, r = std::numeric_limits<int>::max();
				if (shape == 3) l = i, r = std::numeric_limits<int>::max() - i;
				if (l > r) std::swap(l, r);
				ranges.emplace_back(l, r);
			}
			for (int block : {0, 1, 37, 1000000000, std::numeric_limits<int>::max()}) check(ranges, block);
		}
	}
}
