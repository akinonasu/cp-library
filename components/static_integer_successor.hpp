#ifndef STATIC_INTEGER_SUCCESSOR_HPP
#define STATIC_INTEGER_SUCCESSOR_HPP

#include "splitmix64.hpp"
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

namespace static_integer_successor_detail {
class Index {
	struct Key { std::uint64_t prefix, group_length; bool operator==(const Key &) const = default; };
	struct Hash {
		std::uint64_t seed;
		Hash() noexcept : seed(std::chrono::steady_clock::now().time_since_epoch().count()) {}
		std::size_t operator()(Key k) const noexcept { return splitmix64(k.prefix ^ seed) ^ splitmix64(k.group_length + seed); }
	};
	struct Range { int first = -1, last = -1; };
	std::unordered_map<Key, Range, Hash> prefixes_;
	struct Directory { std::uint64_t low; unsigned shift; std::vector<int> offsets; };
	std::unordered_map<unsigned, Directory> directories_;
	static int common(std::uint64_t a, std::uint64_t b, int bits) { return std::countl_zero(a ^ b) - (64 - bits); }
  public:
	std::uint64_t add(unsigned group, std::span<const std::uint64_t> keys, int bits) {
		if (keys.size() <= std::size_t(bits)) return 0;
		std::uint64_t stride = (keys.back() - keys.front()) / (keys.size() - 1);
		bool arithmetic = stride && (keys.back() - keys.front()) % (keys.size() - 1) == 0;
		for (std::size_t i = 1; arithmetic && i < keys.size(); ++i) arithmetic = keys[i] - keys[i - 1] == stride;
		if (arithmetic) return stride;
		int samples = int((keys.size() + bits - 1) / bits);
		unsigned width = std::bit_width(keys.back() - keys.front());
		unsigned shift = width - std::min<unsigned>(width, std::bit_width(unsigned(samples)));
		unsigned buckets = unsigned(((keys.back() - keys.front()) >> shift) + 1);
		Directory directory{keys.front(), shift, std::vector<int>(std::size_t(buckets) + 1)};
		std::size_t cursor = 0;
		bool large_bucket = false;
		for (unsigned bucket = 0; bucket <= buckets; ++bucket) {
			while (cursor < keys.size() && ((keys[cursor] - keys.front()) >> shift) < bucket) ++cursor;
			directory.offsets[bucket] = int(cursor);
			if (bucket && directory.offsets[bucket] - directory.offsets[bucket - 1] > 4 * bits) large_bucket = true;
		}
		directories_.emplace(group, std::move(directory));
		if (!large_bucket) return 0;
		auto sample = [&](int i) { return keys[std::min(keys.size(), std::size_t(i + 1) * bits) - 1]; };
		int first_length = common(sample(0), sample(samples - 1), bits) + 1;
		for (int i = 0; i < samples; ++i) {
			auto key = sample(i);
			for (int length = first_length; length <= bits; ++length) {
				auto &range = prefixes_[Key{key >> (bits - length), (std::uint64_t(group) << 7) | unsigned(length)}];
				if (range.first == -1) range.first = i;
				range.last = i;
			}
		}
		return 0;
	}
	template <bool Strict> int lower_bound(unsigned group, std::span<const std::uint64_t> keys, int bits, std::uint64_t stride, std::uint64_t x) const {
		int n = int(keys.size());
		if (!n || (Strict ? x >= keys.back() : x > keys.back())) return n;
		if (x < keys.front() || (!Strict && x == keys.front())) return 0;
		if (stride) {
			auto difference = x - keys.front();
			return int(difference / stride + (Strict || difference % stride != 0));
		}
		int begin = 0, end = n;
		if (n > bits) {
			const auto &directory = directories_.at(group);
			unsigned bucket = unsigned((x - directory.low) >> directory.shift);
			begin = directory.offsets[bucket]; end = directory.offsets[bucket + 1];
			if (end - begin <= 4 * bits) {
				while (begin < end) { int mid = begin + (end - begin) / 2; if (Strict ? keys[mid] <= x : keys[mid] < x) begin = mid + 1; else end = mid; }
				return begin;
			}
			int samples = (n - 1) / bits + 1;
			auto sample = [&](int i) { return keys[std::min(keys.size(), std::size_t(i + 1) * bits) - 1]; };
			int chosen = 0;
			if (Strict ? x >= sample(0) : x > sample(0)) {
				int length = common(sample(0), sample(samples - 1), bits), upper = bits + 1;
				Range range{0, samples - 1};
				while (length + 1 < upper) {
					int mid = length + (upper - length) / 2;
					auto it = prefixes_.find(Key{x >> (bits - mid), (std::uint64_t(group) << 7) | unsigned(mid)});
					if (it == prefixes_.end()) upper = mid;
					else { length = mid; range = it->second; }
				}
				if (length == bits) chosen = range.first + Strict;
				else chosen = ((x >> (bits - length - 1)) & 1) ? range.last + 1 : range.first;
			}
			begin = chosen * bits;
			end = begin + std::min(bits, n - begin);
		}
		while (begin < end) { int mid = begin + (end - begin) / 2; if (Strict ? keys[mid] <= x : keys[mid] < x) begin = mid + 1; else end = mid; }
		return begin;
	}
};
}

#endif
