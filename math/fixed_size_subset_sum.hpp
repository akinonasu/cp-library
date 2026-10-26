#ifndef FIXED_SIZE_SUBSET_SUM_HPP
#define FIXED_SIZE_SUBSET_SUM_HPP

#include "../components/fixed_size_subset_sum.hpp"
#include <limits>
#include <numeric>

inline std::vector<long long> fixed_size_subset_sums(std::vector<int> values, int k) {
	assert(k >= 0 && values.size() <= std::size_t(std::numeric_limits<int>::max()));
	for ([[maybe_unused]] int x : values) assert(x >= 0);
	if (!k) return {0};
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	int count = int(values.size());
	if (k > count) return {};
	long long total = std::accumulate(values.begin(), values.end(), 0LL);
	bool complement = k > count - k;
	if (complement) k = count - k;
	if (!k) return {total};
	if (k == 1) {
		std::vector<long long> result(values.begin(), values.end());
		if (complement) {
			for (auto& x : result) x = total - x;
			std::reverse(result.begin(), result.end());
		}
		return result;
	}
	std::uint64_t ways = 1;
	for (int i = 1; i <= k && ways <= 32768; ++i) ways = ways * (count - i + 1) / i;
	std::vector<long long> result;
	if (ways <= 32768) {
		result.reserve(ways);
		auto visit = [&](auto&& self, int start, int left, long long sum) -> void {
			if (!left) { result.push_back(sum); return; }
			for (int i = start; i <= count - left; ++i) self(self, i + 1, left - 1, sum + values[i]);
		};
		visit(visit, 0, k, 0);
		std::sort(result.begin(), result.end());
		result.erase(std::unique(result.begin(), result.end()), result.end());
	} else {
		int base = values[0], step = 0;
		for (int x : values) step = std::gcd(step, x - base);
		for (auto& x : values) x = (x - base) / step;
		long long low = 0, high = 0;
		for (int i = 0; i < k; ++i) low += values[i], high += values[count - 1 - i];
		if (values.back() == count - 1) {
			result.reserve(std::size_t(high - low) + 1);
			for (long long x = low; x <= high; ++x) result.push_back(1LL * k * base + x * step);
		} else {
			if (high >= (1 << 23)) throw std::length_error("sum range is too large");
			auto bound = fixed_size_subset_sum_detail::coefficient_bound(count, k);
			std::vector<unsigned char> present(std::size_t(high - low) + 1);
			std::size_t missing = present.size();
			fixed_size_subset_sum_detail::Digits product{1};
			for (auto pass : fixed_size_subset_sum_detail::passes) {
				missing -= pass.include(values, k, int(low), present);
				fixed_size_subset_sum_detail::multiply(product, pass.modulus);
				if (!missing || fixed_size_subset_sum_detail::less(bound, product)) break;
				int left = 0, right = 0, candidates = 0;
				long long small = low - values[k - 1], large = high - values[count - k];
				for (std::size_t i = 0; i < present.size() && candidates < count; ++i) {
					if (present[i]) continue;
					long long sum = low + i;
					while (left < count && values[left] < sum - large) ++left;
					while (right < count && values[right] <= sum - small) ++right;
					candidates = std::max(candidates, right - left);
				}
				bound = fixed_size_subset_sum_detail::coefficient_bound(candidates, k);
				if (fixed_size_subset_sum_detail::less(bound, product)) break;
			}
			result.reserve(present.size() - missing);
			for (std::size_t i = 0; i < present.size(); ++i)
				if (present[i]) result.push_back(1LL * k * base + (low + i) * step);
		}
	}
	if (complement) {
		for (auto& x : result) x = total - x;
		std::reverse(result.begin(), result.end());
	}
	return result;
}

#endif
