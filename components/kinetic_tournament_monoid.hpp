#ifndef KINETIC_TOURNAMENT_MONOID_HPP
#define KINETIC_TOURNAMENT_MONOID_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace kinetic_tournament_detail {
template<class T, bool maximize> struct Monoid {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using W = __int128_t;
	using U = std::uint64_t;
	static constexpr U never = std::numeric_limits<U>::max();
	struct Node { T value = 0, slope = 0; U remaining = never; int index = -1; };
	struct Tag { W steps = 0, add = 0; };
	static Node identity() { return {}; }
	static Tag tag_identity() { return {}; }
	static bool better(const Node &a, const Node &b) {
		if (a.index == -1) return false;
		if (b.index == -1) return true;
		if (a.value != b.value) return maximize ? a.value > b.value : a.value < b.value;
		return maximize ? a.index > b.index : a.index < b.index;
	}
	static Node op(Node a, Node b) {
		if (a.index == -1) return b;
		if (b.index == -1) return a;
		bool left = better(a, b);
		Node result = left ? a : b;
		result.remaining = std::min(a.remaining, b.remaining);
		if (left == maximize && a.slope < b.slope) {
			U gap = U(a.value) - U(b.value);
			U difference = U(b.slope) - U(a.slope);
			assert(gap > 0);
			result.remaining = std::min(result.remaining, (gap - 1) / difference);
		}
		return result;
	}
	static bool mapping(Tag tag, Node &node) {
		if (node.index == -1) return true;
		if (node.remaining != never && tag.steps > W(node.remaining)) return false;
		W value = W(node.value) + tag.steps * node.slope + tag.add;
		assert(std::numeric_limits<T>::lowest() <= value && value <= std::numeric_limits<T>::max());
		node.value = T(value);
		if (node.remaining != never) node.remaining -= U(tag.steps);
		return true;
	}
	static Tag composition(Tag a, Tag b) { return {a.steps + b.steps, a.add + b.add}; }
};
}

#endif
