#include "fps/taylor_shift.hpp"
#include <cassert>

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	const auto &table = fps_detail::taylor_factorials::kTable;
	static_assert(table.size() == 513 && table[0] == 1);
	std::uint64_t factorial = 1;
	for (int i = 1; i <= 524288; ++i) {
		factorial = factorial * i % 998244353;
		if (i % 1024 == 0) assert(table[i / 1024] == factorial);
	}
#endif
	for (int n : {255, 256, 257, 511, 512, 513}) {
		fps f(n);
		for (int i = 0; i < n; ++i) f[i] = i * i + 3;
		for (mint c : {mint(0), mint(1), mint(-1), mint(17)}) {
			fps expected(n);
			for (int i = n - 1; i >= 0; --i) {
				for (int j = n - 1; j > 0; --j) expected[j] = expected[j - 1] + c * expected[j];
				expected[0] = f[i] + c * expected[0];
			}
			assert(taylor_shift(f, c) == expected);
		}
	}
}
