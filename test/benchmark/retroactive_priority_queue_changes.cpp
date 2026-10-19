#include "before.hpp"
#include "data_structure/priority_queue/partially_retroactive_priority_queue.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>

using Time = std::int64_t;
using Update = std::pair<Time, std::optional<long long>>;
struct Edit { int command; Time time; long long value; };

template <class Queue, bool Notify> void run(const std::vector<Update> &history, const std::vector<Edit> &edits) {
	long long sum = 0;
	std::uint64_t calls = 0;
	auto callback = [&](Time, const long long &value, bool added) { sum += added ? value : -value; ++calls; };
	auto start = std::chrono::steady_clock::now();
	Queue queue;
	if constexpr (Notify) { if (!queue.assign(history, callback)) std::abort(); }
	else { if (!queue.assign(history)) std::abort(); }
	queue.reserve(history.size() + edits.size() + 1);
	auto built = std::chrono::steady_clock::now();
	std::uint64_t checksum = 0;
	int accepted = 0;
	for (auto e : edits) {
		bool ok;
		if constexpr (Notify) {
			if (e.command == 0) ok = queue.insert(e.time, e.value, callback);
			else if (e.command == 1) ok = queue.insert_pop(e.time, callback);
			else if (e.command == 2) ok = queue.set(e.time, e.value, callback);
			else ok = queue.erase(e.time, callback);
		} else {
			if (e.command == 0) ok = queue.insert(e.time, e.value);
			else if (e.command == 1) ok = queue.insert_pop(e.time);
			else if (e.command == 2) ok = queue.set(e.time, e.value);
			else ok = queue.erase(e.time);
		}
		accepted += ok;
		checksum = checksum * 1000000007 + ok;
		checksum = checksum * 1000000007 + std::uint64_t(queue.top().value_or(-1));
		checksum = checksum * 1000000007 + queue.size();
	}
	auto end = std::chrono::steady_clock::now();
	if constexpr (Notify) {
		long long expected = 0;
		queue.for_each([&](Time, const long long &value) { expected += value; });
		if (sum != expected) std::abort();
	}
	std::printf("{\"build_ms\":%.6f,\"mixed_ms\":%.6f,\"bytes\":%zu,\"accepted\":%d,\"checksum\":\"%llu\",\"sum\":%lld,\"calls\":%llu}\n",
		std::chrono::duration<double, std::milli>(built - start).count(), std::chrono::duration<double, std::milli>(end - built).count(),
		queue.memory_usage(), accepted, (unsigned long long)checksum, sum, (unsigned long long)calls);
}

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	int variant = std::atoi(argv[1]), pattern = std::atoi(argv[2]), n = 100000;
	std::mt19937_64 rng(5834509);
	std::vector<Update> history;
	for (int i = 0; i < n; ++i) {
		bool push = pattern == 0 || i < (pattern == 1 ? n / 4 : 512) || (i & 1);
		history.emplace_back(Time(2) * i, push ? std::optional<long long>(rng() % 1000000000) : std::nullopt);
	}
	std::shuffle(history.begin(), history.end(), rng);
	std::vector<Edit> edits;
	for (int i = 0; i < 10000; ++i) {
		int command = rng() % 4;
		Time time = 2 * (rng() % n) + (command < 2 ? 1 : command == 2 ? 0 : int(rng() & 1));
		edits.push_back({command, time, static_cast<long long>(rng() % 1000000000)});
	}
	if (variant == 0) run<QueueBefore<long long>, false>(history, edits);
	else if (variant == 1) run<PartiallyRetroactivePriorityQueue<long long>, false>(history, edits);
	else run<PartiallyRetroactivePriorityQueue<long long>, true>(history, edits);
}
