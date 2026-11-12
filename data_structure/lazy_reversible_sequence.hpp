#ifndef LAZY_REVERSIBLE_SEQUENCE_HPP
#define LAZY_REVERSIBLE_SEQUENCE_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <class S, auto op, auto e, class F, auto mapping, auto composition, auto id, bool commutative = false, int BlockSize = 128>
class LazyReversibleSequence {
	static_assert(BlockSize >= 8 && BlockSize <= 65535);
	static constexpr int minimum = BlockSize / 4;
	struct Empty {};
	using Back = std::conditional_t<commutative, Empty, S>;
	using Value = std::conditional_t<std::is_same_v<S, bool>, unsigned char, S>;
	struct Node {
		int left = 0, right = 0, count = 0, height = 0;
		std::uint16_t first = 0, last = 0;
		bool pending = false, reversed = false;
		S product = e();
		[[no_unique_address]] Back back = make_back();
		F lazy = id();
		std::vector<Value> values;
		static Back make_back() {
			if constexpr (commutative) return {};
			else return e();
		}
	};
	std::vector<Node> nodes_;
	std::vector<Value> scratch_;
	std::vector<Value> front_, back_;
	S front_product_ = e(), back_product_ = e();
	int root_ = 0, free_ = 0, buffered_ = 0;
	std::size_t leaf_bytes_ = 0;
	int count(int x) const { return nodes_[x].count; }
	int height(int x) const { return nodes_[x].height; }
	bool leaf(int x) const { return nodes_[x].left == 0; }
	int acquire() {
		if (free_) {
			int x = free_;
			free_ = nodes_[x].left;
			nodes_[x].left = nodes_[x].right = 0;
			nodes_[x].pending = nodes_[x].reversed = false;
			return x;
		}
		assert(nodes_.size() < std::size_t(std::numeric_limits<int>::max()));
		if constexpr (std::is_nothrow_move_constructible_v<Node>) nodes_.emplace_back();
		else {
			auto previous = nodes_.capacity();
			nodes_.emplace_back();
			if (nodes_.capacity() != previous) {
				leaf_bytes_ = 0;
				for (const auto &node : nodes_) leaf_bytes_ += node.values.capacity() * sizeof(Value);
			}
		}
		return int(nodes_.size()) - 1;
	}
	void release(int x) {
		auto &v = nodes_[x];
		v.values.clear();
		v.left = free_, v.right = v.count = v.height = 0;
		v.pending = v.reversed = false;
		free_ = x;
	}
	void pull_leaf(int x) {
		auto &v = nodes_[x];
		v.count = int(v.values.size()), v.height = 1;
		v.first = v.last = std::uint16_t(v.count);
		S product = e();
		for (const auto &value : v.values) product = op(std::move(product), value);
		v.product = std::move(product);
		if constexpr (!commutative) {
			S product = e();
			for (auto it = v.values.rbegin(); it != v.values.rend(); ++it) product = op(std::move(product), *it);
			v.back = std::move(product);
			if (v.reversed) std::swap(v.product, v.back);
		}
		if (v.pending) {
			v.product = mapping(v.lazy, v.product, v.count);
			if constexpr (!commutative) v.back = mapping(v.lazy, v.back, v.count);
		}
	}
	void pull(int x) {
		auto &v = nodes_[x];
		int l = v.left, r = v.right;
		v.count = count(l) + count(r);
		v.height = std::max(height(l), height(r)) + 1;
		v.first = nodes_[l].first, v.last = nodes_[r].last;
		v.product = op(nodes_[l].product, nodes_[r].product);
		if constexpr (!commutative) v.back = op(nodes_[r].back, nodes_[l].back);
	}
	template <class Iterator>
	void fill_leaf(int x, Iterator first, Iterator last) {
		auto &v = nodes_[x];
		auto previous = v.values.capacity();
		v.values.assign(first, last);
		leaf_bytes_ += (v.values.capacity() - previous) * sizeof(Value);
		v.left = v.right = 0, v.pending = v.reversed = false;
		pull_leaf(x);
	}
	template <class Iterator>
	int make_leaf(Iterator first, Iterator last) {
		int x = acquire();
		fill_leaf(x, first, last);
		return x;
	}
	int make_branch(int left, int right) {
		assert(left && right);
		int x = acquire();
		nodes_[x].left = left, nodes_[x].right = right;
		pull(x);
		return x;
	}
	void flip(int x) {
		if (!x || count(x) == 1) return;
		auto &v = nodes_[x];
		if (!leaf(x)) std::swap(v.left, v.right);
		std::swap(v.first, v.last);
		if constexpr (!commutative) std::swap(v.product, v.back);
		v.reversed = !v.reversed;
	}
	void act(int x, const F &f) {
		if (!x) return;
		auto &v = nodes_[x];
		v.product = mapping(f, v.product, v.count);
		if constexpr (!commutative) v.back = mapping(f, v.back, v.count);
		v.lazy = v.pending ? composition(f, v.lazy) : f;
		v.pending = true;
	}
	void push(int x) {
		auto &v = nodes_[x];
		if (leaf(x)) {
			if (v.reversed) std::reverse(v.values.begin(), v.values.end());
			if (v.pending) for (auto &value : v.values) value = mapping(v.lazy, value, 1);
		} else {
			if (v.reversed) flip(v.left), flip(v.right);
			if (v.pending) act(v.left, v.lazy), act(v.right, v.lazy);
		}
		v.reversed = v.pending = false;
	}
	int rotate_left(int x) {
		int y = nodes_[x].right;
		push(y);
		nodes_[x].right = nodes_[y].left;
		nodes_[y].left = x;
		pull(x), pull(y);
		return y;
	}
	int rotate_right(int x) {
		int y = nodes_[x].left;
		push(y);
		nodes_[x].left = nodes_[y].right;
		nodes_[y].right = x;
		pull(x), pull(y);
		return y;
	}
	int balance(int x) {
		pull(x);
		if (height(nodes_[x].left) > height(nodes_[x].right) + 1) {
			int l = nodes_[x].left;
			push(l);
			if (height(nodes_[l].right) > height(nodes_[l].left)) nodes_[x].left = rotate_left(l);
			return rotate_right(x);
		}
		if (height(nodes_[x].right) > height(nodes_[x].left) + 1) {
			int r = nodes_[x].right;
			push(r);
			if (height(nodes_[r].left) > height(nodes_[r].right)) nodes_[x].right = rotate_right(r);
			return rotate_left(x);
		}
		return x;
	}
	int join(int a, int b) {
		if (!a || !b) return a | b;
		if (height(a) > height(b) + 1) {
			push(a);
			int right = join(nodes_[a].right, b);
			nodes_[a].right = right;
			return balance(a);
		}
		if (height(b) > height(a) + 1) {
			push(b);
			int left = join(a, nodes_[b].left);
			nodes_[b].left = left;
			return balance(b);
		}
		return make_branch(a, b);
	}
	std::pair<int, int> split(int x, int position) {
		if (!position) return {0, x};
		if (position == count(x)) return {x, 0};
		push(x);
		if (leaf(x)) {
			int y = acquire();
			fill_leaf(y, std::make_move_iterator(nodes_[x].values.begin() + position), std::make_move_iterator(nodes_[x].values.end()));
			nodes_[x].values.erase(nodes_[x].values.begin() + position, nodes_[x].values.end());
			pull_leaf(x);
			return {x, y};
		}
		int l = nodes_[x].left, r = nodes_[x].right, left = count(l);
		release(x);
		if (position < left) {
			auto [a, b] = split(l, position);
			return {a, join(b, r)};
		}
		if (position == left) return {l, r};
		auto [a, b] = split(r, position - left);
		return {join(l, a), b};
	}
	std::pair<int, int> pop_first(int x) {
		if (leaf(x)) return {0, x};
		push(x);
		int l = nodes_[x].left, r = nodes_[x].right;
		release(x);
		auto [rest, first] = pop_first(l);
		return {join(rest, r), first};
	}
	std::pair<int, int> pop_last(int x) {
		if (leaf(x)) return {0, x};
		push(x);
		int l = nodes_[x].left, r = nodes_[x].right;
		release(x);
		auto [rest, last] = pop_last(r);
		return {join(l, rest), last};
	}
	int concatenate(int a, int b) {
		if (!a || !b) return a | b;
		if (nodes_[a].last >= minimum && nodes_[b].first >= minimum) return join(a, b);
		auto [left, x] = pop_last(a);
		auto [right, y] = pop_first(b);
		int extra = 0;
		bool prepend = false;
		if (count(x) + count(y) < minimum) {
			if (left) {
				auto result = pop_last(left);
				left = result.first, extra = result.second, prepend = true;
			} else if (right) {
				auto result = pop_first(right);
				right = result.first, extra = result.second;
			}
		}
		scratch_.clear();
		auto append = [&](int z) {
			if (!z) return;
			push(z);
			auto &values = nodes_[z].values;
			scratch_.insert(scratch_.end(), std::make_move_iterator(values.begin()), std::make_move_iterator(values.end()));
		};
		if (prepend) append(extra);
		append(x), append(y);
		if (!prepend) append(extra);
		int middle = 0;
		if (int(scratch_.size()) <= BlockSize) {
			fill_leaf(x, std::make_move_iterator(scratch_.begin()), std::make_move_iterator(scratch_.end()));
			release(y);
			middle = x;
		} else {
			auto half = scratch_.begin() + scratch_.size() / 2;
			fill_leaf(x, std::make_move_iterator(scratch_.begin()), std::make_move_iterator(half));
			fill_leaf(y, std::make_move_iterator(half), std::make_move_iterator(scratch_.end()));
			middle = join(x, y);
		}
		if (extra) release(extra);
		scratch_.clear();
		return join(join(left, middle), right);
	}
	void materialize(int x) {
		auto &v = nodes_[x];
		if (v.pending) {
			for (auto &value : v.values) value = mapping(v.lazy, value, 1);
			v.pending = false;
		}
	}
	int insert_node(int x, int p, S value) {
		if (!x) return make_leaf(&value, &value + 1);
		if (leaf(x)) {
			materialize(x);
			int length = count(x), position = nodes_[x].reversed ? length - p : p;
			bool edge = p == 0 || p == length;
			auto previous = nodes_[x].values.capacity();
			nodes_[x].values.insert(nodes_[x].values.begin() + position, std::move(value));
			leaf_bytes_ += (nodes_[x].values.capacity() - previous) * sizeof(Value);
			if (edge || commutative) {
				auto &v = nodes_[x];
				if constexpr (commutative) v.product = op(v.product, v.values[position]);
				else v.product = p ? op(v.product, v.values[position]) : op(v.values[position], v.product);
				if constexpr (!commutative) v.back = p ? op(v.values[position], v.back) : op(v.back, v.values[position]);
				v.count = length + 1, v.first = v.last = std::uint16_t(v.count);
			} else pull_leaf(x);
			if (count(x) <= BlockSize) return x;
			auto [a, b] = split(x, count(x) / 2);
			return join(a, b);
		}
		push(x);
		int left = count(nodes_[x].left);
		if (p <= left) {
			int child = insert_node(nodes_[x].left, p, std::move(value));
			nodes_[x].left = child;
		} else {
			int child = insert_node(nodes_[x].right, p - left, std::move(value));
			nodes_[x].right = child;
		}
		return balance(x);
	}
	int erase_node(int x, int p, S &value) {
		if (leaf(x)) {
			materialize(x);
			if (nodes_[x].reversed) p = count(x) - 1 - p;
			value = std::move(nodes_[x].values[p]);
			nodes_[x].values.erase(nodes_[x].values.begin() + p);
			if (nodes_[x].values.empty()) { release(x); return 0; }
			pull_leaf(x);
			return x;
		}
		push(x);
		int l = nodes_[x].left, r = nodes_[x].right, left = count(l);
		release(x);
		if (p < left) l = erase_node(l, p, value);
		else r = erase_node(r, p - left, value);
		return concatenate(l, r);
	}
	void set_node(int x, int p, S value) {
		if (leaf(x)) {
			materialize(x);
			if (nodes_[x].reversed) p = count(x) - 1 - p;
			nodes_[x].values[p] = std::move(value);
			pull_leaf(x);
			return;
		}
		push(x);
		int left = count(nodes_[x].left);
		if (p < left) set_node(nodes_[x].left, p, std::move(value));
		else set_node(nodes_[x].right, p - left, std::move(value));
		pull(x);
	}
	S fold(int x, int l, int r, bool reversed = false) const {
		for (;;) {
			const auto &v = nodes_[x];
			if (!l && r == v.count) {
				if constexpr (!commutative) if (reversed) return v.back;
				return v.product;
			}
			if (v.pending) return fold_lazy(x, l, r, nullptr, reversed);
			if (leaf(x)) {
				S result = e();
				if (reversed ^ v.reversed) for (int i = l; i < r; ++i) result = op(std::move(result), v.values[v.count - 1 - i]);
				else for (int i = l; i < r; ++i) result = op(std::move(result), v.values[i]);
				return result;
			}
			int a = reversed ? v.right : v.left, b = reversed ? v.left : v.right;
			int left = count(a);
			reversed ^= v.reversed;
			if (r <= left) { x = a; continue; }
			if (l >= left) { x = b; l -= left, r -= left; continue; }
			return op(fold(a, l, left, reversed), fold(b, 0, r - left, reversed));
		}
	}
	S fold_lazy(int x, int l, int r, const F *carry, bool reversed) const {
		const auto &v = nodes_[x];
		if (!l && r == v.count) {
			S result = v.product;
			if constexpr (!commutative) if (reversed) result = v.back;
			return carry ? mapping(*carry, std::move(result), v.count) : result;
		}
		F combined = id();
		if (v.pending) {
			combined = carry ? composition(*carry, v.lazy) : v.lazy;
			carry = &combined;
		}
		if (leaf(x)) {
			S result = e();
			if (reversed ^ v.reversed) for (int i = l; i < r; ++i) result = op(std::move(result), v.values[v.count - 1 - i]);
			else for (int i = l; i < r; ++i) result = op(std::move(result), v.values[i]);
			return carry ? mapping(*carry, std::move(result), r - l) : result;
		}
		int a = reversed ? v.right : v.left, b = reversed ? v.left : v.right;
		int left = count(a);
		reversed ^= v.reversed;
		if (r <= left) return fold_lazy(a, l, r, carry, reversed);
		if (l >= left) return fold_lazy(b, l - left, r - left, carry, reversed);
		return op(fold_lazy(a, l, left, carry, reversed), fold_lazy(b, 0, r - left, carry, reversed));
	}
	void apply_node(int x, int l, int r, const F &f) {
		if (l == 0 && r == count(x)) { act(x, f); return; }
		if (leaf(x)) {
			auto &v = nodes_[x];
			if (v.reversed) { int left = v.count - r; r = v.count - l, l = left; }
			if (v.pending) {
				F combined = composition(f, v.lazy);
				for (int i = 0; i < l; ++i) v.values[i] = mapping(v.lazy, v.values[i], 1);
				for (int i = l; i < r; ++i) v.values[i] = mapping(combined, v.values[i], 1);
				for (int i = r; i < v.count; ++i) v.values[i] = mapping(v.lazy, v.values[i], 1);
				v.pending = false;
			} else for (int i = l; i < r; ++i) v.values[i] = mapping(f, v.values[i], 1);
			pull_leaf(x);
			return;
		}
		push(x);
		int left = count(nodes_[x].left);
		if (l < left) apply_node(nodes_[x].left, l, std::min(r, left), f);
		if (r > left) apply_node(nodes_[x].right, std::max(0, l - left), r - left, f);
		pull(x);
	}
	void collect(int x, std::vector<S> &values) {
		if (!x) return;
		push(x);
		if (leaf(x)) values.insert(values.end(), nodes_[x].values.begin(), nodes_[x].values.end());
		else collect(nodes_[x].left, values), collect(nodes_[x].right, values);
	}
	int build(const std::vector<int> &leaves, int l, int r) {
		if (r - l == 1) return leaves[l];
		int m = l + (r - l) / 2;
		int a = build(leaves, l, m), b = build(leaves, m, r);
		return make_branch(a, b);
	}
	struct Boundary {
		int node, offset, first = 0, depth = 0;
		std::array<int, 64> path;
	};
	Boundary locate(int p) {
		Boundary result;
		int x = root_;
		while (!leaf(x)) {
			push(x);
			result.path[result.depth++] = x;
			int left = count(nodes_[x].left);
			if (p < left) x = nodes_[x].left;
			else p -= left, result.first += left, x = nodes_[x].right;
		}
		result.node = x, result.offset = p;
		return result;
	}
	void repair(const Boundary &boundary) {
		for (int i = boundary.depth; i-- > 0;) pull(boundary.path[i]);
	}
	bool reverse_boundary(int l, int r) {
		auto a = locate(l), b = locate(r - 1);
		int x = a.node, y = b.node, i = a.offset, j = b.offset + 1;
		if (x == y) {
			if (nodes_[x].reversed) { int left = count(x) - j; j = count(x) - i, i = left; }
			std::reverse(nodes_[x].values.begin() + i, nodes_[x].values.begin() + j);
			pull_leaf(x), repair(a);
			return true;
		}
		int nx = count(x), ny = count(y), tail = ny - j;
		int first_size = tail + nx - i, last_size = j + i;
		if (first_size < minimum || last_size < minimum || first_size > BlockSize || last_size > BlockSize) return false;
		if (i || tail) {
			push(x), push(y);
			auto &u = nodes_[x].values, &v = nodes_[y].values;
			auto capacity = u.capacity() + v.capacity();
			scratch_.clear();
			scratch_.insert(scratch_.end(), std::make_move_iterator(u.begin()), std::make_move_iterator(u.begin() + i));
			scratch_.insert(scratch_.end(), std::make_move_iterator(v.begin() + j), std::make_move_iterator(v.end()));
			if (first_size > nx) u.insert(u.end(), first_size - nx, e());
			if (tail > i) std::move_backward(u.begin() + i, u.begin() + nx, u.begin() + first_size);
			else if (tail < i) std::move(u.begin() + i, u.begin() + nx, u.begin() + tail);
			if (first_size < nx) u.erase(u.begin() + first_size, u.end());
			std::copy(std::make_move_iterator(scratch_.rbegin()), std::make_move_iterator(scratch_.rbegin() + tail), u.begin());
			v.erase(v.begin() + j, v.end());
			v.insert(v.end(), std::make_move_iterator(scratch_.rbegin() + tail), std::make_move_iterator(scratch_.rend()));
			leaf_bytes_ += (u.capacity() + v.capacity() - capacity) * sizeof(Value);
			scratch_.clear();
			pull_leaf(x), pull_leaf(y);
			repair(a), repair(b);
		}
		int length = b.first + ny - a.first;
		auto [first, rest] = split(root_, a.first);
		auto [middle, last] = split(rest, length);
		flip(middle);
		root_ = join(join(first, middle), last);
		return true;
	}
	void flush_front() {
		int n = int(front_.size());
		if (!n) return;
		if (n == 1) root_ = insert_node(root_, 0, std::move(front_[0]));
		else {
			int x = make_leaf(std::make_move_iterator(front_.rbegin()), std::make_move_iterator(front_.rend()));
			root_ = concatenate(x, root_);
		}
		front_.clear(), front_product_ = e(), buffered_ -= n;
	}
	void flush_back() {
		int n = int(back_.size());
		if (!n) return;
		if (n == 1) root_ = insert_node(root_, count(root_), std::move(back_[0]));
		else {
			int x = make_leaf(std::make_move_iterator(back_.begin()), std::make_move_iterator(back_.end()));
			root_ = concatenate(root_, x);
		}
		back_.clear(), back_product_ = e(), buffered_ -= n;
	}
	void flush() { if (buffered_) { flush_front(); flush_back(); } }
	void bounds(int l, int r) const { assert(0 <= l && l <= r && r <= size()); (void)l; (void)r; }

public:
	using value_type = S;
	using action_type = F;
	LazyReversibleSequence() {
		nodes_.emplace_back();
		scratch_.reserve(2 * BlockSize), front_.reserve(BlockSize), back_.reserve(BlockSize);
	}
	explicit LazyReversibleSequence(const std::vector<S> &values) : LazyReversibleSequence() { assign(values); }
	LazyReversibleSequence(const LazyReversibleSequence &other)
		: nodes_(other.nodes_), scratch_(other.scratch_), front_(other.front_), back_(other.back_),
		front_product_(other.front_product_), back_product_(other.back_product_),
		root_(other.root_), free_(other.free_), buffered_(other.buffered_) {
		for (const auto &node : nodes_) leaf_bytes_ += node.values.capacity() * sizeof(Value);
	}
	LazyReversibleSequence &operator=(const LazyReversibleSequence &other) {
		if (this != &other) { LazyReversibleSequence copy(other); swap(copy); }
		return *this;
	}
	LazyReversibleSequence(LazyReversibleSequence &&other) : LazyReversibleSequence() { swap(other); }
	LazyReversibleSequence &operator=(LazyReversibleSequence &&other) {
		if (this != &other) { LazyReversibleSequence moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(LazyReversibleSequence &other) noexcept(std::is_nothrow_swappable_v<S>) {
		nodes_.swap(other.nodes_), scratch_.swap(other.scratch_);
		front_.swap(other.front_), back_.swap(other.back_);
		using std::swap;
		swap(front_product_, other.front_product_), swap(back_product_, other.back_product_);
		std::swap(root_, other.root_), std::swap(free_, other.free_), std::swap(leaf_bytes_, other.leaf_bytes_);
		std::swap(buffered_, other.buffered_);
	}
	friend void swap(LazyReversibleSequence &a, LazyReversibleSequence &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
	int size() const { return count(root_) + buffered_; }
	bool empty() const { return !root_ && !buffered_; }
	void reserve(std::size_t n) {
		assert(n < std::size_t(1) << 30);
		if constexpr (std::is_nothrow_move_constructible_v<Node>) nodes_.reserve(2 * (n / minimum + 2) + 64);
		else {
			auto previous = nodes_.capacity();
			nodes_.reserve(2 * (n / minimum + 2) + 64);
			if (nodes_.capacity() != previous) {
				leaf_bytes_ = 0;
				for (const auto &node : nodes_) leaf_bytes_ += node.values.capacity() * sizeof(Value);
			}
		}
	}
	std::size_t memory_usage() const {
		return nodes_.capacity() * sizeof(Node) + leaf_bytes_ + (scratch_.capacity() + front_.capacity() + back_.capacity()) * sizeof(Value);
	}
	void clear() {
		nodes_.erase(nodes_.begin() + 1, nodes_.end());
		scratch_.clear(), front_.clear(), back_.clear();
		root_ = free_ = buffered_ = 0, leaf_bytes_ = 0;
		front_product_ = back_product_ = e();
	}
	void assign(const std::vector<S> &values) {
		assert(values.size() < std::size_t(1) << 30);
		clear();
		if (values.empty()) return;
		reserve(values.size());
		std::vector<int> leaves;
		leaves.reserve((values.size() + BlockSize - 1) / BlockSize);
		for (std::size_t first = 0; first < values.size(); first += BlockSize)
			leaves.push_back(make_leaf(values.begin() + first, values.begin() + std::min(first + BlockSize, values.size())));
		root_ = build(leaves, 0, int(leaves.size()));
	}
	void insert(int p, S value) {
		bounds(p, p);
		assert(size() < (1 << 30) - 1);
		if (!p) {
			front_product_ = op(value, front_product_);
			front_.push_back(std::move(value)), ++buffered_;
			if (front_.size() == BlockSize) flush_front();
		} else if (p == size()) {
			back_product_ = op(back_product_, value);
			back_.push_back(std::move(value)), ++buffered_;
			if (back_.size() == BlockSize) flush_back();
		} else {
			flush();
			root_ = insert_node(root_, p, std::move(value));
		}
	}
	S erase(int p) { bounds(p, p + 1); flush(); S value = e(); root_ = erase_node(root_, p, value); return value; }
	S get(int p) const {
		bounds(p, p + 1);
		if (buffered_) {
			int front = int(front_.size());
			if (p < front) return front_[front - 1 - p];
			p -= front;
			if (p >= count(root_)) return back_[p - count(root_)];
		}
		return fold(root_, p, p + 1);
	}
	void set(int p, S value) { bounds(p, p + 1); flush(); set_node(root_, p, std::move(value)); }
	S prod(int l, int r) const {
		bounds(l, r);
		if (l == r) return e();
		if (!l && r == size()) return all_prod();
		if (!buffered_) return fold(root_, l, r);
		S result = e();
		int front = int(front_.size()), length = count(root_);
		if (l < front) {
			int right = std::min(r, front);
			if (!l && right == front) result = front_product_;
			else for (int i = l; i < right; ++i) result = op(std::move(result), front_[front - 1 - i]);
		}
		int left = std::max(0, l - front), right = std::min(length, r - front);
		if (left < right) result = op(std::move(result), fold(root_, left, right));
		int offset = front + length;
		if (r > offset) {
			if (l <= offset && r == size()) result = op(std::move(result), back_product_);
			else for (int i = std::max(l, offset); i < r; ++i) result = op(std::move(result), back_[i - offset]);
		}
		return result;
	}
	S all_prod() const {
		S result = nodes_[root_].product;
		if (!front_.empty()) result = op(front_product_, result);
		if (!back_.empty()) result = op(result, back_product_);
		return result;
	}
	void apply(int p, const F &f) { apply(p, p + 1, f); }
	void apply(int l, int r, const F &f) { bounds(l, r); if (l != r) { flush(); apply_node(root_, l, r, f); } }
	void reverse(int l, int r) {
		bounds(l, r);
		if (r - l <= 1) return;
		flush();
		if (!l && r == size()) { flip(root_); return; }
		if (reverse_boundary(l, r)) return;
		auto [a, b] = split(root_, l);
		auto [c, d] = split(b, r - l);
		flip(c);
		root_ = concatenate(a, concatenate(c, d));
	}
	void rotate(int l, int m, int r) {
		bounds(l, m), bounds(m, r);
		if (l == m || m == r) return;
		flush();
		auto [a, b] = split(root_, l);
		auto [c, d] = split(b, m - l);
		auto [f, g] = split(d, r - m);
		root_ = concatenate(a, concatenate(f, concatenate(c, g)));
	}
	void move(int l, int r, int p) {
		bounds(l, r);
		assert(0 <= p && p <= size() - (r - l));
		if (l == r || l == p) return;
		flush();
		auto [a, b] = split(root_, l);
		auto [c, d] = split(b, r - l);
		auto [f, g] = split(concatenate(a, d), p);
		root_ = concatenate(f, concatenate(c, g));
	}
	std::vector<S> to_vector() { flush(); std::vector<S> result; result.reserve(size()); collect(root_, result); return result; }
};

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
