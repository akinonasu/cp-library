#include "data_structure/interval_set.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

class Baseline {
	std::map<std::int64_t, std::int64_t> ranges;
	std::uint64_t total = 0;
public:
	std::uint64_t insert(std::int64_t l, std::int64_t r) {
		if (l == r) return 0;
		auto it = ranges.lower_bound(l);
		if (it != ranges.begin() && std::prev(it)->second >= l) --it;
		auto old = total;
		while (it != ranges.end() && it->first <= r) {
			l = (std::min)(l, it->first); r = (std::max)(r, it->second);
			total -= it->second - it->first;
			it = ranges.erase(it);
		}
		ranges.emplace(l, r); total += r - l;
		return total - old;
	}
	std::uint64_t erase(std::int64_t l, std::int64_t r) {
		if (l == r) return 0;
		auto it = ranges.lower_bound(l);
		if (it != ranges.begin() && std::prev(it)->second > l) --it;
		auto old = total;
		while (it != ranges.end() && it->first < r) {
			auto [a, b] = *it;
			it = ranges.erase(it); total -= b - a;
			if (a < l) { ranges.emplace(a, l); total += l - a; }
			if (r < b) { ranges.emplace(r, b); total += b - r; break; }
		}
		return old - total;
	}
	bool contains(std::int64_t x) const {
		auto it = ranges.upper_bound(x);
		return it != ranges.begin() && x < std::prev(it)->second;
	}
	std::optional<std::int64_t> next(std::int64_t x) const {
		auto it = ranges.upper_bound(x);
		if (it != ranges.begin() && x < std::prev(it)->second) return x;
		if (it == ranges.end()) return std::nullopt;
		return it->first;
	}
	std::optional<std::int64_t> prev(std::int64_t x) const {
		auto it = ranges.upper_bound(x);
		if (it == ranges.begin()) return std::nullopt;
		return (std::min)(x, std::prev(it)->second - 1);
	}
	std::int64_t mex(std::int64_t x) const {
		auto it = ranges.upper_bound(x);
		return it != ranges.begin() && x < std::prev(it)->second ? std::prev(it)->second : x;
	}
	std::uint64_t size() const { return total; }
	auto begin() const { return ranges.begin(); }
	auto end() const { return ranges.end(); }
};

struct Query { std::int64_t l, r; unsigned type; };

template<class Set>
void run(const std::string &workload, const std::vector<Query> &queries) {
	Set set;
	if (workload == "split") set.insert(0, 1000000000000LL);
	if (workload == "sliding") set.insert(0, 1000000);
	if (workload == "query" || workload == "fragmented")
		for (int i = 0; i < 100000; ++i) set.insert(i * 10, i * 10 + 4);
	std::uint64_t checksum = 0;
	auto mix = [&](std::uint64_t value) { checksum = (checksum ^ value) * 0x9e3779b97f4a7c15ULL; };
	auto start = std::chrono::steady_clock::now();
	for (auto [l, r, type] : queries) {
		if (workload == "split") { mix(set.erase(l, r)); mix(set.insert(l, r)); }
		else if (workload == "sliding") { mix(set.insert(l, r)); mix(set.erase(0, l)); }
		else if (type == 0) mix(set.insert(l, r));
		else if (type == 1) mix(set.erase(l, r));
		else if (type == 2) mix(set.contains(l));
		else if (type == 3) mix(set.next(l).value_or(-1));
		else if (type == 4) mix(set.prev(l).value_or(-1));
		else mix(set.mex(l));
	}
	auto stop = std::chrono::steady_clock::now();
	mix(set.size());
	for (auto [l, r] : set) { mix(l); mix(r); }
	std::printf("%.9f\n%llu\n", std::chrono::duration<double>(stop - start).count(),
		static_cast<unsigned long long>(checksum));
}

int main(int argc, char **argv) {
	if (argc != 4) return 1;
	std::string variant = argv[1], workload = argv[2];
	int count = std::atoi(argv[3]);
	std::mt19937_64 rng(731);
	std::vector<Query> queries;
	for (int i = 0; i < count; ++i) {
		std::int64_t l = rng() % 1000000, r = l + 1;
		unsigned type = rng() % 6;
		if (workload == "grow") { l %= 100000; r = l + 1; type = 0; }
		if (workload == "range") { r = l + rng() % 257; type %= 2; }
		if (workload == "split") { l = rng() % 999999000000LL; r = l + 1000; }
		if (workload == "sliding") { l = static_cast<std::int64_t>(i + 1) * 17; r = l + 1000000; }
		if (workload == "query") type = 2 + rng() % 4;
		queries.push_back({l, r, type});
	}
	if (variant == "baseline") run<Baseline>(workload, queries);
	else if (variant == "library") run<IntervalSet<>>(workload, queries);
	else return 1;
}
