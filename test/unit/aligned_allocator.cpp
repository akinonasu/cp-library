#include "components/aligned_allocator.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>

int main() {
	using Allocator = AlignedAllocator<std::int64_t>;
	std::vector<std::int64_t, Allocator> values(100, 17);
	assert(reinterpret_cast<std::uintptr_t>(values.data()) % 64 == 0);
	values.resize(1000, -9);
	assert(reinterpret_cast<std::uintptr_t>(values.data()) % 64 == 0);
	auto copy = values;
	copy[0] = 42;
	assert(values[0] == 17 && copy[0] == 42 && copy[100] == -9);
	values.swap(copy);
	assert(values[0] == 42 && copy[0] == 17);
	struct alignas(128) Wide { std::uint64_t value; };
	std::vector<Wide, AlignedAllocator<Wide, 128>> wide(2);
	assert(reinterpret_cast<std::uintptr_t>(wide.data()) % 128 == 0);
	wide[1].value = 10;
	assert(wide[0].value == 0 && wide[1].value == 10);
	Allocator allocator;
	bool caught = false;
	try { allocator.allocate(std::numeric_limits<std::size_t>::max()); }
	catch (const std::bad_array_new_length &) { caught = true; }
	assert(caught);
	AlignedAllocator<char> other(allocator);
	assert(other == AlignedAllocator<char>{});
}
