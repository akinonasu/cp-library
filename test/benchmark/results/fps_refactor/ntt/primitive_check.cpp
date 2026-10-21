#include "CompareBefore.hpp"
#include "CompareAfter.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>

using A = fps_detail::NTTBefore;
using B = fps_detail::NTTAfter;

void equal(const std::vector<unsigned> &a, const std::vector<unsigned> &b, const char *operation, int n, int columns) {
	for (std::size_t i = 0; i < a.size(); ++i) {
		if (A::out(a[i]) != B::out(b[i])) {
			std::fprintf(stderr, "%s n%d columns%d index%zu a%u b%u\n", operation, n, columns, i, a[i], b[i]);
			std::abort();
		}
	}
}

int main() {
	std::mt19937 random(20261021);
	for (int size = 2; size <= 65536; size *= 2) {
		A before(size);
		B after(size);
		for (int columns = 1; columns <= size; columns *= 2) {
			int rows = size / columns;
			std::vector<unsigned> a(2 * size), b;
			for (auto &value : a) value = random() % A::twice;
			b = a;
			before.project(a.data(), rows, columns);
			after.project(b.data(), rows, columns);
			equal(a, b, "project", size, columns);
			for (auto &value : a) value = random() % A::twice;
			b = a;
			before.composition_forward_y(a.data(), size, columns);
			after.composition_forward_y(b.data(), size, columns);
			equal(a, b, "forward_y", size, columns);
			if (columns >= 2 && columns <= size / 2) {
				for (bool inverse : {false, true}) {
					for (auto &value : a) value = random() % A::twice;
					b = a;
					before.double_y(a.data(), rows, columns, inverse);
					after.double_y(b.data(), rows, columns, inverse);
					equal(a, b, "double_y", size, columns);
				}
			}
		}
	}
	std::puts("NTT composition internal paths match through 65536");
}
