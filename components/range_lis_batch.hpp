#ifndef RANGE_LIS_BATCH_HPP
#define RANGE_LIS_BATCH_HPP

#include <algorithm>
#include <cstdint>
#include <functional>
#include <span>
#include <utility>
#include <vector>

namespace range_lis_detail {
inline void replace_max(std::uint32_t *heap, int size, std::uint32_t value) {
	int at = 0;
	while (at * 2 + 1 < size) {
		int child = at * 2 + 1;
		if (child + 1 < size && heap[child] < heap[child + 1]) ++child;
		if (value >= heap[child]) break;
		heap[at] = heap[child];
		at = child;
	}
	heap[at] = value;
}
inline void replace_min(std::uint32_t *heap, int size, std::uint32_t value) {
	int at = 0;
	while (at * 2 + 1 < size) {
		int child = at * 2 + 1;
		if (child + 1 < size && heap[child] > heap[child + 1]) ++child;
		if (value <= heap[child]) break;
		heap[at] = heap[child];
		at = child;
	}
	heap[at] = value;
}
inline void batch_sqrt(std::span<const int> permutation, std::span<const std::pair<int, int>> intervals, std::vector<int> &answers) {
	int n = int(permutation.size());
	std::vector<int> offset(std::size_t(n) + 2);
	for (auto [l, r] : intervals) if (r > l) ++offset[std::size_t(r) + 1];
	for (std::size_t i = 1; i < offset.size(); ++i) offset[i] += offset[i - 1];
	std::vector<std::pair<int, int>> ordered(offset.back());
	auto cursor = offset;
	for (int i = 0; i < int(intervals.size()); ++i) {
		auto [l, r] = intervals[i];
		if (r > l) ordered[cursor[r]++] = {l, i};
	}
	int block = 1;
	while (block < n / block) ++block;
	int count = n ? (n - 1) / block + 1 : 0;
	std::vector<int> sizes(count), fenwick(std::size_t(n) + 1);
	std::vector<std::uint32_t> values(n), heap(n), pending(block);
	std::vector<bool> dirty(count);
	auto update = [&](unsigned at, int delta) {
		for (; at; at &= at - 1) fenwick[at] += delta;
	};
	for (int i = 0; i < n; ++i) {
		int x = permutation[i], part = x / block;
		int left = part * block, right = left + std::min(block, n - left);
		auto *local = heap.data() + left;
		if (dirty[part]) {
			int used = 0;
			for (int j = 0; j < sizes[part]; ++j) if (local[j] & 1) pending[used++] = local[j] >> 1;
			std::make_heap(pending.begin(), pending.begin() + used, std::greater<>{});
			for (int j = left; j < right; ++j) if (used && values[j] > pending[0]) {
				auto value = values[j];
				values[j] = pending[0];
				replace_min(pending.data(), used, value);
			}
			dirty[part] = false;
		}
		values[x] = unsigned(i) + 1;
		update(unsigned(i) + 1, 1);
		std::uint32_t carry = 0;
		for (int j = x + 1; j < right; ++j) if (values[j] > carry) std::swap(values[j], carry);
		for (int j = part + 1; j < count; ++j) {
			auto *other = heap.data() + j * block;
			if (sizes[j] && carry < (other[0] >> 1)) {
				dirty[j] = true;
				auto next = other[0] >> 1;
				replace_max(other, sizes[j], (carry << 1) | 1);
				carry = next;
			}
		}
		update(carry, -1);
		int used = 0;
		for (int j = left; j < right; ++j) if (values[j]) local[used++] = values[j] << 1;
		sizes[part] = used;
		std::make_heap(local, local + used);
		for (int j = offset[std::size_t(i) + 1]; j < offset[std::size_t(i) + 2]; ++j) {
			auto [l, id] = ordered[j];
			int answer = 0;
			for (unsigned at = unsigned(l) + 1; at <= unsigned(n); at += at & -at) answer += fenwick[at];
			answers[id] = answer;
		}
	}
}
}

#endif
