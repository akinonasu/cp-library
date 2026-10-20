#include "data_structure/advanced/retroactive_priority_queue.hpp"
#include "forward.hpp"
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
struct Edit { int command; Time time; Value value; };
struct Result { double build, edit, top, size; std::size_t bytes; std::uint64_t checksum; int accepted; };
using Clock = std::chrono::steady_clock;
double elapsed(Clock::time_point begin) { return std::chrono::duration<double, std::milli>(Clock::now() - begin).count(); }

template <class Queue>
Result run(const std::vector<Update> &history, const std::vector<Edit> &edits, const std::vector<Time> &queries) {
	auto begin = Clock::now();
	Queue queue(history);
	queue.reserve(history.size() + edits.size());
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
	}
	double edit = elapsed(begin);
	begin = Clock::now();
	for (auto time : queries) checksum = checksum * 1000000007 + std::uint64_t(queue.top(time).value_or(-1));
	double top = elapsed(begin);
	begin = Clock::now();
	for (auto time : queries) checksum = checksum * 1000000007 + queue.size(time);
	double size = elapsed(begin);
	if (history.size() <= 2000) {
		std::map<Time, std::optional<Value>> timeline(history.begin(), history.end());
		for (auto e : edits) {
			auto old = timeline;
			if (e.command == 0 && !timeline.count(e.time)) timeline[e.time] = e.value;
			else if (e.command == 1 && !timeline.count(e.time)) timeline[e.time] = std::nullopt;
			else if (e.command == 2 && timeline.count(e.time) && timeline[e.time]) timeline[e.time] = e.value;
			else if (e.command == 3) timeline.erase(e.time);
			int count = 0;
			for (auto [t, value] : timeline) { (void)t; count += value ? 1 : -1; if (count < 0) break; }
			if (count < 0) timeline = std::move(old);
		}
		for (auto time : queries) {
			std::multiset<Value> values;
			for (auto [t, value] : timeline) {
				if (t > time) break;
				if (value) values.insert(*value);
				else values.erase(values.begin());
			}
			Value minimum = values.empty() ? Value(-1) : *values.begin();
			if (queue.top(time).value_or(-1) != minimum || queue.size(time) != int(values.size())) std::abort();
		}
	}
	return {build, edit, top, size, queue.memory_usage(), checksum, accepted};
}

template <int B> using Queue = RetroactivePriorityQueue<Value, std::less<Value>, Time, B>;
using Forward = ForwardPriorityQueue<Value, std::less<Value>, Time, 256>;
int main(int argc, char **argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 100000;
	int q = argc > 2 ? std::atoi(argv[2]) : 5000;
	int trials = argc > 3 ? std::atoi(argv[3]) : 5;
	std::mt19937_64 rng(5834509);
	using Fn = Result (*)(const std::vector<Update> &, const std::vector<Edit> &, const std::vector<Time> &);
	Fn functions[] = {run<Queue<64>>, run<Queue<128>>, run<Queue<256>>, run<Queue<512>>, run<Queue<1024>>, run<Forward>};
	const char *names[] = {"B64", "B128", "B256", "B512", "B1024", "forward256"};
	std::puts("workload,variant,trial,build_ms,edit_ms,top_ms,size_ms,bytes,accepted,checksum");
	for (int pattern = 0; pattern < 3; ++pattern) {
		std::vector<Update> history;
		for (int i = 0; i < n; ++i) {
			bool push = pattern == 0 || i < (pattern == 1 ? n / 4 : std::min(n, 512)) || (i & 1);
			history.emplace_back(Time(2) * i, push ? std::optional<Value>(rng() % 1000000000) : std::nullopt);
		}
		std::shuffle(history.begin(), history.end(), rng);
		std::vector<Edit> edits;
		std::vector<Time> queries;
		for (int i = 0; i < q; ++i) {
			int command = rng() % 4;
			Time time = 2 * (rng() % n) + (command < 2 ? 1 : command == 2 ? 0 : int(rng() & 1));
			edits.push_back({command, time, Value(rng() % 1000000000)});
			queries.push_back(rng() % (2 * n));
		}
		std::uint64_t expected = 0;
		for (int trial = 0; trial < trials; ++trial) {
			std::vector<int> order{0, 1, 2, 3, 4, 5};
			std::shuffle(order.begin(), order.end(), rng);
			for (int index : order) {
				auto r = functions[index](history, edits, queries);
				if (!trial && index == order[0]) expected = r.checksum;
				if (r.checksum != expected) std::abort();
				std::printf("%s,%s,%d,%.6f,%.6f,%.6f,%.6f,%zu,%d,%llu\n", pattern == 0 ? "growing" : pattern == 1 ? "balanced" : "steady", names[index], trial, r.build, r.edit, r.top, r.size, r.bytes, r.accepted, (unsigned long long)r.checksum);
			}
		}
	}
}
