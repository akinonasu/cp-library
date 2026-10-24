#ifndef TWO_SAT_HPP
#define TWO_SAT_HPP

#include "../components/scc_core.hpp"

class TwoSAT {
	int n_;
	std::vector<std::pair<int, int>> edges_;
	std::vector<int> forced_;
	std::vector<bool> answer_;
	bool valid_ = false;
	int closure() {
		std::vector<int> head(2 * n_, -1), next(edges_.size()), trail;
		std::vector<signed char> values(n_, -1);
		trail.reserve(n_);
		for (std::size_t i = 0; i < edges_.size(); ++i) {
			int from = edges_[i].first;
			next[i] = head[from];
			head[from] = static_cast<int>(i);
		}
		std::size_t budget = (std::min)(std::size_t(n_) + edges_.size(), std::numeric_limits<std::size_t>::max() / 2) * 2;
		auto propagate = [&](int root) {
			trail.clear();
			trail.push_back(root);
			values[root >> 1] = root & 1;
			for (std::size_t i = 0; i < trail.size(); ++i) {
				if (!budget) return -1;
				--budget;
				for (int at = head[trail[i]]; at >= 0; at = next[at]) {
					if (!budget) return -1;
					--budget;
					int to = edges_[at].second;
					if (values[to >> 1] < 0) {
						values[to >> 1] = to & 1;
						trail.push_back(to);
					} else if (values[to >> 1] != (to & 1)) {
						for (int v : trail) values[v >> 1] = -1;
						return 0;
					}
				}
			}
			return 1;
		};
		for (int to : forced_) {
			if (values[to >> 1] < 0) {
				int result = propagate(to);
				if (result != 1) return result;
			} else if (values[to >> 1] != (to & 1)) return 0;
		}
		for (int i = 0; i < n_; ++i) if (values[i] < 0) {
			if (head[2 * i] < 0) values[i] = 0;
			else if (head[2 * i + 1] < 0) values[i] = 1;
			else {
				int result = propagate(2 * i);
				if (result < 0) return -1;
				if (!result) {
					result = propagate(2 * i + 1);
					if (result != 1) return result;
				}
			}
		}
		for (int i = 0; i < n_; ++i) answer_[i] = values[i];
		return 1;
	}
	static int vertices(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		return n * 2;
	}

  public:
	explicit TwoSAT(int n = 0) : n_(n), answer_(vertices(n) / 2) {}
	int size() const { return n_; }
	void reserve(std::size_t clauses) {
		assert(clauses <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(clauses * 2);
	}
	void add_clause(int i, bool f, int j, bool g) {
		assert(0 <= i && i < n_ && 0 <= j && j < n_);
		valid_ = false;
		if (i == j && f != g) return;
		assert(edges_.size() <= std::size_t(std::numeric_limits<int>::max() - 1 - int(i != j)));
		edges_.emplace_back(2 * i + !f, 2 * j + g);
		if (i == j) forced_.push_back(2 * i + f);
		if (i != j) edges_.emplace_back(2 * j + !g, 2 * i + f);
	}
	bool satisfiable() {
		valid_ = false;
		if (edges_.empty()) {
			std::fill(answer_.begin(), answer_.end(), false);
			return valid_ = true;
		}
		if (n_ >= 1024) {
			int result = closure();
			if (result >= 0) return valid_ = result;
		}
		auto [count, id] = scc_detail::ids<true>(2 * n_, edges_);
		if (count < 0) return false;
		for (int i = 0; i < n_; ++i) {
			if (id[2 * i] == id[2 * i + 1]) return false;
			answer_[i] = id[2 * i] < id[2 * i + 1];
		}
		return valid_ = true;
	}
	std::vector<bool> answer() const { assert(valid_); return answer_; }
};

#endif
