#include "components/modular_power_sequence.hpp"
#include <cassert>
#include <random>

int main() {
	std::mt19937 rng(47823);
	for (unsigned mod : {1U, 2U, 17U, 257U, 65537U, 998244353U, 2147483647U, 2147483648U, 4294967295U}) {
		for (std::size_t n : {0, 1, 7, 8, 9, 15, 16, 17, 255, 256, 257, 263, 1023, 1024, 1025}) {
			for (unsigned root : {0U, 1U, mod - 1, unsigned(rng())}) {
				auto result = modular_power_sequence_detail::generate(n, root, mod);
				assert(result.size() == n);
				std::uint64_t value = 1 % mod;
				for (auto x : result) {
					assert(x == value && x < mod);
					value = value * root % mod;
				}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
				if (mod < (1U << 31) && __builtin_cpu_supports("avx2")) {
					std::vector<unsigned> guarded(n + 2, 0xdeadbeef);
					modular_power_sequence_detail::generate(guarded.data() + 1, n, root, Barrett32(mod));
					assert(guarded.front() == 0xdeadbeef && guarded.back() == 0xdeadbeef);
					assert(std::equal(result.begin(), result.end(), guarded.begin() + 1));
				}
#endif
			}
		}
	}
}
