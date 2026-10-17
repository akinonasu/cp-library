#include "../fixtures/retroactive_scheduling.hpp"
#include <cstdlib>
#include <numeric>
#include <queue>
#include <random>
#include <tuple>

void require(bool ok) { if (!ok) std::abort(); }

long long greedy(const std::vector<int> &deadline, const std::vector<long long> &reward) {
	int n = deadline.size();
	std::vector<int> order(n);
	std::iota(order.begin(), order.end(), 0);
	std::sort(order.begin(), order.end(), [&](int a, int b) { return deadline[a] < deadline[b]; });
	std::priority_queue<long long, std::vector<long long>, std::greater<long long>> heap;
	long long sum = 0;
	for (int i : order) {
		heap.push(reward[i]); sum += reward[i];
		if (int(heap.size()) > deadline[i]) sum -= heap.top(), heap.pop();
	}
	return sum;
}

long long brute(const std::vector<int> &deadline, const std::vector<long long> &reward) {
	int n = deadline.size();
	long long answer = 0;
	for (int mask = 0; mask < (1 << n); ++mask) {
		std::vector<int> d;
		long long sum = 0;
		for (int i = 0; i < n; ++i) if (mask >> i & 1) d.push_back(deadline[i]), sum += reward[i];
		std::sort(d.begin(), d.end());
		bool valid = true;
		for (int i = 0; i < int(d.size()); ++i) if (i + 1 > d[i]) valid = false;
		if (valid) answer = std::max(answer, sum);
	}
	return answer;
}

int main() {
	RetroactiveScheduling first({1, 2, 3}, {3, 6, 3});
	require(first.set(2, 1, 4) == 10 && first.set(1, 3, 9) == 13);
	RetroactiveScheduling second({1, 2, 3, 4, 5}, std::vector<long long>(5, 1000000000));
	require(second.set(0, 1, 1000000000) == 5000000000LL);
	RetroactiveScheduling third({6, 2, 4, 1, 5, 1, 6, 6, 5, 3}, {45, 65, 71, 52, 86, 52, 48, 60, 40, 98});
	std::vector<std::tuple<int, int, long long, long long>> queries = {
		{5, 6, 5, 394}, {8, 4, 34, 379}, {6, 7, 83, 462}, {1, 3, 21, 457}, {7, 5, 85, 459},
		{7, 4, 51, 414}, {8, 2, 81, 443}, {2, 7, 54, 479}, {6, 1, 5, 401}, {8, 6, 30, 396}};
	for (auto [i, d, p, expected] : queries) require(third.set(i - 1, d, p) == expected);
	std::mt19937_64 rng(36310496);
	for (int trial = 0; trial < 100; ++trial) {
		int n = trial < 30 ? 1 + trial % 8 : 1 + rng() % 100;
		std::vector<int> d(n);
		std::vector<long long> p(n);
		for (int i = 0; i < n; ++i) d[i] = 1 + rng() % n, p[i] = 1 + rng() % 1000000000;
		RetroactiveScheduling scheduling(d, p);
		require(scheduling.answer() == greedy(d, p));
		for (int query = 0; query < 300; ++query) {
			int i = rng() % n;
			d[i] = 1 + rng() % n;
			p[i] = 1 + rng() % (query % 5 ? 1000000000 : 2);
			long long expected = greedy(d, p);
			if (n <= 8) require(expected == brute(d, p));
			require(scheduling.set(i, d[i], p[i]) == expected);
		}
	}
	int n = 100000;
	RetroactiveScheduling one(std::vector<int>(n, 1), std::vector<long long>(n, 1000000000));
	RetroactiveScheduling all(std::vector<int>(n, n), std::vector<long long>(n, 1000000000));
	for (int i = 0; i < 2000; ++i) {
		require(one.set(i, 1, 1) == 1000000000);
		require(all.set(i, n, 1) == 100000000000000LL - (i + 1LL) * 999999999);
	}
}
