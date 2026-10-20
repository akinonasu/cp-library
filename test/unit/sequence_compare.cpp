#include "components/sequence_compare.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <unistd.h>
#endif

template<class T>
void check(const T *a, const T *b, int n) {
	int forward = 0, backward = 0;
	while (forward < n && a[forward] == b[forward]) ++forward;
	while (backward < n && a[n - 1 - backward] == b[n - 1 - backward]) ++backward;
	assert(sequence_compare_detail::prefix(a, b, n) == forward);
	assert(sequence_compare_detail::prefix<false>(a, b, n) == forward);
	if (!n) {
		assert(sequence_compare_detail::suffix(a, b, 0) == 0);
		assert(sequence_compare_detail::suffix<false>(a, b, 0) == 0);
		return;
	}
	assert(sequence_compare_detail::suffix(a + n, b + n, n) == backward);
	assert(sequence_compare_detail::suffix<false>(a + n, b + n, n) == backward);
}

template<class T>
void typed() {
	std::mt19937_64 random(20261019);
	for (int n : {1, 2, 3, 7, 8, 9, 31, 32, 33, 63, 64, 65, 71, 72, 73, 127, 128, 129, 257, 4096}) {
		std::vector<T> a(n), b(n);
		for (T &value : a) value = T(random());
		b = a;
		check(a.data(), b.data(), n);
		for (int position : {0, n / 2, n - 1}) {
			b[position] = T(a[position] ^ T(1));
			check(a.data(), b.data(), n);
			b[position] = a[position];
		}
	}
}

struct Symbol {
	int value;
	friend bool operator==(Symbol a, Symbol b) { return a.value % 2 == b.value % 2; }
};

int main() {
	check<char>(nullptr, nullptr, 0);
	typed<char>(); typed<std::uint32_t>(); typed<std::uint64_t>(); typed<__uint128_t>();
	float floats[] = {-0.0f, 1.0f, 2.0f}, other[] = {0.0f, 1.0f, 2.0f};
	check(floats, other, 3);
	Symbol symbols[] = {{0}, {1}, {2}}, equivalent[] = {{2}, {3}, {4}};
	check(symbols, equivalent, 3);
	for (int n : {1, 7, 8, 9, 31, 32, 33, 63, 64, 65, 71, 72, 73, 95, 96, 97, 127, 128, 129, 257}) {
		for (int offset = 0; offset < 32; ++offset) {
			std::vector<unsigned char> a(n + 32, 0xff), b(n + 32, 0xff);
			for (int mismatch = 0; mismatch <= n; ++mismatch) {
				if (mismatch < n) b[31 - offset + mismatch] = 0;
				check(a.data() + offset, b.data() + 31 - offset, n);
				if (mismatch < n) b[31 - offset + mismatch] = 0xff;
			}
		}
	}
#if defined(__unix__) || defined(__APPLE__)
	long page = sysconf(_SC_PAGESIZE);
	assert(page >= 4096);
	void *first = mmap(nullptr, 3 * page, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	void *second = mmap(nullptr, 3 * page, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(first != MAP_FAILED && second != MAP_FAILED);
	auto *a = static_cast<unsigned char *>(first) + page, *b = static_cast<unsigned char *>(second) + page;
	int ok = mprotect(a, page, PROT_READ | PROT_WRITE) | mprotect(b, page, PROT_READ | PROT_WRITE);
	assert(ok == 0);
	std::fill_n(a, page, 0xff), std::fill_n(b, page, 0xff);
	for (int n : {1, 7, 8, 31, 32, 63, 64, 71, 72, 73, 127, 128, 129, 257, 4096}) {
		for (bool end_a : {false, true}) for (bool end_b : {false, true}) {
			auto *x = a + (end_a ? page - n : 0), *y = b + (end_b ? page - n : 0);
			check(x, y, n);
			for (int position : {0, n / 2, n - 1}) {
				y[position] = 0;
				check(x, y, n);
				y[position] = 0xff;
			}
		}
	}
	int unmap_first = munmap(first, 3 * page), unmap_second = munmap(second, 3 * page);
	assert(unmap_first == 0 && unmap_second == 0);
#endif
}
