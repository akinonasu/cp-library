#ifndef KINETIC_TOURNAMENT_HPP
#define KINETIC_TOURNAMENT_HPP

#include "generic_segtree_beats.hpp"
#include "../../components/kinetic_tournament_monoid.hpp"
#include <optional>

template<class T = std::int64_t, bool maximize = true> class KineticTournament {
	using M = kinetic_tournament_detail::Monoid<T, maximize>;
	using Node = typename M::Node;
	GenericSegtreeBeats<Node, M::op, M::identity, typename M::Tag, M::mapping, M::composition, M::tag_identity> tree_;
	std::vector<T> slopes_;
	std::vector<unsigned char> active_;
	int count_ = 0;
	void check([[maybe_unused]] int i) const { assert(0 <= i && i < size()); }
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= size()); }

  public:
	struct Result { int index; T value; };
	using value_type = T;
	KineticTournament() = default;
	explicit KineticTournament(const std::vector<T> &slopes, T value = 0) { build(slopes, value); }
	KineticTournament(const std::vector<T> &slopes, const std::vector<T> &values) { build(slopes, values); }
	KineticTournament(const std::vector<T> &slopes, const std::vector<std::optional<T>> &values) { build(slopes, values); }
	KineticTournament(const std::vector<T> &slopes, std::nullopt_t) { build(slopes, std::nullopt); }
	KineticTournament(const KineticTournament &) = default;
	KineticTournament &operator=(const KineticTournament &) = default;
	KineticTournament(KineticTournament &&other) : KineticTournament() { swap(other); }
	KineticTournament &operator=(KineticTournament &&other) {
		if (this != &other) { KineticTournament next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(KineticTournament &other) noexcept {
		std::swap(tree_, other.tree_);
		slopes_.swap(other.slopes_); active_.swap(other.active_); std::swap(count_, other.count_);
	}
	void build(const std::vector<T> &slopes, const std::vector<std::optional<T>> &values) {
		assert(slopes.size() == values.size() && slopes.size() <= std::size_t(std::numeric_limits<int>::max() / 4));
		assert(std::is_sorted(slopes.begin(), slopes.end()));
		slopes_ = slopes;
		active_.assign(slopes.size(), 0);
		count_ = 0;
		std::vector<Node> nodes(slopes.size());
		for (int i = 0; i < size(); ++i) if (values[i]) {
			nodes[i] = {*values[i], slopes_[i], M::never, i};
			active_[i] = 1; ++count_;
		}
		tree_.build(nodes);
	}
	void build(const std::vector<T> &slopes, const std::vector<T> &values) { build(slopes, std::vector<std::optional<T>>(values.begin(), values.end())); }
	void build(const std::vector<T> &slopes, T value = 0) { build(slopes, std::vector<std::optional<T>>(slopes.size(), value)); }
	void build(const std::vector<T> &slopes, std::nullopt_t) { build(slopes, std::vector<std::optional<T>>(slopes.size())); }
	int size() const { return static_cast<int>(slopes_.size()); }
	int active_size() const { return count_; }
	bool empty() const { return count_ == 0; }
	const std::vector<T> &slopes() const { return slopes_; }
	void clear() { tree_.build(std::vector<Node>(size())); std::fill(active_.begin(), active_.end(), 0); count_ = 0; }
	void set(int i, T value) {
		check(i);
		tree_.set(i, Node{value, slopes_[i], M::never, i});
		count_ += !active_[i]; active_[i] = 1;
	}
	bool erase(int i) {
		check(i);
		if (!active_[i]) return false;
		tree_.set(i, M::identity());
		active_[i] = 0; --count_;
		return true;
	}
	std::optional<T> get(int i) { check(i); return active_[i] ? std::optional<T>(tree_.get(i).value) : std::nullopt; }
	void apply(int l, int r, T a, T b = 0) {
		check(l, r); assert(a >= 0);
		if (l == r || (!a && !b)) return;
		if (!l && r == size()) tree_.apply({a, b});
		else tree_.apply(l, r, {a, b});
	}
	void add(int l, int r, T value) { apply(l, r, 0, value); }
	void advance(int l, int r, T steps = 1) { apply(l, r, steps, 0); }
	void all_apply(T a, T b = 0) { apply(0, size(), a, b); }
	std::optional<Result> prod(int l, int r) {
		check(l, r);
		if (!l && r == size()) return all_prod();
		auto node = tree_.prod(l, r);
		return node.index == -1 ? std::nullopt : std::optional<Result>(Result{node.index, node.value});
	}
	std::optional<Result> all_prod() const {
		auto node = tree_.all_prod();
		return node.index == -1 ? std::nullopt : std::optional<Result>(Result{node.index, node.value});
	}
	std::vector<std::optional<T>> values() {
		auto nodes = tree_.values();
		std::vector<std::optional<T>> result(size());
		for (int i = 0; i < size(); ++i) if (active_[i]) result[i] = nodes[i].value;
		return result;
	}
};

#endif
