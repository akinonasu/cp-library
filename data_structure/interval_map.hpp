#ifndef INTERVAL_MAP_HPP
#define INTERVAL_MAP_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template<class T = std::int64_t, class V = std::int64_t, class Size = std::conditional_t<
	std::is_integral_v<T> && sizeof(T) <= 8, std::uint64_t,
	std::conditional_t<std::is_same_v<T, __int128_t>, __uint128_t, T>>, class Equal = std::equal_to<V>>
class IntervalMap {
	static_assert(std::numeric_limits<T>::is_integer && !std::is_same_v<T, bool>);
	static_assert(std::numeric_limits<Size>::is_integer && !std::is_same_v<Size, bool>);

public:
	using coordinate_type = T;
	using mapped_type = V;
	using size_type = Size;
	using value_equal = Equal;
	struct interval_type { T left, right; V value; };
	struct segment_type { T right; V value; };
	using const_iterator = typename std::map<T, segment_type>::const_iterator;
	using const_reverse_iterator = typename std::map<T, segment_type>::const_reverse_iterator;

private:
	std::map<T, segment_type> ranges;
	Size total = 0;
	[[no_unique_address]] Equal equal;
	static Size length(const T &l, const T &r) { return static_cast<Size>(r) - static_cast<Size>(l); }
	auto split(const T &x) {
		auto it = ranges.lower_bound(x);
		if (it != ranges.end() && it->first == x) return it;
		if (it == ranges.begin()) return it;
		auto previous = std::prev(it);
		if (previous->second.right <= x) return it;
		auto next = ranges.emplace_hint(it, x, segment_type{previous->second.right, previous->second.value});
		previous->second.right = x;
		return next;
	}

public:
	explicit IntervalMap(Equal compare = Equal{}) : equal(std::move(compare)) {}
	IntervalMap(T l, T r, const V &value, Equal compare = Equal{}) : IntervalMap(std::move(compare)) { assign(l, r, value); }
	explicit IntervalMap(std::vector<interval_type> intervals, Equal compare = Equal{}) : IntervalMap(std::move(compare)) { assign(std::move(intervals)); }
	IntervalMap(std::initializer_list<interval_type> intervals, Equal compare = Equal{}) : IntervalMap(std::vector<interval_type>(intervals), std::move(compare)) {}
	IntervalMap(const IntervalMap &) = default;
	IntervalMap(IntervalMap &&other) noexcept(std::is_nothrow_copy_constructible_v<Equal> && std::is_nothrow_constructible_v<Size, int> && std::is_nothrow_swappable_v<Size> && std::is_nothrow_swappable_v<Equal>)
		: equal(other.equal) { swap(other); }
	IntervalMap &operator=(IntervalMap other) noexcept(std::is_nothrow_swappable_v<Size> && std::is_nothrow_swappable_v<Equal>) { swap(other); return *this; }

	void assign(T l, T r, const V &value) {
		assert(l <= r);
		if (l == r) return;
		auto existing = find(l);
		if (existing != end() && r <= existing->second.right && equal(existing->second.value, value)) return;
		V stored(value);
		auto last = split(r), first = split(l);
		for (auto it = first; it != last; ++it) total -= length(it->first, it->second.right);
		ranges.erase(first, last);
		auto it = ranges.emplace_hint(last, l, segment_type{r, std::move(stored)});
		total += length(l, r);
		if (it != ranges.begin()) {
			auto previous = std::prev(it);
			if (previous->second.right == l && equal(previous->second.value, it->second.value)) {
				previous->second.right = r;
				ranges.erase(it);
				it = previous;
			}
		}
		auto next = std::next(it);
		if (next != ranges.end() && it->second.right == next->first && equal(it->second.value, next->second.value)) {
			it->second.right = next->second.right;
			ranges.erase(next);
		}
	}
	void assign(std::vector<interval_type> intervals) {
		IntervalMap next(equal);
		for (const auto &[l, r, value] : intervals) next.assign(l, r, value);
		swap(next);
	}
	void set(T x, const V &value) {
		if constexpr (std::numeric_limits<T>::is_bounded) assert(x < (std::numeric_limits<T>::max)());
		assign(x, static_cast<T>(x + 1), value);
	}
	size_type erase(T l, T r) {
		assert(l <= r);
		if (l == r) return 0;
		auto last = split(r), first = split(l);
		size_type removed = 0;
		for (auto it = first; it != last; ++it) removed += length(it->first, it->second.right);
		ranges.erase(first, last);
		total -= removed;
		return removed;
	}
	size_type erase(T x) {
		if constexpr (std::numeric_limits<T>::is_bounded) if (x == (std::numeric_limits<T>::max)()) return 0;
		return erase(x, static_cast<T>(x + 1));
	}
	const_iterator find(T x) const {
		auto it = ranges.upper_bound(x);
		if (it == ranges.begin()) return ranges.end();
		--it;
		return x < it->second.right ? it : ranges.end();
	}
	const_iterator lower_bound(T x) const {
		auto it = ranges.upper_bound(x);
		if (it != ranges.begin() && x < std::prev(it)->second.right) --it;
		return it;
	}
	std::optional<V> get(T x) const {
		auto it = find(x);
		return it == end() ? std::nullopt : std::optional<V>(it->second.value);
	}
	const V &at(T x) const { auto it = find(x); assert(it != end()); return it->second.value; }
	bool contains(T x) const { return find(x) != end(); }
	bool contains(T l, T r) const {
		assert(l <= r);
		if (l == r) return true;
		auto it = find(l);
		if (it == end()) return false;
		T right = it->second.right;
		while (right < r) {
			++it;
			if (it == end() || right != it->first) return false;
			right = it->second.right;
		}
		return true;
	}
	bool contains(T l, T r, const V &value) const {
		assert(l <= r);
		if (l == r) return true;
		auto it = find(l);
		return it != end() && r <= it->second.right && equal(it->second.value, value);
	}
	bool intersects(T l, T r) const {
		assert(l <= r);
		if (l == r) return false;
		auto it = lower_bound(l);
		return it != end() && it->first < r;
	}
	template<class F> void for_each(T l, T r, F &&visit) const {
		assert(l <= r);
		if (l == r) return;
		for (auto it = lower_bound(l); it != end() && it->first < r; ++it)
			std::invoke(visit, (std::max)(l, it->first), (std::min)(r, it->second.right), it->second.value);
	}
	size_type count(T l, T r) const {
		size_type result = 0;
		for_each(l, r, [&](const T &a, const T &b, const V &) { result += length(a, b); });
		return result;
	}
	size_type size() const { return total; }
	std::size_t interval_count() const { return ranges.size(); }
	bool empty() const { return ranges.empty(); }
	const_iterator begin() const { return ranges.begin(); }
	const_iterator end() const { return ranges.end(); }
	const_reverse_iterator rbegin() const { return ranges.rbegin(); }
	const_reverse_iterator rend() const { return ranges.rend(); }
	void clear() { ranges.clear(); total = 0; }
	void swap(IntervalMap &other) noexcept(std::is_nothrow_swappable_v<Size> && std::is_nothrow_swappable_v<Equal>) {
		ranges.swap(other.ranges);
		using std::swap;
		swap(total, other.total); swap(equal, other.equal);
	}
	friend void swap(IntervalMap &a, IntervalMap &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

#endif
