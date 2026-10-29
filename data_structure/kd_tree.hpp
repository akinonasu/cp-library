#ifndef KD_TREE_HPP
#define KD_TREE_HPP

#include "../components/d_ary_heap.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template<class T = std::int64_t, int Dimension = 2,
	class Distance = std::conditional_t<std::is_integral_v<T>, __int128_t, T>>
class KDTree {
	static_assert(Dimension > 0);
	static_assert(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>);
	static_assert(!std::is_integral_v<T> || sizeof(T) <= 8);
	static_assert(std::numeric_limits<Distance>::is_bounded && !std::is_same_v<Distance, bool>);
	static_assert(std::numeric_limits<Distance>::is_integer == std::is_integral_v<T>);
	static_assert(std::numeric_limits<Distance>::is_integer ? sizeof(Distance) <= 16 :
		(std::is_floating_point_v<Distance> && std::numeric_limits<Distance>::digits >= std::numeric_limits<T>::digits));
  public:
	using point_type = std::array<T, Dimension>;
	using distance_type = Distance;
	struct neighbor_type {
		int index;
		Distance distance_squared;
		bool operator==(const neighbor_type&) const = default;
	};

  private:
	using Wide = std::conditional_t<std::is_integral_v<T>, std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>, Distance>;
	struct Entry { point_type point; int index; };
	struct Node { point_type lower, upper; int begin, end, minimum; std::uint32_t right = 0; };
	struct Less {
		bool operator()(const neighbor_type& a, const neighbor_type& b) const {
			return a.distance_squared < b.distance_squared || (a.distance_squared == b.distance_squared && a.index < b.index);
		}
	};
	using Heap = DaryHeap<neighbor_type, Less, 4>;
	std::vector<Entry> points_;
	std::vector<int> positions_;
	std::vector<Node> nodes_;
	int leaf_size_ = 16;
	static void check_point([[maybe_unused]] const point_type& point) {
		if constexpr (std::is_floating_point_v<T>)
			for ([[maybe_unused]] T value : point) assert(std::isfinite(value));
	}
	void check_exclude([[maybe_unused]] int exclude) const { assert(-1 <= exclude && exclude < size()); }
	static Distance square_difference(T a, T b) {
		Wide difference = Wide(a) - Wide(b);
		Distance value = Distance(difference);
		if constexpr (std::numeric_limits<Distance>::is_integer && sizeof(Distance) < sizeof(unsigned))
			return Distance(unsigned(value) * unsigned(value));
		return value * value;
	}
	static Distance distance(const point_type& a, const point_type& b) {
		Distance result = 0;
		for (int d = 0; d < Dimension; ++d) result += square_difference(a[d], b[d]);
		return result;
	}
	static Distance lower_bound(const Node& node, const point_type& query) {
		Distance result = 0;
		for (int d = 0; d < Dimension; ++d) {
			if (query[d] < node.lower[d]) result += square_difference(query[d], node.lower[d]);
			else if (node.upper[d] < query[d]) result += square_difference(query[d], node.upper[d]);
		}
		return result;
	}
	std::uint32_t construct(int begin, int end) {
		std::uint32_t id = std::uint32_t(nodes_.size());
		Node node{points_[begin].point, points_[begin].point, begin, end, points_[begin].index};
		for (int i = begin + 1; i < end; ++i) {
			node.minimum = std::min(node.minimum, points_[i].index);
			for (int d = 0; d < Dimension; ++d) {
				node.lower[d] = std::min(node.lower[d], points_[i].point[d]);
				node.upper[d] = std::max(node.upper[d], points_[i].point[d]);
			}
		}
		nodes_.push_back(node);
		if (end - begin <= leaf_size_) return id;
		int axis = 0;
		for (int d = 1; d < Dimension; ++d)
			if (Wide(node.upper[axis]) - Wide(node.lower[axis]) < Wide(node.upper[d]) - Wide(node.lower[d])) axis = d;
		int middle = begin + (end - begin) / 2;
		std::nth_element(points_.begin() + begin, points_.begin() + middle, points_.begin() + end,
			[axis](const Entry& a, const Entry& b) {
				return a.point[axis] < b.point[axis] || (a.point[axis] == b.point[axis] && a.index < b.index);
			});
		construct(begin, middle);
		nodes_[id].right = construct(middle, end);
		return id;
	}
	bool promising(Distance bound, std::uint32_t id, const neighbor_type& best) const {
		return Less{}({nodes_[id].minimum, bound}, best);
	}
	void nearest_node(std::uint32_t id, const point_type& query, int exclude, neighbor_type& best) const {
		const auto& node = nodes_[id];
		if (!node.right) {
			for (int i = node.begin; i < node.end; ++i) {
				const auto& point = points_[i];
				if (point.index == exclude) continue;
				neighbor_type candidate{point.index, distance(query, point.point)};
				if (Less{}(candidate, best)) best = candidate;
			}
			return;
		}
		std::uint32_t first = id + 1, second = node.right;
		Distance a = lower_bound(nodes_[first], query), b = lower_bound(nodes_[second], query);
		if (Less{}({nodes_[second].minimum, b}, {nodes_[first].minimum, a})) std::swap(first, second), std::swap(a, b);
		if (promising(a, first, best)) nearest_node(first, query, exclude, best);
		if (promising(b, second, best)) nearest_node(second, query, exclude, best);
	}
	void nearest_k_node(std::uint32_t id, const point_type& query, int k, int exclude, Heap& heap) const {
		const auto& node = nodes_[id];
		if (!node.right) {
			for (int i = node.begin; i < node.end; ++i) {
				const auto& point = points_[i];
				if (point.index == exclude) continue;
				neighbor_type candidate{point.index, distance(query, point.point)};
				if (int(heap.size()) < k) heap.push(candidate);
				else if (Less{}(candidate, heap.top())) heap.replace_top(candidate);
			}
			return;
		}
		std::uint32_t first = id + 1, second = node.right;
		Distance a = lower_bound(nodes_[first], query), b = lower_bound(nodes_[second], query);
		if (Less{}({nodes_[second].minimum, b}, {nodes_[first].minimum, a})) std::swap(first, second), std::swap(a, b);
		if (int(heap.size()) < k || promising(a, first, heap.top())) nearest_k_node(first, query, k, exclude, heap);
		if (int(heap.size()) < k || promising(b, second, heap.top())) nearest_k_node(second, query, k, exclude, heap);
	}
	static bool nonempty_box(const point_type& lower, const point_type& upper) {
		for (int d = 0; d < Dimension; ++d) if (!(lower[d] < upper[d])) return false;
		return true;
	}
	template<bool Report, class F>
	int range_node(std::uint32_t id, const point_type& lower, const point_type& upper, F& visit) const {
		const auto& node = nodes_[id];
		bool inside = true;
		for (int d = 0; d < Dimension; ++d) {
			if (node.upper[d] < lower[d] || !(node.lower[d] < upper[d])) return 0;
			inside &= !(node.lower[d] < lower[d]) && node.upper[d] < upper[d];
		}
		if (inside) {
			if constexpr (Report) for (int i = node.begin; i < node.end; ++i) visit(points_[i].index);
			return node.end - node.begin;
		}
		if (node.right) return range_node<Report>(id + 1, lower, upper, visit) + range_node<Report>(node.right, lower, upper, visit);
		int count = 0;
		for (int i = node.begin; i < node.end; ++i) {
			bool found = true;
			for (int d = 0; d < Dimension; ++d)
				found &= !(points_[i].point[d] < lower[d]) && points_[i].point[d] < upper[d];
			if (found) {
				if constexpr (Report) visit(points_[i].index);
				++count;
			}
		}
		return count;
	}

  public:
	KDTree() = default;
	explicit KDTree(std::span<const point_type> points, int leaf_size = 16) { build(points, leaf_size); }
	void build(std::span<const point_type> points, int leaf_size = 16) {
		assert(points.size() <= std::size_t(std::numeric_limits<int>::max()) && leaf_size > 0);
		KDTree next;
		next.leaf_size_ = leaf_size;
		next.points_.reserve(points.size());
		for (int i = 0; i < int(points.size()); ++i) {
			check_point(points[i]);
			next.points_.push_back({points[i], i});
		}
		next.positions_.resize(points.size());
		if (!points.empty()) {
			next.nodes_.reserve(std::min(points.size() * 2, (points.size() / std::size_t(leaf_size) + 1) * 4));
			next.construct(0, int(points.size()));
			for (int i = 0; i < int(points.size()); ++i) next.positions_[next.points_[i].index] = i;
		}
		*this = std::move(next);
	}
	int size() const { return int(points_.size()); }
	bool empty() const { return points_.empty(); }
	int leaf_size() const { return leaf_size_; }
	const point_type& point(int index) const {
		assert(0 <= index && index < size());
		return points_[positions_[index]].point;
	}
	std::optional<neighbor_type> nearest(const point_type& query, int exclude = -1) const {
		check_point(query);
		check_exclude(exclude);
		if (empty() || (size() == 1 && exclude == 0)) return std::nullopt;
		neighbor_type best{std::numeric_limits<int>::max(), std::numeric_limits<Distance>::max()};
		nearest_node(0, query, exclude, best);
		return best;
	}
	std::vector<neighbor_type> k_nearest(const point_type& query, int k, int exclude = -1) const {
		assert(k >= 0);
		check_point(query);
		check_exclude(exclude);
		k = std::min(k, size() - (exclude != -1));
		if (!k) return {};
		if (k == 1) return {*nearest(query, exclude)};
		Heap heap;
		heap.reserve(k);
		nearest_k_node(0, query, k, exclude, heap);
		auto result = heap.values();
		std::sort(result.begin(), result.end(), Less{});
		return result;
	}
	int range_count(const point_type& lower, const point_type& upper) const {
		if (empty() || !nonempty_box(lower, upper)) return 0;
		auto visit = [](int) {};
		return range_node<false>(0, lower, upper, visit);
	}
	template<class F> void range_visit(const point_type& lower, const point_type& upper, F&& visit) const {
		if (!empty() && nonempty_box(lower, upper)) range_node<true>(0, lower, upper, visit);
	}
	std::vector<int> range_report(const point_type& lower, const point_type& upper) const {
		std::vector<int> result;
		range_visit(lower, upper, [&](int index) { result.push_back(index); });
		return result;
	}
};

#endif
