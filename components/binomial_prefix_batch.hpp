#ifndef BINOMIAL_PREFIX_BATCH_HPP
#define BINOMIAL_PREFIX_BATCH_HPP

#include "binomial_prefix_online.hpp"
#include <limits>
#include <utility>

namespace binomial_prefix_detail {
template<class Mint, class Table, class K>
std::vector<Mint> batch(const std::vector<std::pair<int, K>>& queries, int block_size) {
	static_assert(std::is_integral_v<K> && std::is_signed_v<K> && sizeof(K) <= 8);
	assert(block_size >= 0 && Mint::mod() > 1);
	std::vector<Mint> answer(queries.size());
	if (queries.empty()) return answer;
	int limit = 0;
	for (auto [n, k] : queries) {
		(void)k;
		assert(0 <= n && n < std::numeric_limits<int>::max() && std::uint64_t(n) < Mint::mod());
		limit = std::max(limit, n);
	}
	std::vector<Mint> power2(std::size_t(limit) + 1);
	power2[0] = Mint(1);
	for (int n = 1; n <= limit; ++n) power2[n] = power2[n - 1] + power2[n - 1];
	struct Point { int n, k; std::size_t id; bool reflect; };
	std::vector<Point> points;
	points.reserve(queries.size());
	int maximum = 0;
	for (std::size_t i = 0; i < queries.size(); ++i) {
		auto [n, k] = queries[i];
		if (k <= 0) continue;
		if (std::uint64_t(k) >= std::uint64_t(n) + 1) { answer[i] = power2[n]; continue; }
		bool reflect = k > (n + 1) / 2;
		int m = reflect ? n + 1 - int(k) : int(k);
		if (m == 1) answer[i] = reflect ? power2[n] - Mint(1) : Mint(1);
		else if ((n & 1) && m == (n + 1) / 2) answer[i] = power2[n - 1];
		else {
			points.push_back({n, m, i, reflect});
			maximum = std::max(maximum, n);
		}
	}
	if (points.empty()) return answer;
	auto store = [&](const Point& point, Mint value) {
		answer[point.id] = point.reflect ? power2[point.n] - value : value;
	};
	if (std::all_of(points.begin(), points.end(), [&](const Point& point) { return point.n == maximum; })) {
		Table table(maximum);
		int first = maximum, last = 0;
		for (const auto& point : points) first = std::min(first, point.k), last = std::max(last, point.k);
		std::vector<Mint> prefix(std::size_t(last - first) + 1);
		Mint value;
		if constexpr (std::is_same_v<Mint, mint>) value = table.factorial[maximum] * mint_detail::inner_product<true>(table.inverse_factorial.data(), table.inverse_factorial.data() + maximum, first);
		else value = table.range(maximum, 0, first);
		prefix[0] = value;
		for (int k = first; k < last; ++k) prefix[k - first + 1] = value += table.binom(maximum, k);
		for (const auto& point : points) store(point, prefix[point.k - first]);
	} else {
		int block = block_size ? block_size : std::max(1, int(std::round(maximum / std::sqrt(double(points.size())))));
		if (!block_size) {
			int alternative = std::max(1, int(std::round(maximum / std::sqrt(double(queries.size())))));
			if (alternative != block && alternative >= block / 2 + block % 2) {
				std::size_t count = std::min(points.size(), std::size_t(256));
				auto estimate = [&](int width) {
					int rows = maximum / width;
					double sum = 0;
					for (std::size_t i = 0; i < count; ++i) {
						const auto& point = points[i * (points.size() / count)];
						int row = std::min(int((std::int64_t(point.n) + width / 2) / width), rows);
						int column = std::min(int((std::int64_t(point.k - 1) + width / 2) / width), row);
						std::int64_t distance = std::abs(point.n - row * width) + std::int64_t(std::abs(point.k - 1 - column * width));
						sum += std::min(std::int64_t(std::min(point.k - 1, (point.n + 1) / 2 - point.k)), distance);
					}
					return double(maximum) * maximum / (4 * double(width)) + double(points.size()) * sum / double(count);
				};
				if (estimate(alternative) < estimate(block)) block = alternative;
			}
		}
		Online<Mint, Table> online(maximum, block);
		for (const auto& point : points) store(point, online.query(point.n, point.k));
	}
	return answer;
}
}

#endif
