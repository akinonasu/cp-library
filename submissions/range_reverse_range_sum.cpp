#line 1 "test/lazy_reversible_sequence_reverse.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_reverse_range_sum"
#line 1 "data_structure/lazy_reversible_sequence.hpp"



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
			if constexpr (!commutative) pull_leaf(x), repair(a);
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


#line 1 "components/fast_io_bulk.hpp"



#line 1 "components/compiler_features.hpp"



#if defined(__x86_64__) || defined(__i386__)
#define CP_X86 1
#else
#define CP_X86 0
#endif
#if CP_X86 && defined(__GNUC__) && !defined(__clang__)
#define CP_GCC_X86 1
#else
#define CP_GCC_X86 0
#endif
#if CP_X86 && (defined(__GNUC__) || defined(__clang__))
#define CP_GNU_X86 1
#else
#define CP_GNU_X86 0
#endif
#if CP_GCC_X86 && defined(__linux__) && defined(__x86_64__)
#define CP_GCC_LINUX64 1
#else
#define CP_GCC_LINUX64 0
#endif


#line 5 "components/fast_io_bulk.hpp"

#line 1 "components/fast_io_output.hpp"



#line 1 "components/fast_io.hpp"



#line 6 "components/fast_io.hpp"
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 7 "components/fast_io_output.hpp"

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 8 "components/fast_io_bulk.hpp"
#include <span>
#line 10 "components/fast_io_bulk.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	template <bool Signed = false>
	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid, unsigned *signs = nullptr) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		unsigned n0 = 0, n1 = 0;
		if constexpr (Signed) {
			if (a < 1 || a > 11 || b <= a + 1 || b - a - 1 > 11) [[unlikely]] {
				invalid = _mm256_set1_epi32(-1);
				p += std::min(b + 1, 32u);
				*signs = 0;
				return _mm256_setzero_si256();
			}
			n0 = p[0] == '-', n1 = p[a + 1] == '-';
			*signs = n0 | (n1 << 1);
		}
		if (a < n0 + 1 || a - n0 > 10 || b <= a + n1 + 1 || b - a - n1 - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		auto window = bytes;
		if constexpr (Signed) window = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1 + n1)), _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + n0)));
		else window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - n1 - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a - n0].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	template <bool Signed = false>
	static std::uint32_t one(const char *&p, bool &bad) {
		bool negative = false;
		if constexpr (Signed) negative = *p == '-', p += negative;
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		if constexpr (Signed) bad |= value > 2147483647ULL + negative;
		else bad |= value > 0xffffffffULL;
		p += n + 1;
		return negative ? 0u - std::uint32_t(value) : std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	template <bool Signed = false>
	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				unsigned signs0 = 0, signs1 = 0, signs2 = 0, signs3 = 0;
				const auto g0 = two<Signed>(p[0], invalid, &signs0), g1 = two<Signed>(p[1], invalid, &signs1), g2 = two<Signed>(p[2], invalid, &signs2), g3 = two<Signed>(p[3], invalid, &signs3);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				auto max_high = _mm256_set1_epi32(42), max_low = _mm256_set1_epi32(94967295);
				auto signs = _mm256_setzero_si256();
				if constexpr (Signed) {
					signs = _mm256_setr_epi32(signs0 & 1, signs1 & 1, signs2 & 1, signs3 & 1, signs0 >> 1, signs1 >> 1, signs2 >> 1, signs3 >> 1);
					max_high = _mm256_set1_epi32(21), max_low = _mm256_add_epi32(_mm256_set1_epi32(47483647), signs);
				}
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, max_high),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, max_high), _mm256_cmpgt_epi32(low, max_low)));
				invalid = _mm256_or_si256(invalid, overflow);
				auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				if constexpr (Signed) {
					const auto mask = _mm256_sub_epi32(_mm256_setzero_si256(), signs);
					value = _mm256_sub_epi32(_mm256_xor_si256(value, mask), mask);
				}
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <bool Signed = false, class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep<Signed>(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one<Signed>(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <bool Signed = false, class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks<Signed>(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (!Signed && done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				std::conditional_t<Signed, std::int32_t, unsigned> value = 0;
				in.read(value);
				emit(reinterpret_cast<unsigned *>(&value), 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif


namespace fast_io_detail {
template <class Emit, class T, std::size_t... I>
void emit_record(Emit &emit, const T *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}

template <class Emit>
void read_signed_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks<true>(input, count, [&](const unsigned *values, std::size_t n) {
			emit(reinterpret_cast<const std::int32_t *>(values), n);
		});
		return;
	}
#endif
	std::int32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

template <std::size_t Fields, class Emit>
void read_signed_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::int32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_signed_blocks(input, count * Fields, [&](const std::int32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}


#line 5 "test/lazy_reversible_sequence_reverse.test.cpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
long long mapping(long long f, long long x, int n) { return x + f * n; }
long long id() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	std::vector<long long> a(n);
	std::size_t position = 0;
	read_unsigned_records<1>(input, n, [&](std::uint32_t value) { a[position++] = value; });
	LazyReversibleSequence<long long, op, e, long long, mapping, op, id, true, 512> tree(a);
	read_unsigned_records<3>(input, q, [&](std::uint32_t type, std::uint32_t l, std::uint32_t r) {
		if (type == 0) tree.reverse(l, r);
		else { output.write(tree.prod(l, r)); output.write('\n'); }
	});
}
