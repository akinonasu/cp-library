#ifndef TEST_RETROACTIVE_SCHEDULING_HPP
#define TEST_RETROACTIVE_SCHEDULING_HPP

#include "../../data_structure/priority_queue/partially_retroactive_priority_queue.hpp"

class RetroactiveScheduling {
	using Queue = PartiallyRetroactivePriorityQueue<long long, std::greater<long long>>;
	Queue queue;
	int n;
	std::vector<int> deadline;
	std::vector<long long> reward;
	long long total = 0, remaining = 0;
	std::int64_t time(int i) const { return n + std::int64_t(n - deadline[i]) * (n + 1) + i; }
public:
	RetroactiveScheduling(std::vector<int> d, std::vector<long long> p)
		: n(d.size()), deadline(std::move(d)), reward(std::move(p)) {
		std::vector<Queue::update_type> history;
		history.reserve(3 * n);
		for (int i = 0; i < n; ++i) {
			total += reward[i];
			history.emplace_back(i, 0);
			history.emplace_back(time(i), reward[i]);
			history.emplace_back(n + std::int64_t(i) * (n + 1) + n, std::nullopt);
		}
		[[maybe_unused]] bool ok = queue.assign(history, [&](std::int64_t, const long long &value, bool added) { remaining += added ? value : -value; });
		assert(ok);
	}
	long long answer() const { return total - remaining; }
	long long set(int i, int d, long long p) {
		auto on_change = [&](std::int64_t, const long long &value, bool added) { remaining += added ? value : -value; };
		[[maybe_unused]] bool ok;
		if (deadline[i] == d) ok = queue.set(time(i), p, on_change);
		else {
			ok = queue.erase(time(i), on_change);
			assert(ok);
			deadline[i] = d;
			ok = queue.insert(time(i), p, on_change);
		}
		assert(ok);
		total += p - reward[i];
		reward[i] = p;
		return answer();
	}
};

#endif
