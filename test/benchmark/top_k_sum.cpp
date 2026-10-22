#include "data_structure/top_k_sum.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#ifdef TOP_K_REFERENCE_HEADER
#include TOP_K_REFERENCE_HEADER
#endif

using Value = long long;

struct TreeBaseline {
	std::multiset<Value> lower, upper;
	std::size_t k;
	Value total = 0;
	explicit TreeBaseline(std::size_t k) : k(k) {}
	void balance() {
		while (upper.size() > k) {
			auto it = upper.begin();
			total -= *it;
			lower.insert(*it);
			upper.erase(it);
		}
		while (upper.size() < k && !lower.empty()) {
			auto it = std::prev(lower.end());
			total += *it;
			upper.insert(*it);
			lower.erase(it);
		}
	}
	void insert(Value value) {
		if (upper.empty() || value >= *upper.begin()) {
			upper.insert(value);
			total += value;
		} else lower.insert(value);
		balance();
	}
	void erase(Value value) {
		auto it = upper.find(value);
		if (it != upper.end()) {
			total -= *it;
			upper.erase(it);
		} else lower.erase(lower.find(value));
		balance();
	}
	void replace(Value old_value, Value new_value) { erase(old_value); insert(new_value); }
	Value sum() const { return total; }
};

#ifdef TOP_K_REFERENCE_HEADER
struct HeapReference : MaximumSum<Value> {
	using MaximumSum<Value>::MaximumSum;
	void replace(Value old_value, Value new_value) { erase(old_value); insert(new_value); }
	Value sum() const { return query(); }
};
#endif

template<class Set>
void run(const std::string &scenario) {
	constexpr int n = 200000, q = 200000;
	const int length = scenario == "long_stream" ? 4096 : n;
	std::mt19937_64 rng(306281);
	std::vector<Value> initial(length), updates(q);
	std::vector<int> positions(q);
	for (auto &value : initial) value = static_cast<Value>(rng() % 1000000000);
	for (int i = 0; i < q; ++i) {
		positions[i] = scenario == "window" ? i % length : rng() % length;
		updates[i] = scenario == "long_stream" ? Value(1000000000) + i : rng() % 1000000000;
		if (scenario == "duplicates") updates[i] %= 16;
	}
	if (scenario == "duplicates") for (auto &value : initial) value %= 16;
	auto values = initial;
	const auto start = std::chrono::steady_clock::now();
	Set set(length / 2);
	for (auto value : initial) set.insert(value);
	std::uint64_t checksum = 0;
	for (int i = 0; i < q; ++i) {
		const auto position = positions[i];
		set.replace(values[position], updates[i]);
		values[position] = updates[i];
		checksum = checksum * 1000003 + static_cast<std::uint64_t>(set.sum());
	}
	const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	std::cout << ms << ' ' << checksum << '\n';
}

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	const std::string variant = argv[1], scenario = argv[2];
	if (variant == "library") run<TopKSum<Value>>(scenario);
	else if (variant == "tree") run<TreeBaseline>(scenario);
#ifdef TOP_K_REFERENCE_HEADER
	else if (variant == "heap") run<HeapReference>(scenario);
#endif
	else return 1;
}
