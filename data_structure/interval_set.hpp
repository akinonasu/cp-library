#ifndef INTERVAL_SET_HPP
#define INTERVAL_SET_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template<class T = std::int64_t, class Size = std::conditional_t<
	std::is_integral_v<T> && sizeof(T) <= 8, std::uint64_t,
	std::conditional_t<std::is_same_v<T, __int128_t>, __uint128_t, T>>>
class IntervalSet {
	static_assert(std::numeric_limits<T>::is_integer && !std::is_same_v<T, bool>);
	static_assert(std::numeric_limits<Size>::is_integer && !std::is_same_v<Size, bool>);
	std::map<T, T> ranges;
	Size total = 0;
	static Size length(const T &l, const T &r) {
		return static_cast<Size>(r) - static_cast<Size>(l);
	}

public:
	using coordinate_type = T;
	using size_type = Size;
	using interval_type = std::pair<T, T>;
	using const_iterator = typename std::map<T, T>::const_iterator;
	using const_reverse_iterator = typename std::map<T, T>::const_reverse_iterator;

	IntervalSet() = default;
	explicit IntervalSet(std::vector<interval_type> intervals) { assign(std::move(intervals)); }
	IntervalSet(std::initializer_list<interval_type> intervals) { assign(intervals); }
	IntervalSet(const IntervalSet &) = default;
	IntervalSet(IntervalSet &&other) noexcept(std::is_nothrow_constructible_v<Size, int> && std::is_nothrow_swappable_v<Size>) { swap(other); }
	IntervalSet &operator=(IntervalSet other) noexcept(std::is_nothrow_swappable_v<Size>) { swap(other); return *this; }

	void assign(std::vector<interval_type> intervals) {
		std::sort(intervals.begin(), intervals.end());
		IntervalSet result;
		auto last = result.ranges.end();
		for (auto [l, r] : intervals) {
			assert(l <= r);
			if (l == r) continue;
			if (last != result.ranges.end() && l <= last->second) {
				if (last->second < r) {
					result.total += length(last->second, r);
					last->second = r;
				}
			} else {
				last = result.ranges.emplace_hint(result.ranges.end(), l, r);
				result.total += length(l, r);
			}
		}
		swap(result);
	}

	size_type insert(T l, T r) {
		assert(l <= r);
		if (l == r) return 0;
		auto it = ranges.upper_bound(l);
		if (it != ranges.begin() && std::prev(it)->second >= l) --it;
		if (it == ranges.end() || it->first > r) {
			ranges.emplace_hint(it, l, r);
			auto added = length(l, r);
			total += added;
			return added;
		}
		if (it->first <= l && r <= it->second) return 0;
		T left = (std::min)(l, it->first), right = (std::max)(r, it->second);
		auto removed = length(it->first, it->second);
		auto next_it = std::next(it);
		while (next_it != ranges.end() && next_it->first <= right) {
			right = (std::max)(right, next_it->second);
			removed += length(next_it->first, next_it->second);
			next_it = ranges.erase(next_it);
		}
		if (left == it->first) it->second = right;
		else {
			auto node = ranges.extract(it);
			node.key() = left;
			node.mapped() = right;
			ranges.insert(next_it, std::move(node));
		}
		size_type added = length(left, right) - removed;
		total += added;
		return added;
	}

	size_type insert(T x) {
		if constexpr (std::numeric_limits<T>::is_bounded) assert(x < (std::numeric_limits<T>::max)());
		return insert(x, static_cast<T>(x + 1));
	}

	size_type erase(T l, T r) {
		assert(l <= r);
		if (l == r) return 0;
		auto it = ranges.upper_bound(l);
		if (it != ranges.begin() && std::prev(it)->second > l) --it;
		size_type removed = 0;
		if (it != ranges.end() && it->first < l) {
			T right = it->second;
			if (r < right) {
				ranges.emplace_hint(std::next(it), r, right);
				it->second = l;
				removed = length(l, r);
				total -= removed;
				return removed;
			}
			removed = length(l, right);
			it->second = l;
			++it;
		}
		while (it != ranges.end() && it->first < r) {
			if (r < it->second) {
				removed += length(it->first, r);
				auto hint = std::next(it);
				auto node = ranges.extract(it);
				node.key() = r;
				ranges.insert(hint, std::move(node));
				break;
			}
			removed += length(it->first, it->second);
			it = ranges.erase(it);
		}
		total -= removed;
		return removed;
	}

	size_type erase(T x) {
		if constexpr (std::numeric_limits<T>::is_bounded)
			if (x == (std::numeric_limits<T>::max)()) return 0;
		return erase(x, static_cast<T>(x + 1));
	}

	const_iterator find(T x) const {
		auto it = ranges.upper_bound(x);
		if (it == ranges.begin()) return ranges.end();
		--it;
		return x < it->second ? it : ranges.end();
	}
	const_iterator lower_bound(T x) const {
		auto it = ranges.upper_bound(x);
		if (it != ranges.begin() && x < std::prev(it)->second) --it;
		return it;
	}
	bool contains(T x) const { return find(x) != end(); }
	bool contains(T l, T r) const {
		assert(l <= r);
		if (l == r) return true;
		auto it = find(l);
		return it != end() && r <= it->second;
	}
	bool intersects(T l, T r) const {
		assert(l <= r);
		if (l == r) return false;
		auto it = lower_bound(l);
		return it != end() && it->first < r;
	}
	size_type count(T l, T r) const {
		assert(l <= r);
		if (l == r) return 0;
		size_type result = 0;
		for (auto it = lower_bound(l); it != end() && it->first < r; ++it)
			result += length((std::max)(l, it->first), (std::min)(r, it->second));
		return result;
	}
	std::optional<T> next(T x) const {
		auto it = lower_bound(x);
		if (it == end()) return std::nullopt;
		return (std::max)(x, it->first);
	}
	std::optional<T> prev(T x) const {
		auto it = ranges.upper_bound(x);
		if (it == ranges.begin()) return std::nullopt;
		--it;
		return (std::min)(x, static_cast<T>(it->second - 1));
	}
	T mex(T x = 0) const {
		auto it = find(x);
		return it == end() ? x : it->second;
	}
	std::optional<T> min() const {
		if (empty()) return std::nullopt;
		return begin()->first;
	}
	std::optional<T> max() const {
		if (empty()) return std::nullopt;
		return static_cast<T>(ranges.rbegin()->second - 1);
	}
	size_type size() const { return total; }
	std::size_t interval_count() const { return ranges.size(); }
	bool empty() const { return ranges.empty(); }
	const_iterator begin() const { return ranges.begin(); }
	const_iterator end() const { return ranges.end(); }
	const_reverse_iterator rbegin() const { return ranges.rbegin(); }
	const_reverse_iterator rend() const { return ranges.rend(); }
	void clear() { ranges.clear(); total = 0; }
	void swap(IntervalSet &other) noexcept(std::is_nothrow_swappable_v<Size>) {
		ranges.swap(other.ranges);
		using std::swap;
		swap(total, other.total);
	}
	friend void swap(IntervalSet &a, IntervalSet &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

#endif
