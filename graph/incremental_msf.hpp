#ifndef INCREMENTAL_MSF_HPP
#define INCREMENTAL_MSF_HPP

#include "../components/am_tree.hpp"
#include <bit>
#include <type_traits>

template<class Cost = std::int64_t, class Sum = __int128_t> class IncrementalMSF {
	static_assert(std::is_integral_v<Cost> && !std::is_same_v<Cost, bool> && sizeof(Cost) <= 8);
	static_assert(std::numeric_limits<Sum>::is_integer && std::numeric_limits<Sum>::is_signed && sizeof(Sum) <= 16);
	static_assert(std::numeric_limits<Sum>::digits >= std::numeric_limits<Cost>::digits);

  public:
	using cost_type = Cost;
	using sum_type = Sum;
	struct Edge { Cost cost; int id; };

  private:
	using Unsigned = std::make_unsigned_t<Cost>;
	using Key = std::conditional_t<(sizeof(Cost) <= 4), std::uint64_t, Edge>;
	static constexpr Unsigned sign = std::is_signed_v<Cost> ? Unsigned(1) << (std::numeric_limits<Unsigned>::digits - 1) : 0;
	static Key pack(Cost cost, int id) {
		if constexpr (sizeof(Cost) <= 4) return (std::uint64_t(Unsigned(cost) ^ sign) << 32) | std::uint32_t(id);
		else return {cost, id};
	}
	static Edge unpack(Key key) {
		if constexpr (sizeof(Cost) <= 4) return {std::bit_cast<Cost>(Unsigned(Unsigned(key >> 32) ^ sign)), int(std::uint32_t(key))};
		else return key;
	}
	static bool less(Key a, Key b) {
		if constexpr (sizeof(Cost) <= 4) return a < b;
		else return a.cost < b.cost || (a.cost == b.cost && a.id < b.id);
	}
	am_tree_detail::Tree<Key, less> tree_;
	int insertions_ = 0, refresh_ = 0;
	Sum total_ = 0;
	Key maximum_{};
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
	void update_total(Cost added, Cost removed = 0) {
		__int128_t next = __int128_t(total_) + __int128_t(added) - __int128_t(removed);
		assert(std::numeric_limits<Sum>::min() <= next && next <= std::numeric_limits<Sum>::max());
		total_ = Sum(next);
	}

  public:
	explicit IncrementalMSF(int n = 0) : tree_(n) {}
	IncrementalMSF(const IncrementalMSF&) = default;
	IncrementalMSF& operator=(const IncrementalMSF&) = default;
	IncrementalMSF(IncrementalMSF&& other) noexcept
		: tree_(std::move(other.tree_)), insertions_(std::exchange(other.insertions_, 0)),
		  refresh_(std::exchange(other.refresh_, 0)), total_(std::exchange(other.total_, Sum(0))), maximum_(other.maximum_) {}
	IncrementalMSF& operator=(IncrementalMSF&& other) noexcept {
		if (this != &other) {
			tree_ = std::move(other.tree_);
			insertions_ = std::exchange(other.insertions_, 0);
			refresh_ = std::exchange(other.refresh_, 0);
			total_ = std::exchange(other.total_, Sum(0));
			maximum_ = other.maximum_;
		}
		return *this;
	}
	int size() const { return tree_.size(); }
	int edge_count() const { return insertions_; }
	int component_count() const { return tree_.component_count(); }
	int forest_edge_count() const { return size() - component_count(); }
	bool connected() const { return component_count() <= 1; }
	Sum total_cost() const { return total_; }
	int add_edge(int u, int v, Cost cost) {
		vertex(u), vertex(v);
		assert(insertions_ < std::numeric_limits<int>::max());
		int id = insertions_++;
		if (u == v) return id;
		Key key = pack(cost, id);
		if (connected()) {
			if (!refresh_) {
				bool first = true;
				tree_.for_each([&](Key edge) {
					if (first || less(maximum_, edge)) maximum_ = edge;
					first = false;
				});
				refresh_ = size();
			}
			--refresh_;
			if (!less(key, maximum_)) return id;
		}
		auto removed = tree_.insert(u, v, key);
		if (!removed) { update_total(cost); return -1; }
		Edge old = unpack(*removed);
		if (old.id != id) update_total(cost, old.cost);
		return old.id;
	}
	std::optional<Edge> path_max(int u, int v) {
		vertex(u), vertex(v);
		auto key = tree_.path_max(u, v);
		if (!key) return std::nullopt;
		return unpack(*key);
	}
	bool same(int u, int v) { vertex(u), vertex(v); return u == v || tree_.path_max(u, v).has_value(); }
	int component_size(int v) { vertex(v); return tree_.component_size(v); }
	std::vector<Edge> forest_edges() const {
		std::vector<Edge> result;
		result.reserve(forest_edge_count());
		tree_.for_each([&](Key edge) { result.push_back(unpack(edge)); });
		return result;
	}
	void clear() { tree_.clear(); insertions_ = refresh_ = 0; total_ = 0; }
};

#endif
