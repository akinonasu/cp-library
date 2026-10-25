#ifndef MO_WITH_UPDATES_HPP
#define MO_WITH_UPDATES_HPP

#include "mo_order.hpp"
#include <array>

class MoWithUpdates {
public:
	struct Query { int l, r, time; };

private:
	int n_;
	std::vector<int> positions_;
	std::vector<Query> queries_;

public:
	explicit MoWithUpdates(int n = 0) : n_(n) { assert(n >= 0); }
	int size() const { return n_; }
	int query_count() const { return static_cast<int>(queries_.size()); }
	int update_count() const { return static_cast<int>(positions_.size()); }
	int update_position(int id) const {
		assert(0 <= id && id < update_count());
		return positions_[id];
	}
	Query query(int id) const {
		assert(0 <= id && id < query_count());
		return queries_[id];
	}
	int add_update(int p) {
		assert(0 <= p && p < n_ && positions_.size() < std::size_t(std::numeric_limits<int>::max()));
		positions_.push_back(p);
		return update_count() - 1;
	}
	int add_query(int l, int r) {
		assert(0 <= l && l <= r && r <= n_ && queries_.size() < std::size_t(std::numeric_limits<int>::max()));
		queries_.push_back({l, r, update_count()});
		return query_count() - 1;
	}
	std::vector<int> order(int block_size = 0) const {
		assert(block_size >= 0);
		if (queries_.empty()) return {};
		if (positions_.empty()) {
			std::vector<std::pair<int, int>> ranges;
			ranges.reserve(queries_.size());
			for (auto q : queries_) ranges.emplace_back(q.l, q.r);
			return mo_order(ranges, block_size);
		}
		if (!block_size) block_size = std::max(1, static_cast<int>(std::min(double(n_), std::ceil(std::cbrt(double(n_) * n_ * update_count() / query_count())))));
		struct Key { int left, right, time, id; };
		std::vector<Key> keys;
		keys.reserve(queries_.size());
		for (int i = 0; i < query_count(); ++i) {
			auto q = queries_[i];
			int left = q.l / block_size, right = q.r / block_size;
			keys.push_back({left, left & 1 ? -right : right, (left ^ right) & 1 ? -q.time : q.time, i});
		}
		std::sort(keys.begin(), keys.end(), [](const Key &a, const Key &b) {
			return std::array{a.left, a.right, a.time, a.id} < std::array{b.left, b.right, b.time, b.id};
		});
		std::vector<int> result;
		result.reserve(keys.size());
		for (auto k : keys) result.push_back(k.id);
		return result;
	}
	template<class AddLeft, class AddRight, class EraseLeft, class EraseRight, class Apply, class Rollback, class Answer>
	void run(AddLeft add_left, AddRight add_right, EraseLeft erase_left, EraseRight erase_right,
			 Apply apply, Rollback rollback, Answer answer, int block_size = 0) const {
		int l = 0, r = 0, time = 0;
		for (int id : order(block_size)) {
			auto q = queries_[id];
			while (time < q.time) {
				int p = positions_[time];
				apply(time++, l <= p && p < r);
			}
			while (q.time < time) {
				int p = positions_[--time];
				rollback(time, l <= p && p < r);
			}
			while (q.l < l) add_left(--l);
			while (r < q.r) add_right(r++);
			while (l < q.l) erase_left(l++);
			while (q.r < r) erase_right(--r);
			answer(id);
		}
	}
	template<class Add, class Erase, class Apply, class Rollback, class Answer>
	void run(Add add, Erase erase, Apply apply, Rollback rollback, Answer answer, int block_size = 0) const {
		run(add, add, erase, erase, apply, rollback, answer, block_size);
	}
};

#endif
