#include "data_structure/top_k_sum.hpp"
#include <cassert>
#include <cstdlib>
#include <new>

namespace {
int remaining_allocations = -1;
}

void *operator new(std::size_t size) {
	if (remaining_allocations >= 0 && !remaining_allocations--) throw std::bad_alloc();
	if (void *p = std::malloc(size ? size : 1)) return p;
	throw std::bad_alloc();
}
void operator delete(void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }

int main() {
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
