#include "data_structure/advanced/partially_retroactive_priority_queue.hpp"
#include "data_structure/advanced/retroactive_priority_queue.hpp"
#include "separate_incremental.hpp"
#include "separate_bulk.hpp"
#include "fused_bulk.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <optional>
#include <random>
#include <set>
#include <vector>

using Value = long long;
using Time = std::int64_t;
using Update = std::pair<Time, std::optional<Value>>;
using Clock = std::chrono::steady_clock;
struct Edit { int command; Time time; Value value; };
struct Result { double build, mixed; std::size_t bytes; std::uint64_t checksum; int accepted; };
double elapsed(Clock::time_point begin) { return std::chrono::duration<double, std::milli>(Clock::now() - begin).count(); }

template <class Queue>
Result run(const std::vector<Update> &history, const std::vector<Edit> &edits) {
	auto begin = Clock::now();
	Queue queue(history);
	queue.reserve(history.size() + edits.size() + 1);
	double build = elapsed(begin);
	int accepted = 0;
	std::uint64_t checksum = 0;
	begin = Clock::now();
	for (auto e : edits) {
		bool ok;
		if (e.command == 0) ok = queue.insert(e.time, e.value);
		else if (e.command == 1) ok = queue.insert_pop(e.time);
		else if (e.command == 2) ok = queue.set(e.time, e.value);
		else ok = queue.erase(e.time);
		accepted += ok;
		checksum = checksum * 1000000007 + ok;
		checksum = checksum * 1000000007 + std::uint64_t(queue.top().value_or(-1));
		checksum = checksum * 1000000007 + queue.size();
	}
	double mixed = elapsed(begin);
	if (history.size() <= 2000) {
		std::map<Time, std::optional<Value>> timeline(history.begin(), history.end());
		for (auto e : edits) {
			auto old = timeline;
			if (e.command < 2 && !timeline.count(e.time)) timeline[e.time] = e.command ? std::nullopt : std::optional<Value>(e.value);
			else if (e.command == 2 && timeline.count(e.time) && timeline[e.time]) timeline[e.time] = e.value;
			else if (e.command == 3) timeline.erase(e.time);
			int count = 0;
			for (auto [t, value] : timeline) { (void)t; count += value ? 1 : -1; if (count < 0) break; }
			if (count < 0) timeline = std::move(old);
		}
		std::multiset<Value> values;
		for (auto [t, value] : timeline) {
			(void)t;
			if (value) values.insert(*value);
			else values.erase(values.begin());
		}
		Value top = values.empty() ? Value(-1) : *values.begin();
		if (queue.top().value_or(-1) != top || queue.size() != int(values.size())) std::abort();
	}
	return {build, mixed, queue.memory_usage(), checksum, accepted};
}

int main(int argc, char **argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 100000;
	int q = argc > 2 ? std::atoi(argv[2]) : 10000;
	int trials = argc > 3 ? std::atoi(argv[3]) : 5;
	std::mt19937_64 rng(5834509);
	using Fn = Result (*)(const std::vector<Update> &, const std::vector<Edit> &);
	Fn functions[] = {run<Queue_separate_incremental<Value>>, run<Queue_separate_bulk<Value>>, run<Queue_fused_bulk<Value>>, run<PartiallyRetroactivePriorityQueue<Value>>, run<RetroactivePriorityQueue<Value>>};
	const char *names[] = {"separate_incremental", "separate_bulk", "fused_bulk", "current", "fully256"};
	std::puts("workload,variant,trial,build_ms,mixed_ms,bytes,accepted,checksum");
	for (int pattern = 0; pattern < 3; ++pattern) {
		std::vector<Update> history;
		for (int i = 0; i < n; ++i) {
			bool push = pattern == 0 || i < (pattern == 1 ? n / 4 : std::min(n, 512)) || (i & 1);
			history.emplace_back(Time(2) * i, push ? std::optional<Value>(rng() % 1000000000) : std::nullopt);
		}
		std::shuffle(history.begin(), history.end(), rng);
		std::vector<Edit> edits;
		for (int i = 0; i < q; ++i) {
			int command = rng() % 4;
			Time time = 2 * (rng() % n) + (command < 2 ? 1 : command == 2 ? 0 : int(rng() & 1));
			edits.push_back({command, time, Value(rng() % 1000000000)});
		}
		std::uint64_t expected = 0;
		for (int trial = 0; trial < trials; ++trial) {
			std::vector<int> order{0, 1, 2, 3, 4};
			std::shuffle(order.begin(), order.end(), rng);
			for (int variant : order) {
				auto result = functions[variant](history, edits);
				if (expected && expected != result.checksum) std::abort();
				expected = result.checksum;
				std::printf("%s,%s,%d,%.6f,%.6f,%zu,%d,%llu\n", pattern == 0 ? "growing" : pattern == 1 ? "balanced" : "steady", names[variant], trial, result.build, result.mixed, result.bytes, result.accepted, (unsigned long long)result.checksum);
				std::fflush(stdout);
			}
		}
	}
}
