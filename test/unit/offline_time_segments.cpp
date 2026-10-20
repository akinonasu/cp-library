#include "components/offline_time_segments.hpp"
#include <cstdint>
#include <cstdlib>
#include <random>
#include <vector>

void require(bool value) { if (!value) std::abort(); }

struct State {
	std::int64_t sum = 17;
	std::vector<int> history{17};
	std::size_t snapshot() const { return history.size(); }
	void rollback(std::size_t saved) {
		while (history.size() > saved) sum -= history.back(), history.pop_back();
	}
	void add(int x) { sum += x; history.push_back(x); }
};

int main() {
	std::mt19937 rng(20261020);
	for (int n = 0; n <= 70; ++n) {
		std::vector<OfflineTimeSegments::Interval> intervals;
		std::vector<std::int64_t> expected(n, 17);
		for (int i = 0; i < 200; ++i) {
			int l = rng() % (n + 1), r = rng() % (n + 1), value = int(rng() % 201) - 100;
			if (l > r) std::swap(l, r);
			intervals.push_back({l, r, value});
			for (int t = l; t < r; ++t) expected[t] += value;
		}
		OfflineTimeSegments timeline(n, intervals);
		require(timeline.size() == n);
		State state;
		for (int repeat = 0; repeat < 2; ++repeat) {
			int time = 0;
			timeline.run(state, [](State &s, int value) { s.add(value); }, [&](int t, const State &s) {
				require(t == time++ && s.sum == expected[t]);
			});
			require(time == n && state.sum == 17 && state.history == std::vector<int>({17}));
		}
		OfflineTimeSegments empty(n, {});
		require(empty.entry_count() == 0);
		int visits = 0;
		empty.run(state, [](State &, int) { std::abort(); }, [&](int t, const State &s) {
			require(t == visits++ && s.sum == 17);
		});
		require(visits == n);
	}
}
