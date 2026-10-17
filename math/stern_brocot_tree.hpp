#ifndef STERN_BROCOT_TREE_HPP
#define STERN_BROCOT_TREE_HPP

#include "../components/stern_brocot_path.hpp"
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template <std::integral Int = std::uint64_t>
requires (sizeof(Int) <= 8 && !std::is_same_v<Int, bool>)
class SternBrocotTree {
public:
	using value_type = std::pair<Int, Int>;
	using count_type = std::make_unsigned_t<Int>;
	using distance_type = std::conditional_t<(sizeof(Int) <= 4), std::uint64_t, __uint128_t>;
	struct Run {
		char direction;
		count_type count;
		bool operator==(const Run &) const = default;
	};
	using path_type = std::vector<Run>;
private:
	using U = count_type;
	using W = distance_type;
	using Path = stern_brocot_detail::Path<U>;
	Int ln = 0, ld = 1, rn = 1, rd = 0;
	U level = 0;
	void advance(bool right, U k) {
		if (right) { ln += k * U(rn); ld += k * U(rd); }
		else { rn += k * U(ln); rd += k * U(ld); }
		level += k;
	}
	bool move(bool right, U k) {
		W n = W(numerator()) + W(k) * U(right ? rn : ln);
		W d = W(denominator()) + W(k) * U(right ? rd : ld);
		if (n > W(std::numeric_limits<Int>::max()) || d > W(std::numeric_limits<Int>::max())) return false;
		advance(right, k); return true;
	}
public:
	SternBrocotTree() = default;
	SternBrocotTree(Int numerator, Int denominator) {
		assert(numerator > 0 && denominator > 0);
		Path path{U(numerator), U(denominator)};
		while (!path.empty()) { bool right = path.right(); advance(right, path.pop()); }
	}
	explicit SternBrocotTree(value_type value) : SternBrocotTree(value.first, value.second) {}
	Int numerator() const { return ln + rn; }
	Int denominator() const { return ld + rd; }
	value_type value() const { return {numerator(), denominator()}; }
	value_type lower_bound() const { return {ln, ld}; }
	value_type upper_bound() const { return {rn, rd}; }
	std::pair<value_type, value_type> range() const { return {lower_bound(), upper_bound()}; }
	U depth() const { return level; }
	bool is_root() const { return level == 0; }
	bool operator==(const SternBrocotTree &) const = default;
	bool move_left(U k = 1) { return move(false, k); }
	bool move_right(U k = 1) { return move(true, k); }
	bool move_up(U k = 1) {
		if (k > level) return false;
		level -= k;
		while (k) {
			bool right = ln >= rn;
			U step = k == 1 ? 1 : std::min<U>(k, right ? U(ln) / U(rn) : U(rd) / U(ld));
			if (right) { ln -= step * U(rn); ld -= step * U(rd); }
			else { rn -= step * U(ln); rd -= step * U(ld); }
			k -= step;
		}
		return true;
	}
	std::optional<SternBrocotTree> left(U k = 1) const {
		auto result = *this;
		if (!result.move_left(k)) return std::nullopt;
		return result;
	}
	std::optional<SternBrocotTree> right(U k = 1) const {
		auto result = *this;
		if (!result.move_right(k)) return std::nullopt;
		return result;
	}
	std::optional<SternBrocotTree> parent() const {
		auto result = *this;
		if (!result.move_up()) return std::nullopt;
		return result;
	}
	bool contains(value_type x) const {
		assert(x.first > 0 && x.second > 0);
		return W(ln) * U(x.second) < W(ld) * U(x.first) && W(x.first) * U(rd) < W(x.second) * U(rn);
	}
	bool is_ancestor(const SternBrocotTree &other) const { return contains(other.value()); }
	path_type path() const { return encode(numerator(), denominator()); }
	template <class Emit>
	static void encode(Int numerator, Int denominator, Emit &&emit) {
		assert(numerator > 0 && denominator > 0);
		Path path{U(numerator), U(denominator)};
		while (!path.empty()) { char direction = path.right() ? 'R' : 'L'; emit(direction, path.pop()); }
	}
	static path_type encode(Int numerator, Int denominator) {
		path_type result;
		encode(numerator, denominator, [&](char direction, U count) { result.push_back({direction, count}); });
		return result;
	}
	static std::optional<SternBrocotTree> decode(std::span<const Run> path) {
		SternBrocotTree result;
		for (auto [direction, count] : path) {
			if (direction != 'L' && direction != 'R') return std::nullopt;
			if (!result.move(direction == 'R', count)) return std::nullopt;
		}
		return result;
	}
	static std::optional<SternBrocotTree> ancestor(value_type x, U depth) {
		assert(x.first > 0 && x.second > 0);
		SternBrocotTree result;
		Path path{U(x.first), U(x.second)};
		while (depth && !path.empty()) {
			bool right = path.right();
			U step = std::min(depth, path.pop());
			result.advance(right, step); depth -= step;
		}
		if (depth) return std::nullopt;
		return result;
	}
	std::optional<SternBrocotTree> ancestor(U depth) const {
		if (depth > level) return std::nullopt;
		return ancestor(value(), depth);
	}
	static SternBrocotTree lca(value_type x, value_type y) {
		assert(x.first > 0 && x.second > 0 && y.first > 0 && y.second > 0);
		SternBrocotTree result;
		Path a{U(x.first), U(x.second)}, b{U(y.first), U(y.second)};
		while (!a.empty() && !b.empty() && a.right() == b.right()) {
			bool right = a.right();
			U k = a.pop(), l = b.pop();
			result.advance(right, std::min(k, l));
			if (k != l) break;
		}
		return result;
	}
	SternBrocotTree lca(const SternBrocotTree &other) const { return lca(value(), other.value()); }
	W distance(const SternBrocotTree &other) const { return W(level) + other.level - 2 * W(lca(other).level); }
};

#endif
