#include "components/range_add_min_node.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

namespace {
using T = std::int32_t;
using U = std::uint32_t;
using Node = range_add_min_detail::Node<T>;
constexpr T low = std::numeric_limits<T>::min(), high = std::numeric_limits<T>::max();
constexpr U umax = std::numeric_limits<U>::max();

struct Block {
	alignas(32) std::array<U, 9> storage{};
	U* data() { return storage.data() + 1; }
};

void single_block() {
	Block block;
	U* values = block.data();
	std::fill(values, values + 8, umax);
	std::fill(values, values + 3, 0);
	U change = Node::normalize<false>(values, 3, true, 0, 1, 1);
	assert(change == 1);
	assert(values[0] == 0 && values[1] == umax && values[2] == umax);
	for (unsigned i = 3; i < 8; ++i) assert(values[i] == umax);
	assert(std::bit_cast<T>(U(U(high) + change)) == low);
	change = Node::normalize<false>(values, 3, false, 0, 1, umax);
	assert(change == umax);
	for (unsigned i = 0; i < 3; ++i) assert(values[i] == 0);
	for (unsigned i = 3; i < 8; ++i) assert(values[i] == umax);
	assert(std::bit_cast<T>(U(U(low) + change)) == high);
	std::fill(values, values + 8, 0);
	change = Node::normalize<false>(values, 8, true, 0, 3, U(-2), 1, U(-3), 1, U(-4));
	assert(change == U(-9));
	assert(values[0] == 7 && values[1] == 0 && values[2] == 7);
	for (unsigned i = 3; i < 8; ++i) assert(values[i] == 9);
	change = Node::normalize<false>(values, 8, false, 0, 3, 2, 1, 3, 1, 4);
	assert(change == 9);
	for (unsigned i = 0; i < 8; ++i) assert(values[i] == 0);
	constexpr std::array<U, 8> probes{0x80000000u, umax, 0x80000001u, 0, 0x7fffffffu, 1, 0x90000000u, 7};
	for (unsigned count = 1; count <= 8; ++count) {
		for (unsigned i = 0; i < 8; ++i) values[i] = i < count ? probes[i] : umax;
		for (unsigned l = 0; l <= count; ++l) for (unsigned r = l; r <= count; ++r) {
			U expected = l == r ? umax : *std::min_element(values + l, values + r);
			assert(Node::minimum<false>(values, l, r) == expected);
			assert(Node::minimum<true>(values, l, r) == expected);
		}
		assert(Node::full_minimum<false>(values) == *std::min_element(values, values + 8));
		std::array<U, 8> old;
		std::copy(values, values + 8, old.begin());
		Node::subtract(values, count, umax);
		for (unsigned i = 0; i < 8; ++i) assert(values[i] == (i < count ? U(old[i] - umax) : old[i]));
	}
	assert(block.storage[0] == 0);
}

void paired_blocks() {
	std::mt19937 random(19260817);
	for (unsigned count = 1; count <= 8; ++count) for (unsigned first = 0; first < 8; ++first)
		for (unsigned last = 0; last < count; ++last) for (bool negative : {false, true}) for (int trial = 0; trial < 16; ++trial) {
			Block left_block, right_block;
			U* left = left_block.data();
			U* right = right_block.data();
			std::array<std::int64_t, 8> x{}, y{};
			T left_root = std::bit_cast<T>(U(random())), right_root = std::bit_cast<T>(U(random()));
			for (unsigned i = 0; i < 8; ++i) {
				std::uint64_t span = std::uint64_t(std::int64_t(high) - left_root);
				left[i] = U(std::uint64_t(random()) % (span + 1));
				span = std::uint64_t(std::int64_t(high) - right_root);
				right[i] = i < count ? U(std::uint64_t(random()) % (span + 1)) : umax;
			}
			left[random() % 8] = 0;
			right[random() % count] = 0;
			for (unsigned i = 0; i < 8; ++i) {
				x[i] = std::int64_t(left_root) + left[i];
				if (i < count) y[i] = std::int64_t(right_root) + right[i];
			}
			auto available = [&](std::int64_t value) { return std::uint64_t(negative ? value - low : high - value); };
			std::uint64_t common = umax;
			for (unsigned i = first + 1; i < 8; ++i) common = std::min(common, available(x[i]));
			for (unsigned i = 0; i < last; ++i) common = std::min(common, available(y[i]));
			common = std::uint64_t(random()) % (common + 1);
			std::uint64_t lc = std::uint64_t(random()) % (available(x[first]) + 1);
			std::uint64_t rc = std::uint64_t(random()) % (available(y[last]) + 1);
			if (trial % 4 == 0) common = 0;
			if (trial % 4 == 1) lc = 0;
			if (trial % 4 == 2) rc = 0;
			std::int64_t direction = negative ? -1 : 1;
			for (unsigned i = first + 1; i < 8; ++i) x[i] += direction * std::int64_t(common);
			for (unsigned i = 0; i < last; ++i) y[i] += direction * std::int64_t(common);
			x[first] += direction * std::int64_t(lc);
			y[last] += direction * std::int64_t(rc);
			for (unsigned i = 0; i < 8; ++i) assert(low <= x[i] && x[i] <= high);
			for (unsigned i = 0; i < count; ++i) assert(low <= y[i] && y[i] <= high);
			std::int64_t xmin = *std::min_element(x.begin(), x.end());
			std::int64_t ymin = *std::min_element(y.begin(), y.begin() + count);
			auto encode = [&](std::uint64_t value) { return negative ? U(U(0) - U(value)) : U(value); };
			auto change = Node::normalize_boundaries<false>(left, right, count, negative, first, last, encode(common), encode(lc), encode(rc));
			assert(change.first == U(U(xmin) - U(left_root)) && change.second == U(U(ymin) - U(right_root)));
			for (unsigned i = 0; i < 8; ++i) assert(left[i] == U(x[i] - xmin));
			for (unsigned i = 0; i < count; ++i) assert(right[i] == U(y[i] - ymin));
			for (unsigned i = count; i < 8; ++i) assert(right[i] == umax);
			assert(left_block.storage[0] == 0 && right_block.storage[0] == 0);
		}
}

template<class Value> void materialize() {
	using V = std::make_unsigned_t<Value>;
	using Part = range_add_min_detail::Node<Value>;
	const V maximum = std::numeric_limits<V>::max(), sign = V(V(1) << (sizeof(Value) * 8 - 1));
	const std::array<V, 7> probes{0, 1, V(sign - 1), sign, V(sign + 1), V(maximum - 1), maximum};
	std::mt19937_64 random(128576 + sizeof(Value));
	for (unsigned count = 0; count <= 8; ++count) for (V base : probes) for (unsigned trial = 0; trial < 16; ++trial) {
		alignas(64) std::array<V, 10> source{};
		alignas(64) std::array<Value, 10> result{};
		for (unsigned i = 0; i < source.size(); ++i) {
			source[i] = trial < probes.size() ? probes[(i + trial) % probes.size()] : V(random());
			result[i] = std::bit_cast<Value>(V(random()));
		}
		source.front() = V(0xa5), source.back() = V(0x5a);
		result.front() = std::bit_cast<Value>(V(0xc3)), result.back() = std::bit_cast<Value>(V(0x3c));
		const auto old_source = source;
		auto expected = result;
		for (unsigned i = 0; i < count; ++i) expected[i + 1] = std::bit_cast<Value>(V(source[i + 1] + base));
		Part::materialize(source.data() + 1, result.data() + 1, count, base);
		assert(source == old_source && result == expected);
		if (!count) Part::materialize(nullptr, nullptr, 0, base);
	}
}

template<class Value> void build_blocks() {
	using V = std::make_unsigned_t<Value>;
	using Part = range_add_min_detail::Node<Value>;
	const Value low = std::numeric_limits<Value>::min(), high = std::numeric_limits<Value>::max();
	const V maximum = std::numeric_limits<V>::max();
	std::mt19937_64 random(674319 + sizeof(Value));
	for (unsigned count = 0; count <= 8; ++count) for (unsigned trial = 0; trial < 32; ++trial) {
		alignas(64) std::array<V, 10> values;
		values.fill(maximum);
		values.front() = V(0xa5), values.back() = V(0x5a);
		for (unsigned i = 0; i < count; ++i) {
			if (trial == 0) values[i + 1] = V(high);
			else if (trial == 1) values[i + 1] = i % 2 ? V(high) : V(low);
			else if (trial == 2) values[i + 1] = V(i + 1);
			else if (trial == 3) values[i + 1] = V(low);
			else if (trial == 4) values[i + 1] = maximum;
			else values[i + 1] = V(random());
		}
		const auto original = values;
		Value minimum = high;
		for (unsigned i = 0; i < count; ++i) minimum = std::min(minimum, std::bit_cast<Value>(original[i + 1]));
		auto expected = original;
		for (unsigned i = 0; i < count; ++i) expected[i + 1] = V(original[i + 1] - V(minimum));
		Value result = Part::build(values.data() + 1, count);
		assert(result == minimum && values == expected);
		alignas(64) std::array<Value, 10> restored;
		restored.fill(std::bit_cast<Value>(V(0x3c)));
		auto expected_restored = restored;
		for (unsigned i = 0; i < count; ++i) expected_restored[i + 1] = std::bit_cast<Value>(original[i + 1]);
		Part::materialize(values.data() + 1, restored.data() + 1, count, V(result));
		assert(restored == expected_restored && values == expected);
		if (!count) assert(Part::build(nullptr, 0) == high);
	}
}
}

int main() {
	single_block();
	paired_blocks();
	materialize<std::int8_t>();
	materialize<std::int16_t>();
	materialize<std::int32_t>();
	materialize<std::int64_t>();
	build_blocks<std::int8_t>();
	build_blocks<std::int16_t>();
	build_blocks<std::int32_t>();
	build_blocks<std::int64_t>();
}
