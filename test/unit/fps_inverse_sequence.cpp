#include "components/ntt.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
#if defined(__x86_64__) || defined(__i386__)
	using N = fps_detail::NTT;
	if (!N::vectorized()) return 0;
	for (int n : {8, 24, 256, 4080, 4088, 4096, 4104, 4112, 4120, 4128, 8192, 65536, 262168})
		for (int first : {1, 17, 4194303, int(N::mod) - n})
			for (int offset : {1, 8}) {
				std::vector<std::uint32_t> result(n + offset + 1, 0xdeadbeef);
				N::inverse_sequence8(result.data() + offset, first, n);
				for (int i = 0; i < offset; ++i) assert(result[i] == 0xdeadbeef);
				assert(result.back() == 0xdeadbeef);
				for (int i = 0; i < n; ++i) {
					assert(result[offset + i] < N::twice);
					assert(std::uint64_t(N::out(result[offset + i])) * (first + i) % N::mod == 1);
				}
			}
#endif
}
