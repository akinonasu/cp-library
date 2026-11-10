#ifndef DOUBLE_ENDED_PRIORITY_QUEUE_HPP
#define DOUBLE_ENDED_PRIORITY_QUEUE_HPP

#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

template<class T, class Compare = std::less<T>>
class DoubleEndedPriorityQueue {
	std::vector<T> data;
	[[no_unique_address]] Compare compare;
	void up_min(std::size_t i) {
		while (i >= 2) {
			std::size_t p = ((i / 2 - 1) / 2) * 2;
			if (!compare(data[i], data[p])) break;
			std::swap(data[i], data[p]);
			i = p;
		}
	}
	void up_max(std::size_t i) {
		while (i >= 3) {
			std::size_t p = ((i / 2 - 1) / 2) * 2 + 1;
			if (!compare(data[p], data[i])) break;
			std::swap(data[i], data[p]);
			i = p;
		}
	}
	void down_min(std::size_t i, T value) {
		while (2 * i + 2 < data.size()) {
			std::size_t child = 2 * i + 2;
			if (child + 2 < data.size() && compare(data[child + 2], data[child])) child += 2;
			if (!compare(data[child], value)) break;
			data[i] = std::move(data[child]);
			i = child;
			if (i + 1 < data.size() && compare(data[i + 1], value)) std::swap(value, data[i + 1]);
		}
		data[i] = std::move(value);
	}
	void down_max(std::size_t i, T value) {
		while (2 * i < data.size()) {
			std::size_t child = 2 * i + 1;
			if (child == data.size()) --child;
			if (2 * i + 2 < data.size()) {
				std::size_t right = 2 * i + 3;
				if (right == data.size()) --right;
				if (compare(data[child], data[right])) child = right;
			}
			if (!compare(value, data[child])) break;
			data[i] = std::move(data[child]);
			i = child;
			if ((i & 1) && compare(value, data[i - 1])) std::swap(value, data[i - 1]);
		}
		data[i] = std::move(value);
	}

public:
	using value_type = T;
	using size_type = std::size_t;
	using value_compare = Compare;
	explicit DoubleEndedPriorityQueue(Compare compare = Compare{}) : compare(std::move(compare)) {}
	explicit DoubleEndedPriorityQueue(std::vector<T> values, Compare compare = Compare{})
		: data(std::move(values)), compare(std::move(compare)) {
		for (size_type i = 1; i < data.size(); i += 2)
			if (this->compare(data[i], data[i - 1])) std::swap(data[i], data[i - 1]);
		for (size_type k = (data.size() / 2 + 1) / 2; k--;) {
			down_min(k * 2, std::move(data[k * 2]));
			if (k * 2 + 1 < data.size()) down_max(k * 2 + 1, std::move(data[k * 2 + 1]));
		}
	}
	DoubleEndedPriorityQueue(const DoubleEndedPriorityQueue&) = default;
	DoubleEndedPriorityQueue(DoubleEndedPriorityQueue&& other) noexcept(std::is_nothrow_copy_constructible_v<Compare>)
		: data(std::move(other.data)), compare(other.compare) { other.data.clear(); }
	DoubleEndedPriorityQueue& operator=(DoubleEndedPriorityQueue other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		swap(other);
		return *this;
	}
	size_type size() const { return data.size(); }
	bool empty() const { return data.empty(); }
	void reserve(size_type n) { data.reserve(n); }
	const T& min() const { assert(!empty()); return data[0]; }
	const T& max() const { assert(!empty()); return data[data.size() > 1]; }
	void push(T value) {
		data.push_back(std::move(value));
		size_type i = data.size() - 1;
		if (i & 1) {
			if (compare(data[i], data[i - 1])) {
				std::swap(data[i], data[i - 1]);
				up_min(i - 1);
			} else up_max(i);
		} else if (i) {
			size_type p = ((i / 2 - 1) / 2) * 2;
			if (compare(data[i], data[p])) {
				std::swap(data[i], data[p]);
				up_min(p);
			} else if (compare(data[p + 1], data[i])) {
				std::swap(data[i], data[p + 1]);
				up_max(p + 1);
			}
		}
	}
	T pop_min() {
		assert(!empty());
		T result = std::move(data[0]);
		if (data.size() == 1) data.pop_back();
		else {
			T value = std::move(data.back());
			data.pop_back();
			down_min(0, std::move(value));
		}
		return result;
	}
	T pop_max() {
		assert(!empty());
		if (data.size() == 1) return pop_min();
		T result = std::move(data[1]);
		if (data.size() == 2) data.pop_back();
		else {
			T value = std::move(data.back());
			data.pop_back();
			down_max(1, std::move(value));
		}
		return result;
	}
	void clear() { data.clear(); }
	void swap(DoubleEndedPriorityQueue& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		using std::swap;
		data.swap(other.data);
		swap(compare, other.compare);
	}
	friend void swap(DoubleEndedPriorityQueue& a, DoubleEndedPriorityQueue& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

#endif
