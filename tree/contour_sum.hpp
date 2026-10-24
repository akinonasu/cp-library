#ifndef TREE_CONTOUR_SUM_HPP
#define TREE_CONTOUR_SUM_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace tree_contour_sum_detail {
template<class T, bool = std::is_integral_v<T>> struct Storage { using type = T; };
template<class T> struct Storage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct Storage<__int128_t, false> { using type = __uint128_t; };
#endif
}

template<class T = std::int64_t> class TreeContourSum {
	static_assert(!std::is_same_v<T, bool>);
	using U = typename tree_contour_sum_detail::Storage<T>::type;
	struct Micro { std::size_t distances; int first; std::uint8_t count, maximum; };
	struct Bucket { std::size_t offset = 0; int length = 0; };
	struct Route { int centroid, branch, distance; };
	std::vector<int> position_;
	std::vector<T> values_;
	std::vector<Micro> micro_;
	std::vector<std::uint8_t> distances_;
	std::vector<Bucket> buckets_;
	std::vector<std::size_t> offsets_;
	std::vector<Route> routes_;
	std::vector<U> sums_;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
	void add_bucket(int group, int distance, U value) {
		const auto& bucket = buckets_[group];
		for (unsigned i = unsigned(distance) + 1; i <= unsigned(bucket.length); i += i & -i)
			sums_[bucket.offset + i - 1] += value;
	}
	U sum_bucket(int group, int l, int r) const {
		const auto& bucket = buckets_[group];
		unsigned a = unsigned(std::clamp(l, 0, bucket.length)), b = unsigned(std::clamp(r, 0, bucket.length));
		U result{};
		while (b > a) result += sums_[bucket.offset + b - 1], b &= b - 1;
		while (a > b) result -= sums_[bucket.offset + a - 1], a &= a - 1;
		return result;
	}

  public:
	TreeContourSum() = default;
	TreeContourSum(int n, std::span<const std::pair<int, int>> edges, const std::vector<T>& values) { build(n, edges, values); }
	void build(int n, std::span<const std::pair<int, int>> edges, const std::vector<T>& values) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		assert(values.size() == std::size_t(n) && edges.size() == std::size_t(n ? n - 1 : 0));
		position_.assign(n, -1), micro_.resize(n);
		values_.clear(), values_.reserve(std::size_t(n) * 2), distances_.clear();
		buckets_.clear(), buckets_.reserve(std::size_t(n) * 2), sums_.clear();
		offsets_.assign(std::size_t(n) + 1, 0), routes_.clear();
		if (!n) return;
		std::vector<int> start(std::size_t(n) + 1), adjacent(edges.size() * 2);
		for (auto [u, v] : edges) {
			assert(0 <= u && u < n && 0 <= v && v < n && u != v);
			++start[u + 1], ++start[v + 1];
		}
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		{
			auto cursor = start;
			for (auto [u, v] : edges) adjacent[cursor[u]++] = v, adjacent[cursor[v]++] = u;
		}
		unsigned stride = std::bit_width(unsigned(n));
		if (std::size_t(n) > std::numeric_limits<std::size_t>::max() / stride / sizeof(Route)) throw std::length_error("TreeContourSum routes");
		auto pending = std::make_unique_for_overwrite<Route[]>(std::size_t(n) * stride);
		std::vector<std::uint8_t> removed(n), levels(n);
		std::vector<int> previous(n, -1), sub(n), distance(n), work;
		work.reserve(n);
		std::vector<std::pair<int, unsigned>> tasks{{0, 0}};
		std::vector<U> full, branch;
		auto append_bucket = [&](int id, const std::vector<U>& raw) {
			if (raw.size() > sums_.max_size() - sums_.size()) throw std::length_error("TreeContourSum buckets");
			std::size_t offset = sums_.size();
			buckets_[id] = {offset, int(raw.size())};
			sums_.insert(sums_.end(), raw.begin(), raw.end());
			for (unsigned i = 1; i <= raw.size(); ++i) {
				unsigned j = i + (i & -i);
				if (j <= raw.size()) sums_[offset + j - 1] += sums_[offset + i - 1];
			}
		};
		while (!tasks.empty()) {
			auto [s, level] = tasks.back();
			tasks.pop_back();
			work.clear(), work.push_back(s), previous[s] = -2;
			for (std::size_t i = 0; i < work.size(); ++i) {
				int v = work[i];
				sub[v] = 1;
				for (int e = start[v]; e < start[v + 1]; ++e) {
					int u = adjacent[e];
					if (removed[u] || u == previous[v]) continue;
					assert(level || previous[u] == -1);
					previous[u] = v, work.push_back(u);
				}
			}
			assert(level || work.size() == std::size_t(n));
			int count = int(work.size());
			if (count <= 32) {
				int first = int(values_.size());
				U total{};
				for (int v : work) {
					position_[v] = int(values_.size());
					values_.push_back(values[v]), total += U(values[v]);
				}
				values_.push_back(T(total));
				std::size_t base = distances_.size();
				if (std::size_t(count) * count > distances_.max_size() - base) throw std::length_error("TreeContourSum distances");
				distances_.resize(base + std::size_t(count) * count);
				std::array<std::uint8_t, 32> maximum{};
				for (int j = 1; j < count; ++j) {
					int p = position_[previous[work[j]]] - first;
					auto* row = distances_.data() + base + std::size_t(j) * count;
					const auto* parent_row = distances_.data() + base + std::size_t(p) * count;
					std::uint8_t farthest = 0;
					for (int k = 0; k < j; ++k) {
						std::uint8_t d = parent_row[k] + 1;
						row[k] = d, distances_[base + std::size_t(k) * count + j] = d;
						farthest = std::max(farthest, d), maximum[k] = std::max(maximum[k], d);
					}
					maximum[j] = farthest;
				}
				for (int j = 0; j < count; ++j)
					micro_[work[j]] = {base + std::size_t(j) * count, first, std::uint8_t(count), maximum[j]};

				for (int v : work) removed[v] = 1;
				continue;
			}
			int c = -1;
			for (auto it = work.rbegin(); it != work.rend(); ++it) {
				int v = *it;
				if (c == -1 && sub[v] >= (count + 1) / 2) c = v;
				if (previous[v] >= 0) sub[previous[v]] += sub[v];
			}
			removed[c] = 1;
			position_[c] = int(values_.size());
			values_.push_back(values[c]), micro_[c] = {0, position_[c], 0, 0};
			int all = int(buckets_.size());
			buckets_.push_back({});
			assert(level < stride);
			pending[std::size_t(level) * n + c] = {all, -1, 0}, levels[c] = std::uint8_t(level + 1);
			full.assign(1, U(values[c]));
			for (int e = start[c]; e < start[c + 1]; ++e) {
				int root = adjacent[e];
				if (removed[root]) continue;
				int group = int(buckets_.size());
				buckets_.push_back({});
				branch.assign(2, U{});
				work.clear(), work.push_back(root), previous[root] = c, distance[root] = 1;
				for (std::size_t i = 0; i < work.size(); ++i) {
					int v = work[i], d = distance[v];
					pending[std::size_t(level) * n + v] = {all, group, d}, levels[v] = std::uint8_t(level + 1);
					if (std::size_t(d) >= full.size()) full.resize(std::size_t(d) + 1);
					if (std::size_t(d) >= branch.size()) branch.resize(std::size_t(d) + 1);
					full[d] += U(values[v]), branch[d] += U(values[v]);
					for (int j = start[v]; j < start[v + 1]; ++j) {
						int u = adjacent[j];
						if (removed[u] || u == previous[v]) continue;
						previous[u] = v, distance[u] = d + 1, work.push_back(u);
					}
				}
				append_bucket(group, branch);
				tasks.emplace_back(root, level + 1);
			}
			append_bucket(all, full);
		}
		for (int v = 0; v < n; ++v) offsets_[v + 1] = offsets_[v] + levels[v];
		routes_.reserve(offsets_.back());
		for (int v = 0; v < n; ++v)
			for (unsigned level = levels[v]; level-- > 0;) routes_.push_back(pending[std::size_t(level) * n + v]);
	}

	int size() const { return int(position_.size()); }
	bool empty() const { return position_.empty(); }
	const T& get(int v) const { check(v); return values_[position_[v]]; }
	void add(int v, T value) {
		check(v);
		values_[position_[v]] = T(U(values_[position_[v]]) + U(value));
		const auto& micro = micro_[v];
		if (micro.count) values_[micro.first + micro.count] = T(U(values_[micro.first + micro.count]) + U(value));
		for (std::size_t i = offsets_[v]; i < offsets_[v + 1]; ++i) {
			const auto& route = routes_[i];
			add_bucket(route.centroid, route.distance, U(value));
			if (route.branch != -1) add_bucket(route.branch, route.distance, U(value));
		}
	}
	void set(int v, T value) { add(v, T(U(value) - U(get(v)))); }
	T prod(int v, std::int64_t l, std::int64_t r) const {
		check(v);
		l = std::max<std::int64_t>(l, 0), r = std::min<std::int64_t>(r, size());
		if (l >= r) return T{};
		U answer{};
		const auto& micro = micro_[v];
		if (micro.count && l <= micro.maximum) {
			if (l == 0 && r > micro.maximum) answer += U(values_[micro.first + micro.count]);
			else {
				const auto* distances = distances_.data() + micro.distances;
				const auto* values = values_.data() + micro.first;
				for (int i = 0; i < micro.count; ++i)
					if (l <= distances[i] && distances[i] < r) answer += U(values[i]);
			}
		}
		for (std::size_t i = offsets_[v]; i < offsets_[v + 1]; ++i) {
			const auto& route = routes_[i];
			int low = int(l) - route.distance, high = int(r) - route.distance;
			answer += sum_bucket(route.centroid, low, high);
			if (route.branch != -1) answer -= sum_bucket(route.branch, low, high);
		}
		return T(answer);
	}
};

#endif
