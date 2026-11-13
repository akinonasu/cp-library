#include "data_structure/top_k_sum.hpp"
#include <cassert>
#include <cstdlib>
#include <new>

namespace {
int remaining_allocations = -1;
int allocations = 0;
std::size_t largest_allocation = 0;
bool track_allocations = false;
}

void *operator new(std::size_t size) {
	if (remaining_allocations >= 0 && !remaining_allocations--) throw std::bad_alloc();
	if (void *p = std::malloc(size ? size : 1)) {
		if (track_allocations) { ++allocations; largest_allocation = (std::max)(largest_allocation, size); }
		return p;
	}
	throw std::bad_alloc();
}
[[gnu::noinline]] void operator delete(void *p) noexcept {
	std::free(p);
}
void operator delete(void *p, std::size_t) noexcept { ::operator delete(p); }

struct Less {
	bool operator()(int a, int b) const { return a < b; }
};

template<class Compare>
void check_storage() {
	TopKSum<int, std::int64_t, Compare> set;
	for (int i = 0; i < 8192; ++i) {
		allocations = 0; largest_allocation = 0;
		track_allocations = true;
		set.insert(i);
		track_allocations = false;
		assert(allocations <= 4 && largest_allocation <= 4096);
	}
	allocations = 0; largest_allocation = 0;
	track_allocations = true;
	set.insert(0, 1000000000);
	track_allocations = false;
	assert(allocations <= 4 && largest_allocation <= 4096);
	for (int i = 1; i < 8192; ++i) assert(set.erase(i));
	allocations = 0; largest_allocation = 0;
	track_allocations = true;
	auto copy = set;
	track_allocations = false;
	assert(allocations <= 8 && largest_allocation <= 4096 && copy.size() == 1000000001);
	copy.clear();
	assert(copy.empty());
}

struct LifetimeSum {
	inline static int copies = 0, destroyed = 0;
	std::int64_t value;
	explicit LifetimeSum(std::int64_t x) : value(x) {}
	LifetimeSum(const LifetimeSum &other) : value(other.value) { ++copies; }
	LifetimeSum &operator=(const LifetimeSum &) = default;
	~LifetimeSum() { ++destroyed; }
	LifetimeSum &operator+=(const LifetimeSum &x) { value += x.value; return *this; }
	friend LifetimeSum operator*(const LifetimeSum &x, std::int64_t count) { return LifetimeSum(x.value * count); }
};

int main() {
	check_storage<std::less<int>>();
	check_storage<std::greater<int>>();
	check_storage<Less>();
	TopKSum<int, LifetimeSum> history;
	for (int i = 0; i < 8192; ++i) history.insert(i);
	for (int i = 1; i < 8192; ++i) assert(history.erase(i));
	LifetimeSum::copies = 0;
	auto live_copy = history;
	assert(LifetimeSum::copies <= 16 && live_copy.size() == 1);
	LifetimeSum::destroyed = 0;
	live_copy.clear();
	assert(LifetimeSum::destroyed <= 16 && live_copy.empty());
	for (int n : {32, 272, 4112}) {
		SmallestKSum<int> original;
		for (int i = 0; i < n; ++i) original.insert(i);
		for (int fail = 0; fail < 3; ++fail) {
			auto set = original;
			remaining_allocations = fail;
			bool inserted = false;
			try { set.insert(n); inserted = true; }
			catch (const std::bad_alloc &) {}
			remaining_allocations = -1;
			assert(set.size() == n + inserted);
			assert(set.count(n) == inserted);
			assert(set.kth(0) == 0 && set.kth(n - 1) == n - 1);
			assert(set.sum_all() == std::int64_t(n) * (n - 1) / 2 + (inserted ? n : 0));
			if (!inserted) set.insert(n);
			assert(set.erase(n));
			assert(set.size() == n && set.sum_all() == original.sum_all());
		}
	}
}
