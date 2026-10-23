#ifndef OFFLINE_DYNAMIC_CONNECTIVITY_HPP
#define OFFLINE_DYNAMIC_CONNECTIVITY_HPP

#include "../../components/offline_time_segments.hpp"
#include "../../components/radix_sort.hpp"
#include "../../data_structure/union_find/rollback_union_find.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

class OfflineDynamicConnectivity {
	struct Event { std::uint64_t key; int time; std::uint32_t order; };
	struct Query { int type, u, v; };
	int n_;
	std::vector<Event> events_;
	std::vector<Query> queries_;
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }
	void update_(int u, int v, bool remove) {
		check_(u), check_(v);
		if (u == v) return;
		assert(events_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
		if (u > v) std::swap(u, v);
		std::uint64_t key = (std::uint64_t(static_cast<unsigned>(u)) << 32) | static_cast<unsigned>(v);
		std::uint32_t order = static_cast<std::uint32_t>(events_.size()) | (std::uint32_t(remove) << 31);
		events_.push_back({key, query_count(), order});
	}
	int query_(int type, int u, int v) {
		assert(queries_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
		int id = query_count();
		queries_.push_back({type, u, v});
		return id;
	}
	template <bool Hooks, class Link, class Cut, class Visit>
	void run_(Link &link, Cut &cut, Visit &visit) const {
		if (queries_.empty()) return;
		auto events = events_;
		stable_radix_sort(events, [](const Event &event) { return event.key; });
		std::vector<OfflineTimeSegments::Interval> intervals;
		std::vector<std::pair<int, int>> edges;
		intervals.reserve(events.size() / 2), edges.reserve(events.size() / 2);
		for (std::size_t i = 0; i < events.size();) {
			std::uint64_t key = events[i].key;
			int count = 0, start = 0;
			auto append = [&](int end) {
				if (start == end) return;
				intervals.push_back({start, end, static_cast<int>(edges.size())});
				edges.emplace_back(static_cast<int>(key >> 32), static_cast<int>(key & 0xffffffffU));
			};
			do {
				const auto &event = events[i++];
				if (!(event.order >> 31)) {
					if (count++ == 0) start = event.time;
				} else if (count && --count == 0) append(event.time);
			} while (i < events.size() && events[i].key == key);
			if (count) append(query_count());
		}
		OfflineTimeSegments timeline(query_count(), intervals);
		struct State {
			RollbackUnionFind uf;
			std::vector<std::pair<int, int>> history;
			Link &link;
			Cut &cut;
			std::size_t snapshot() const { return uf.snapshot(); }
			void merge(int u, int v) {
				int a = uf.leader(u), b = uf.leader(v);
				if (a == b) return;
				if (uf.size(a) < uf.size(b)) std::swap(a, b);
				uf.merge(a, b);
				if constexpr (Hooks) {
					history.emplace_back(a, b);
					link(a, b);
				}
			}
			void rollback(std::size_t saved) {
				if constexpr (Hooks) {
					while (uf.snapshot() > saved) {
						auto [a, b] = history.back();
						cut(a, b);
						history.pop_back(), uf.undo();
					}
				} else uf.rollback(saved);
			}
		};
		State state{RollbackUnionFind(n_), {}, link, cut};
		std::size_t capacity = std::min<std::size_t>(n_ ? n_ - 1 : 0, edges.size());
		state.uf.reserve(capacity);
		if constexpr (Hooks) state.history.reserve(capacity);
		timeline.run(state, [&](State &s, int id) { s.merge(edges[id].first, edges[id].second); },
			[&](int id, const State &s) { visit(id, s.uf); });
	}

  public:
	explicit OfflineDynamicConnectivity(int n = 0) : n_(n) { assert(n >= 0); }
	void reset(int n) { assert(n >= 0); n_ = n; events_.clear(), queries_.clear(); }
	void reserve(int updates, int queries = 0) {
		assert(updates >= 0 && queries >= 0);
		events_.reserve(updates), queries_.reserve(queries);
	}
	int vertex_count() const { return n_; }
	int query_count() const { return static_cast<int>(queries_.size()); }
	void link(int u, int v) { update_(u, v, false); }
	void cut(int u, int v) { update_(u, v, true); }
	int query_same(int u, int v) { check_(u), check_(v); return query_(0, u, v); }
	int query_size(int v) { check_(v); return query_(1, v, 0); }
	int query_component_count() { return query_(2, 0, 0); }
	int query() { return query_(3, 0, 0); }
	template <class Visit>
	void run(Visit &&visit) const {
		auto noop = [](int, int) {};
		run_<false>(noop, noop, visit);
	}
	template <class Link, class Cut, class Visit>
	void run(Link &&link, Cut &&cut, Visit &&visit) const { run_<true>(link, cut, visit); }
	std::vector<int> solve() const {
		std::vector<int> answers(queries_.size());
		run([&](int id, const RollbackUnionFind &uf) {
			const auto &q = queries_[id];
			if (q.type == 0) answers[id] = uf.same(q.u, q.v);
			else if (q.type == 1) answers[id] = uf.size(q.u);
			else { assert(q.type == 2); answers[id] = uf.component_count(); }
		});
		return answers;
	}
};

#endif
