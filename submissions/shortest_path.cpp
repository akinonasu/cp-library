#line 1 "test/dijkstra_path.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#line 1 "graph/dijkstra.hpp"



#line 1 "components/dijkstra_queue.hpp"



#line 1 "components/RadixHeap.hpp"



#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class Key = std::uint64_t, class T = int>
class RadixHeap {
	static_assert(std::is_integral_v<Key> && !std::is_same_v<Key, bool> && sizeof(Key) <= 8);
	using U = std::make_unsigned_t<Key>;
	using State = std::pair<Key, T>;
	std::vector<State> bucket[std::numeric_limits<U>::digits + 1];
	Key lower;
	std::size_t count = 0;
	int index(Key key) const { return std::bit_width(U(U(key) ^ U(lower))); }
	void refill() {
		assert(count);
		if (!bucket[0].empty()) return;
		int i = 1;
		while (bucket[i].empty()) ++i;
		lower = bucket[i][0].first;
		for (const auto &s : bucket[i]) lower = std::min(lower, s.first);
		for (auto &s : bucket[i]) bucket[index(s.first)].push_back(std::move(s));
		bucket[i].clear();
	}

  public:
	explicit RadixHeap(Key minimum = 0) : lower(minimum) {}
	RadixHeap(const RadixHeap &) = default;
	RadixHeap &operator=(const RadixHeap &) = default;
	RadixHeap(RadixHeap &&other) noexcept : lower(other.lower), count(std::exchange(other.count, 0)) {
		for (int i = 0; i <= std::numeric_limits<U>::digits; ++i) bucket[i] = std::move(other.bucket[i]);
	}
	RadixHeap &operator=(RadixHeap &&other) noexcept {
		if (this != &other) {
			lower = other.lower;
			count = std::exchange(other.count, 0);
			for (int i = 0; i <= std::numeric_limits<U>::digits; ++i) bucket[i] = std::move(other.bucket[i]);
		}
		return *this;
	}

	bool empty() const { return count == 0; }
	std::size_t size() const { return count; }
	Key last() const { return lower; }
	void clear(Key minimum = 0) {
		for (auto &b : bucket) b.clear();
		lower = minimum;
		count = 0;
	}
	void push(State s) {
		assert(s.first >= lower);
		bucket[index(s.first)].push_back(std::move(s));
		++count;
	}
	const State &top() {
		refill();
		return bucket[0].back();
	}
	void pop() {
		refill();
		bucket[0].pop_back();
		--count;
	}
	State pop_value() {
		refill();
		State s = std::move(bucket[0].back());
		bucket[0].pop_back();
		--count;
		return s;
	}
};


#line 14 "components/dijkstra_queue.hpp"

namespace shortest_path_detail {
template<class T> class RadixQueue {
	static_assert(std::is_integral_v<T> && sizeof(T) <= 8);
	struct State { T key; int vertex; };
	State small_[32];
	std::vector<std::uint64_t> buckets_[33];
	std::uint32_t minimum_[33];
	std::uint32_t occupied_ = 0;
	std::uint64_t last_ = 0;
	std::size_t count_ = 0;
	int mode_ = 0;
	RadixHeap<std::uint64_t> wide_;
	void put(std::uint64_t state) {
		std::uint32_t key = state >> 32;
		int index = std::bit_width(key ^ std::uint32_t(last_));
		buckets_[index].push_back(state);
		if (index) {
			occupied_ |= std::uint32_t(1) << (index - 1);
			minimum_[index] = std::min(minimum_[index], key);
		}
	}
	void promote() {
		if (mode_ == 0) {
			for (std::size_t i = 0; i < count_; ++i) wide_.push({std::uint64_t(small_[i].key), small_[i].vertex});
		} else {
			for (auto &bucket : buckets_) {
				for (auto state : bucket) {
					std::uint64_t key = last_ + std::uint32_t(std::uint32_t(state >> 32) - std::uint32_t(last_));
					wide_.push({key, int(std::uint32_t(state))});
				}
				bucket.clear();
			}
		}
		mode_ = 2;
	}

  public:
	bool empty() const { return count_ == 0; }
	void push(T key, int vertex) {
		assert(key >= 0 && std::uint64_t(key) >= last_);
		if (mode_ == 0 && count_ < 32) {
			small_[count_++] = {key, vertex};
			return;
		}
		if (mode_ == 0) {
			bool narrow = std::uint64_t(key) - last_ <= std::uint64_t(std::numeric_limits<std::int32_t>::max());
			for (std::size_t i = 0; i < count_; ++i)
				narrow &= std::uint64_t(small_[i].key) - last_ <= std::uint64_t(std::numeric_limits<std::int32_t>::max());
			if (narrow) {
				std::fill_n(minimum_, 33, std::numeric_limits<std::uint32_t>::max());
				for (std::size_t i = 0; i < count_; ++i)
					put((std::uint64_t(std::uint32_t(small_[i].key)) << 32) | std::uint32_t(small_[i].vertex));
				mode_ = 1;
			} else promote();
		}
		if (mode_ == 1 && std::uint64_t(key) - last_ > std::uint64_t(std::numeric_limits<std::int32_t>::max())) promote();
		if (mode_ == 2) wide_.push({std::uint64_t(key), vertex});
		else put((std::uint64_t(std::uint32_t(key)) << 32) | std::uint32_t(vertex));
		++count_;
	}
	std::pair<T, int> pop() {
		assert(count_);
		if (mode_ == 0) {
			std::size_t best = 0;
			for (std::size_t i = 1; i < count_; ++i)
				if (small_[i].key < small_[best].key) best = i;
			State state = small_[best];
			small_[best] = small_[--count_];
			last_ = std::uint64_t(state.key);
			return {state.key, state.vertex};
		}
		if (mode_ == 2) {
			auto state = wide_.pop_value();
			last_ = state.first;
			--count_;
			return {T(state.first), state.second};
		}
		if (buckets_[0].empty()) {
			int index = std::countr_zero(occupied_) + 1;
			last_ += std::uint32_t(minimum_[index] - std::uint32_t(last_));
			minimum_[index] = std::numeric_limits<std::uint32_t>::max();
			occupied_ &= occupied_ - 1;
			if (buckets_[index].size() == 1) {
				auto state = buckets_[index].back();
				buckets_[index].clear();
				--count_;
				return {T(last_), int(std::uint32_t(state))};
			}
			for (auto state : buckets_[index]) put(state);
			buckets_[index].clear();
		}
		auto state = buckets_[0].back();
		buckets_[0].pop_back();
		--count_;
		return {T(last_), int(std::uint32_t(state))};
	}
};
}


#line 1 "components/dijkstra_topological.hpp"



#line 8 "components/dijkstra_topological.hpp"
#include <functional>
#line 13 "components/dijkstra_topological.hpp"

namespace shortest_path_detail {
template<bool Path, class G, class T>
std::vector<T> topological_run(const G &g, int start, int target, T inf, std::vector<int> &parent) {
	std::size_t n = std::size_t(g.size());
	assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(start >= 0 && static_cast<std::size_t>(start) < n);
	assert(inf > 0);
	std::vector<T> distance(n, inf);
	std::vector<std::size_t> indegree(n);
	std::vector<int> ready;
	ready.reserve(n);
	for (std::size_t v = 0; v < n; ++v)
		for (const auto &[to, weight] : g[int(v)]) {
			assert(to >= 0 && static_cast<std::size_t>(to) < n);
			assert(weight >= 0);
			++indegree[to];
		}
	for (std::size_t v = 0; v < n; ++v)
		if (!indegree[v]) ready.push_back(int(v));
	distance[start] = 0;
	auto improve = [&](int from, const T &cost, const T &allowance, int to, const auto &weight) {
		using Weight = std::remove_cvref_t<decltype(weight)>;
		if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_bounded &&
					  std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
			using Compare = std::common_type_t<Weight, T>;
			if (Compare(weight) >= Compare(allowance)) return false;
		} else if (weight >= allowance) return false;
		T next = cost + weight;
		if (next >= distance[to]) return false;
		if constexpr (Path) {
			if (next >= distance[target]) return false;
			parent[to] = from;
		}
		distance[to] = std::move(next);
		return true;
	};
	for (std::size_t head = 0; head < ready.size(); ++head) {
		int v = ready[head];
		if constexpr (Path) if (v == target) return distance;
		T cost = distance[v];
		bool active = cost < inf;
		if constexpr (Path) active &= cost < distance[target];
		T allowance = active ? inf - cost : T(0);
		for (const auto &[to, weight] : g[v]) {
			if (active) improve(v, cost, allowance, to, weight);
			if (--indegree[to] == 0) ready.push_back(to);
		}
	}
	using State = std::pair<T, int>;
	constexpr bool radix = std::is_integral_v<T> && sizeof(T) <= 8;
	std::conditional_t<radix, shortest_path_detail::RadixQueue<T>, std::vector<State>> queue;
	auto push = [&](T cost, int v) {
		if constexpr (radix) queue.push(cost, v);
		else {
			queue.emplace_back(std::move(cost), v);
			std::push_heap(queue.begin(), queue.end(), std::greater<State>());
		}
	};
	for (std::size_t v = 0; v < n; ++v) {
		if (!indegree[v] || distance[v] == inf || g[v].empty()) continue;
		if constexpr (Path) if (int(v) == target || distance[v] >= distance[target]) continue;
		push(distance[v], int(v));
	}
	while (!queue.empty()) {
		const auto [cost, v] = [&] {
			if constexpr (radix) return queue.pop();
			else {
				std::pop_heap(queue.begin(), queue.end(), std::greater<State>());
				State state = std::move(queue.back());
				queue.pop_back();
				return state;
			}
		}();
		if (cost != distance[v]) continue;
		if constexpr (Path) if (cost >= distance[target]) break;
		T allowance = inf - cost;
		for (const auto &[to, weight] : g[v]) {
			if (!improve(v, cost, allowance, to, weight)) continue;
			if constexpr (Path) if (to == target) continue;
			if (!g[to].empty()) push(distance[to], to);
		}
	}
	return distance;
}
}


#line 1 "components/dijkstra_unique.hpp"



#line 10 "components/dijkstra_unique.hpp"

namespace shortest_path_detail {
template<class G, class T>
std::pair<T, std::vector<int>> unique_path(const G &g, int st, int target, T inf, std::vector<int> &parent) {
	using State = std::pair<T, int>;
	std::vector<State> pending;
	pending.emplace_back(T(0), st);
	while (!pending.empty()) {
		auto [cost, v] = std::move(pending.back());
		pending.pop_back();
		for (const auto &first : g[v]) {
			const auto *edge = &first;
			int from = v;
			T prefix = cost;
			for (;;) {
				const auto &[to, weight] = *edge;
				assert(weight >= 0);
				if (to == st) break;
				using Weight = std::remove_cvref_t<decltype(weight)>;
				T allowance = inf - prefix;
				if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_bounded &&
							  std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
					using Compare = std::common_type_t<Weight, T>;
					if (Compare(weight) >= Compare(allowance)) break;
				} else if (weight >= allowance) break;
				T next = prefix + weight;
				if (to == target) {
					std::vector<int> path{to};
					for (int u = from; u != st; u = parent[u]) path.push_back(u);
					path.push_back(st);
					std::reverse(path.begin(), path.end());
					return {std::move(next), std::move(path)};
				}
				const auto &outgoing = g[to];
				if (outgoing.empty()) break;
				parent[to] = from;
				if (outgoing.size() != 1) {
					pending.emplace_back(std::move(next), to);
					break;
				}
				from = to;
				edge = &outgoing[0];
				prefix = std::move(next);
			}
		}
	}
	return {T(-1), {}};
}
}


#line 1 "components/directed_csr.hpp"



#line 8 "components/directed_csr.hpp"
#include <memory>
#include <span>
#include <stdexcept>
#line 14 "components/directed_csr.hpp"

namespace directed_csr_detail { struct Access; }

template<class T>
class DirectedCSR {
  public:
	struct Edge { int from, to; T weight; };
	using Arc = std::pair<int, T>;
	using weight_type = T;
	using distance_type = std::conditional_t<std::is_integral_v<T> && sizeof(T) < 8, std::int64_t, T>;

  private:
	friend struct directed_csr_detail::Access;
	std::vector<std::size_t> offset_;
	std::vector<Arc> arcs_;
	int single_gap_ = -2;
	bool unique_incoming_ = true;
	void count_edge(const Edge &edge, std::vector<std::uint64_t> &seen) {
		assert(0 <= edge.from && edge.from < size() && 0 <= edge.to && edge.to < size());
		++offset_[edge.from];
		if (unique_incoming_) {
			auto mask = std::uint64_t(1) << (edge.to % 64);
			auto &word = seen[edge.to / 64];
			unique_incoming_ = !(word & mask);
			word |= mask;
		}
	}
	void finish(std::span<const Edge> edges) {
		const auto n = offset_.size() - 1;
		single_gap_ = -2;
		if (edges.size() >= n - (n != 0) && edges.size() <= n &&
			std::all_of(offset_.begin(), offset_.end() - 1, [](std::size_t x) { return x <= 1; })) {
			single_gap_ = -1;
			if (edges.size() != n) single_gap_ = int(std::find(offset_.begin(), offset_.end() - 1, 0) - offset_.begin());
			for (std::size_t v = 0; v <= n; ++v) offset_[v] = v - (single_gap_ >= 0 && v > std::size_t(single_gap_));
			arcs_.resize(edges.size());
			for (const auto &edge : edges) arcs_[edge.from - (single_gap_ >= 0 && edge.from > single_gap_)] = {edge.to, edge.weight};
			return;
		}
		std::size_t active = offset_[0] != 0;
		for (std::size_t v = 1; v < offset_.size(); ++v) {
			active += offset_[v] != 0;
			offset_[v] += offset_[v - 1];
		}
		arcs_.resize(edges.size());
		std::size_t i = edges.size();
#if defined(__GNUC__) || defined(__clang__)
		if (active >= 8192) {
			for (; i > 32; --i) {
				const auto from = edges[i - 33].from;
				__builtin_prefetch(&offset_[from], 1, 3);
				__builtin_prefetch(&arcs_[offset_[from] - 1], 1, 3);
				const auto &edge = edges[i - 1];
				arcs_[--offset_[edge.from]] = {edge.to, edge.weight};
			}
		}
#endif
		for (; i; --i) {
			const auto &edge = edges[i - 1];
			arcs_[--offset_[edge.from]] = {edge.to, edge.weight};
		}
	}
	void clear_edges() {
		single_gap_ = -2;
		unique_incoming_ = true;
		std::fill(offset_.begin(), offset_.end(), 0);
		arcs_.clear();
	}

  public:
	explicit DirectedCSR(int n = 0) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
	}
	DirectedCSR(int n, std::span<const Edge> edges) { build(n, edges); }
	template<class Producer>
	DirectedCSR(int n, std::size_t m, Producer &&produce) { build(n, m, std::forward<Producer>(produce)); }
	void build(int n, std::span<const Edge> edges) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
		single_gap_ = -2;
		unique_incoming_ = edges.size() <= std::size_t(n);
		try {
			std::vector<std::uint64_t> seen(unique_incoming_ ? (std::size_t(n) + 63) / 64 : 0);
			for (const auto &edge : edges) count_edge(edge, seen);
			finish(edges);
		}
		catch (...) { clear_edges(); throw; }
	}
	template<class Producer>
	void build(int n, std::size_t m, Producer &&produce) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
		single_gap_ = -2;
		unique_incoming_ = m <= std::size_t(n);
		try {
			std::vector<std::uint64_t> seen(unique_incoming_ ? (std::size_t(n) + 63) / 64 : 0);
			auto edges = std::make_unique_for_overwrite<Edge[]>(m);
			std::size_t count = 0;
			auto emit = [&](Edge edge) {
				if (count == m) throw std::length_error("DirectedCSR edge count");
				count_edge(edge, seen);
				edges[count++] = std::move(edge);
			};
			std::forward<Producer>(produce)(emit);
			if (count != m) throw std::length_error("DirectedCSR edge count");
			finish(std::span<const Edge>(edges.get(), m));
		} catch (...) { clear_edges(); throw; }
	}
	int size() const { return offset_.empty() ? 0 : int(offset_.size() - 1); }
	bool empty() const { return size() == 0; }
	std::size_t edge_count() const { return arcs_.size(); }
	std::span<const Arc> operator[](int v) const {
		assert(0 <= v && v < size());
		return std::span<const Arc>(arcs_).subspan(offset_[v], offset_[v + 1] - offset_[v]);
	}
};

namespace directed_csr_detail {
template<class T>
struct SingleEdgeView {
	std::span<const typename DirectedCSR<T>::Arc> arcs;
	int n, gap;
	int size() const { return n; }
	std::span<const typename DirectedCSR<T>::Arc> operator[](int v) const {
		if (v == gap) return {};
		return arcs.subspan(v - (gap >= 0 && v > gap), 1);
	}
};

struct Access {
	template<class T>
	static bool unique_incoming(const DirectedCSR<T> &g) { return g.offset_.empty() || g.unique_incoming_; }
	template<class T>
	static bool has_single_edge_layout(const DirectedCSR<T> &g) { return !g.offset_.empty() && g.single_gap_ >= -1; }
	template<class T>
	static SingleEdgeView<T> single_edge_view(const DirectedCSR<T> &g) { return {g.arcs_, g.size(), g.single_gap_}; }
};
}


#line 17 "graph/dijkstra.hpp"

inline constexpr std::int64_t DIJKSTRA_INF = 4'000'000'000'000'000'000;

namespace shortest_path_detail {
template<class T> T infinity() {
	if constexpr (std::numeric_limits<T>::is_bounded && std::numeric_limits<T>::is_integer && std::numeric_limits<T>::digits < 62)
		return std::numeric_limits<T>::max();
	else return T(DIJKSTRA_INF);
}

template<bool Path, class G, class T>
std::vector<T> run(const G &g, int st, int target, T inf, std::vector<int> &parent) {
	const auto n = static_cast<std::size_t>(g.size());
	assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<std::size_t>(st) < n);
	assert(inf > 0);
	std::vector<T> d(n, inf);
	d[st] = 0;
	using State = std::pair<T, int>;
	constexpr bool radix = std::is_integral_v<T> && sizeof(T) <= 8;
	std::conditional_t<radix, RadixQueue<T>, std::vector<State>> pq;
	auto push = [&](T cost, int v) {
		if constexpr (radix) pq.push(cost, v);
		else {
			pq.emplace_back(std::move(cost), v);
			std::push_heap(pq.begin(), pq.end(), std::greater<State>());
		}
	};
	push(0, st);
	while (!pq.empty()) {
		const auto [cost, v] = [&] {
			if constexpr (radix) return pq.pop();
			else {
				std::pop_heap(pq.begin(), pq.end(), std::greater<State>());
				State s = std::move(pq.back());
				pq.pop_back();
				return s;
			}
		}();
		if (d[v] != cost) continue;
		if constexpr (Path) if (cost >= d[target]) break;
		T allowance = inf - cost;
		for (const auto &[to, c] : g[v]) {
			assert(to >= 0 && static_cast<std::size_t>(to) < n);
			assert(c >= 0);
			using Weight = std::remove_cvref_t<decltype(c)>;
			if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_bounded &&
						  std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
				using Compare = std::common_type_t<Weight, T>;
				if (Compare(c) >= Compare(allowance)) continue;
			} else if (c >= allowance) continue;
			T next = cost + c;
			if (next >= d[to]) continue;
			if constexpr (Path) {
				if (next >= d[target]) continue;
				parent[to] = v;
			}
			d[to] = next;
			if constexpr (Path) if (to == target) continue;
			if (!g[to].empty()) push(std::move(next), to);
		}
	}
	return d;
}

template<bool Topological, class G, class T>
std::vector<T> distances(const G &g, int st, T inf) {
	std::vector<int> parent;
	auto d = [&] {
		if constexpr (Topological) return topological_run<false>(g, st, -1, inf, parent);
		else return run<false>(g, st, -1, inf, parent);
	}();
	for (T &val : d)
		if (val == inf) val = -1;
	return d;
}

template<bool Topological, class G, class T>
std::pair<T, std::vector<int>> path(const G &g, int st, int target, T inf) {
	[[maybe_unused]] const auto n = static_cast<std::size_t>(g.size());
	assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<std::size_t>(st) < n);
	assert(target >= 0 && static_cast<std::size_t>(target) < n);
	assert(inf > 0);
	if (st == target) return {T(0), {st}};
	if (g[st].empty()) return {T(-1), {}};
	if (g[st].size() == 1) {
		using Weight = std::remove_cvref_t<decltype(g[st][0].second)>;
		std::vector<int> direct{st};
		T cost = 0;
		int v = st, checkpoint = st;
		std::size_t power = 1, length = 0;
		while (g[v].size() == 1) {
			const auto &[to, weight] = g[v][0];
			assert(to >= 0 && static_cast<std::size_t>(to) < n);
			assert(weight >= 0);
			if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_bounded &&
						  std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
				using Compare = std::common_type_t<Weight, T>;
				if (Compare(weight) >= Compare(inf - cost)) return {T(-1), {}};
			} else if (weight >= inf - cost) return {T(-1), {}};
			cost += weight;
			v = to;
			direct.push_back(v);
			if (v == target) return {std::move(cost), std::move(direct)};
			if (v == checkpoint) return {T(-1), {}};
			if (++length == power) {
				checkpoint = v;
				power *= 2;
				length = 0;
			}
		}
		if (g[v].empty()) return {T(-1), {}};
		if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
			auto suffix = shortest_path_detail::path<Topological>(g, v, target, T(inf - cost));
			if (suffix.first == T(-1)) return {T(-1), {}};
			direct.pop_back();
			direct.insert(direct.end(), suffix.second.begin(), suffix.second.end());
			return {cost + suffix.first, std::move(direct)};
		}
	}
	std::vector<int> parent(g.size(), -1);
	if constexpr (requires { directed_csr_detail::Access::unique_incoming(g); })
		if (directed_csr_detail::Access::unique_incoming(g)) return unique_path(g, st, target, inf, parent);
	auto d = [&] {
		if constexpr (Topological) return topological_run<true>(g, st, target, inf, parent);
		else return run<true>(g, st, target, inf, parent);
	}();
	if (d[target] == inf) return {T(-1), {}};
	std::vector<int> path;
	for (int v = target; v != st; v = parent[v]) path.push_back(v);
	path.push_back(st);
	std::reverse(path.begin(), path.end());
	return {std::move(d[target]), std::move(path)};
}
}

template<bool Topological, class T>
std::vector<T> dijkstra(const std::vector<std::vector<std::pair<int, T>>> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	return shortest_path_detail::distances<Topological>(g, st, inf);
}

template<class T>
std::vector<T> dijkstra(const std::vector<std::vector<std::pair<int, T>>> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	return dijkstra<false>(g, st, inf);
}

template<bool Topological, class T>
std::pair<T, std::vector<int>> dijkstra_path(const std::vector<std::vector<std::pair<int, T>>> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	return shortest_path_detail::path<Topological>(g, st, target, inf);
}

template<class T>
std::pair<T, std::vector<int>> dijkstra_path(const std::vector<std::vector<std::pair<int, T>>> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	return dijkstra_path<false>(g, st, target, inf);
}

template<bool Topological, class W, class T = typename DirectedCSR<W>::distance_type>
std::vector<T> dijkstra(const DirectedCSR<W> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	return shortest_path_detail::distances<Topological>(g, st, inf);
}

template<class W, class T = typename DirectedCSR<W>::distance_type>
std::vector<T> dijkstra(const DirectedCSR<W> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	return dijkstra<false>(g, st, inf);
}

template<bool Topological, class W, class T = typename DirectedCSR<W>::distance_type>
std::pair<T, std::vector<int>> dijkstra_path(const DirectedCSR<W> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	if (directed_csr_detail::Access::has_single_edge_layout(g))
		return shortest_path_detail::path<Topological>(directed_csr_detail::Access::single_edge_view(g), st, target, inf);
	return shortest_path_detail::path<Topological>(g, st, target, inf);
}

template<class W, class T = typename DirectedCSR<W>::distance_type>
std::pair<T, std::vector<int>> dijkstra_path(const DirectedCSR<W> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	return dijkstra_path<false>(g, st, target, inf);
}


#line 1 "components/fast_io_bulk.hpp"



#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#line 15 "components/fast_io.hpp"
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


#line 8 "components/fast_io_bulk.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
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

	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		if (a < 1 || a > 10 || b <= a + 1 || b - a - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		const auto window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	static std::uint32_t one(const char *&p, bool &bad) {
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
		bad |= value > 0xffffffffULL;
		p += n + 1;
		return std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				const auto g0 = two(p[0], invalid), g1 = two(p[1], invalid), g2 = two(p[2], invalid), g3 = two(p[3], invalid);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, _mm256_set1_epi32(42)),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, _mm256_set1_epi32(42)), _mm256_cmpgt_epi32(low, _mm256_set1_epi32(94967295))));
				invalid = _mm256_or_si256(invalid, overflow);
				const auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
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

	template <class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one(s[k], bad);
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

	template <class Emit>
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
				const auto parsed = chunk_blocks(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (done + 2 <= count && in.end - p >= 32) {
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
				unsigned value = 0;
				in.read(value);
				emit(&value, 1);
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

namespace fast_io_detail {
template <class Emit, std::size_t... I>
void emit_record(Emit &emit, const std::uint32_t *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
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

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 8 "test/dijkstra_path.test.cpp"

#ifndef DIJKSTRA_USE_TOPOLOGICAL
#define DIJKSTRA_USE_TOPOLOGICAL false
#endif

int main() {
	FastInput in;
	FastOutput out;
	int n, m, s, t;
	in.read(n), in.read(m), in.read(s), in.read(t);
	using Graph = DirectedCSR<std::uint32_t>;
	Graph g(n, m, [&](auto emit) {
		read_unsigned_records<3>(in, m, [&](std::uint32_t from, std::uint32_t to, std::uint32_t weight) {
			emit(Graph::Edge{int(from), int(to), weight});
		});
	});
	auto [distance, path] = dijkstra_path<DIJKSTRA_USE_TOPOLOGICAL>(g, s, t);
	if (distance == -1) out.println(-1);
	else {
		out.write(distance), out.write(' '), out.println(path.size() - 1);
		write_unsigned_records<2>(out, path.size() - 1, [&](std::size_t i) {
			return std::array{std::uint32_t(path[i]), std::uint32_t(path[i + 1])};
		});
	}
}
