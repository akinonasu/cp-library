#ifndef DIJKSTRA_QUEUE_HPP
#define DIJKSTRA_QUEUE_HPP

#include "RadixHeap.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace shortest_path_detail {
template<class T> class RadixQueue {
	static_assert(std::is_integral_v<T> && sizeof(T) <= 8);
	struct State { T key; int vertex; };
	State small_[32];
	std::vector<std::uint64_t> buckets_[33];
	std::uint32_t minimum_[33];
	std::uint32_t occupied_ = 0;
	std::uint64_t last_ = 0;
	std::size_t count_ = 0;
	int mode_ = 0;
	RadixHeap<std::uint64_t> wide_;
	void put(std::uint64_t state) {
		std::uint32_t key = state >> 32;
		int index = std::bit_width(key ^ std::uint32_t(last_));
		buckets_[index].push_back(state);
		if (index) {
			occupied_ |= std::uint32_t(1) << (index - 1);
			minimum_[index] = std::min(minimum_[index], key);
		}
	}
	void promote() {
		if (mode_ == 0) {
			for (std::size_t i = 0; i < count_; ++i) wide_.push({std::uint64_t(small_[i].key), small_[i].vertex});
		} else {
			for (auto &bucket : buckets_) {
				for (auto state : bucket) {
					std::uint64_t key = last_ + std::uint32_t(std::uint32_t(state >> 32) - std::uint32_t(last_));
					wide_.push({key, int(std::uint32_t(state))});
				}
				bucket.clear();
			}
		}
		mode_ = 2;
	}

  public:
	bool empty() const { return count_ == 0; }
	void push(T key, int vertex) {
		assert(key >= 0 && std::uint64_t(key) >= last_);
		if (mode_ == 0 && count_ < 32) {
			small_[count_++] = {key, vertex};
			return;
		}
		if (mode_ == 0) {
			bool narrow = std::uint64_t(key) - last_ <= std::uint64_t(std::numeric_limits<std::int32_t>::max());
			for (std::size_t i = 0; i < count_; ++i)
				narrow &= std::uint64_t(small_[i].key) - last_ <= std::uint64_t(std::numeric_limits<std::int32_t>::max());
			if (narrow) {
				std::fill_n(minimum_, 33, std::numeric_limits<std::uint32_t>::max());
				for (std::size_t i = 0; i < count_; ++i)
					put((std::uint64_t(std::uint32_t(small_[i].key)) << 32) | std::uint32_t(small_[i].vertex));
				mode_ = 1;
			} else promote();
		}
		if (mode_ == 1 && std::uint64_t(key) - last_ > std::uint64_t(std::numeric_limits<std::int32_t>::max())) promote();
		if (mode_ == 2) wide_.push({std::uint64_t(key), vertex});
		else put((std::uint64_t(std::uint32_t(key)) << 32) | std::uint32_t(vertex));
		++count_;
	}
	std::pair<T, int> pop() {
		assert(count_);
		if (mode_ == 0) {
			std::size_t best = 0;
			for (std::size_t i = 1; i < count_; ++i)
				if (small_[i].key < small_[best].key) best = i;
			State state = small_[best];
			small_[best] = small_[--count_];
			last_ = std::uint64_t(state.key);
			return {state.key, state.vertex};
		}
		if (mode_ == 2) {
			auto state = wide_.pop_value();
			last_ = state.first;
			--count_;
			return {T(state.first), state.second};
		}
		if (buckets_[0].empty()) {
			int index = std::countr_zero(occupied_) + 1;
			last_ += std::uint32_t(minimum_[index] - std::uint32_t(last_));
			minimum_[index] = std::numeric_limits<std::uint32_t>::max();
			occupied_ &= occupied_ - 1;
			for (auto state : buckets_[index]) put(state);
			buckets_[index].clear();
		}
		auto state = buckets_[0].back();
		buckets_[0].pop_back();
		--count_;
		return {T(last_), int(std::uint32_t(state))};
	}
};
}

#endif
