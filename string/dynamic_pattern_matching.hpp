#ifndef DYNAMIC_PATTERN_MATCHING_HPP
#define DYNAMIC_PATTERN_MATCHING_HPP

#include "../components/persistent_rope.hpp"
#include "../components/suffix_range.hpp"
#include <memory>
#include <string>

class DynamicPatternMatching {
	struct Piece { SuffixRangeIndex::Range range; int start = 0; };
	static std::uint64_t weight(const Piece &piece) { return piece.range.length; }
	using Rope = PersistentRope<Piece, weight>;
	struct State {
		std::shared_ptr<const SuffixRangeIndex> index;
		Rope rope;
		explicit State(std::shared_ptr<const SuffixRangeIndex> index) : index(std::move(index)) {}
	};
	std::shared_ptr<State> state_;
	int root_ = 0;
	Rope &rope() const { return state_->rope; }
	const SuffixRangeIndex &index() const { return *state_->index; }
	Piece character(char c) const {
		auto range = index().character(c);
		return {range, range.left < range.right ? index().suffix_array()[range.left] : -int(static_cast<unsigned char>(c)) - 1};
	}
	Piece subpiece(Piece piece, int l, int r) const {
		return {index().substring(piece.range, l, r), piece.start + l};
	}
	int concat(int a, int b) {
		if (!a || !b) return a ? a : b;
		auto x = rope().back(a), y = rope().front(b);
		auto range = index().concat(x.range, y.range);
		if (range.left == range.right) return rope().join(a, b);
		int left = rope().pop_back(a).first, right = rope().pop_front(b).second;
		int middle = rope().make(Piece{range, index().suffix_array()[range.left]});
		return rope().join(rope().join(left, middle), right);
	}
	std::pair<int, int> split(int root, std::uint64_t p) {
		bool divided = false;
		auto result = rope().split(root, p, [&](Piece piece, std::uint64_t offset) {
			divided = true;
			int k = static_cast<int>(offset);
			return std::pair{subpiece(piece, 0, k), subpiece(piece, k, piece.range.length)};
		});
		if (!divided) return result;
		auto &[a, b] = result;
		if (a && !rope().leaf(a)) {
			auto [rest, last] = rope().pop_back(a);
			a = concat(rest, rope().make(last));
		}
		if (b && !rope().leaf(b)) {
			auto [first, rest] = rope().pop_front(b);
			b = concat(rope().make(first), rest);
		}
		return result;
	}
	int make(std::string_view pattern) {
		std::vector<Piece> pieces;
		while (!pattern.empty()) {
			auto [range, length] = index().longest_prefix(pattern);
			if (!length) { pieces.push_back(character(pattern.front())); length = 1; }
			else pieces.push_back({range, index().suffix_array()[range.left]});
			pattern.remove_prefix(length);
		}
		return rope().build(pieces);
	}
	void check_range([[maybe_unused]] std::uint64_t l, [[maybe_unused]] std::uint64_t r) const { assert(l <= r && r <= size()); }
	void check_growth([[maybe_unused]] std::uint64_t length) const { assert(length <= std::numeric_limits<std::uint64_t>::max() - size()); }

  public:
	using size_type = std::uint64_t;
	DynamicPatternMatching() : DynamicPatternMatching(std::string_view{}) {}
	explicit DynamicPatternMatching(std::string_view text, std::string_view pattern = {})
		: state_(std::make_shared<State>(std::make_shared<const SuffixRangeIndex>(text))) { root_ = make(pattern); }
	DynamicPatternMatching(const DynamicPatternMatching &) = default;
	DynamicPatternMatching &operator=(const DynamicPatternMatching &) = default;
	DynamicPatternMatching(DynamicPatternMatching &&other) : DynamicPatternMatching() { swap(other); }
	DynamicPatternMatching &operator=(DynamicPatternMatching &&other) {
		if (this != &other) { DynamicPatternMatching next(std::move(other)); swap(next); }
		return *this;
	}
	void build(std::string_view text, std::string_view pattern = {}) { DynamicPatternMatching next(text, pattern); swap(next); }
	void swap(DynamicPatternMatching &other) noexcept { state_.swap(other.state_); std::swap(root_, other.root_); }
	void assign(std::string_view pattern) { root_ = make(pattern); }
	void clear() { root_ = 0; }
	size_type size() const { return rope().size(root_); }
	bool empty() const { return root_ == 0; }
	std::string_view text() const { return index().text(); }
	std::size_t storage_nodes() const { return rope().storage_nodes(); }
	void compact() {
		auto next = std::make_shared<State>(state_->index);
		int root = next->rope.copy_from(rope(), root_);
		state_ = std::move(next); root_ = root;
	}
	void insert(size_type p, char c) {
		assert(p <= size()); check_growth(1);
		auto [a, b] = split(root_, p);
		int middle = rope().make(character(c));
		root_ = concat(concat(a, middle), b);
	}
	void insert(size_type p, std::string_view pattern) {
		assert(p <= size()); check_growth(pattern.size());
		if (pattern.empty()) return;
		int middle = make(pattern);
		auto [a, b] = split(root_, p);
		root_ = concat(concat(a, middle), b);
	}
	void erase(size_type p) { assert(p < size()); erase(p, p + 1); }
	void erase(size_type l, size_type r) {
		check_range(l, r);
		if (l == r) return;
		auto [a, rest] = split(root_, l);
		int b = split(rest, r - l).second;
		root_ = concat(a, b);
	}
	void set(size_type p, char c) { assert(p < size()); erase(p); insert(p, c); }
	void move(size_type l, size_type r, size_type p) {
		check_range(l, r); assert(p <= size() - (r - l));
		if (l == r || p == l) return;
		auto [a, rest] = split(root_, l);
		auto [middle, b] = split(rest, r - l);
		int remaining = concat(a, b);
		auto [left, right] = split(remaining, p);
		root_ = concat(concat(left, middle), right);
	}
	void copy(size_type l, size_type r, size_type p) {
		check_range(l, r); assert(p <= size()); check_growth(r - l);
		if (l == r) return;
		int rest = split(root_, l).second, middle = split(rest, r - l).first;
		auto [a, b] = split(root_, p);
		root_ = concat(concat(a, middle), b);
	}
	std::pair<int, int> occurrence_range() const {
		if (!root_) return {0, index().size() + 1};
		if (!rope().leaf(root_)) return {0, 0};
		auto range = rope().value(root_).range;
		return {range.left, range.right};
	}
	int count() const { auto [l, r] = occurrence_range(); return r - l; }
	int occurrence(int i) const { auto [l, r] = occurrence_range(); assert(0 <= i && i < r - l); return index().suffix_array()[l + i]; }
	std::vector<int> occurrences() const { auto [l, r] = occurrence_range(); return {index().suffix_array().begin() + l, index().suffix_array().begin() + r}; }
	const std::vector<int> &suffix_array() const { return index().suffix_array(); }
	char operator[](size_type p) const {
		auto [piece, offset] = rope().at(root_, p);
		return piece.start < 0 ? static_cast<char>(-piece.start - 1) : text()[piece.start + offset];
	}
	std::string str() const {
		std::string result;
		assert(size() <= result.max_size());
		result.reserve(static_cast<std::size_t>(size()));
		rope().for_each(root_, [&](Piece piece) {
			if (piece.start < 0) result.push_back(static_cast<char>(-piece.start - 1));
			else result.append(text().substr(piece.start, piece.range.length));
		});
		return result;
	}
};

#endif
