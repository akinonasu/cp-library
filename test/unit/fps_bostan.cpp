#include "components/fps_bostan.hpp"
#include "components/fps_workspace.hpp"
#include <random>
#include <new>
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <sys/mman.h>
#include <unistd.h>

static void check_inverse_twisted() {
	using N = fps_detail::NTT;
	std::mt19937 random(92837);
	const std::size_t page = std::size_t(sysconf(_SC_PAGESIZE));
	for (int n = 8; n <= (1 << 18); n *= 2) {
		N plan(n, false);
		const std::size_t bytes = std::size_t(n) * sizeof(N::U), readable = (bytes + page - 1) / page * page;
		for (bool at_end : {false, true}) {
			void* source = mmap(nullptr, readable + 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			void* target = mmap(nullptr, readable + 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			assert(source != MAP_FAILED && target != MAP_FAILED);
			auto* source_begin = static_cast<char*>(source) + page;
			auto* target_begin = static_cast<char*>(target) + page;
			assert(mprotect(source, page, PROT_NONE) == 0 && mprotect(source_begin + readable, page, PROT_NONE) == 0);
			assert(mprotect(target, page, PROT_NONE) == 0 && mprotect(target_begin + readable, page, PROT_NONE) == 0);
			auto* input = ::new (source_begin + (at_end ? readable - bytes : 0)) N::U[n];
			auto* actual = ::new (target_begin + (at_end ? readable - bytes : 0)) N::U[n];
			auto expected = fps_detail::allocate_fps_words(n), powers = fps_detail::allocate_fps_words(n);
			for (int i = 0; i < n; ++i) {
				input[i] = expected[i] = i % 4 == 0 ? 2 * N::mod - 1 : random() % (2 * N::mod);
				powers[i] = i % 4 == 0 ? 0 : random() % (2 * N::mod);
			}
			assert(mprotect(source_begin, readable, PROT_READ) == 0);
			plan.fft<true>(expected.get(), n);
			N::product(expected.get(), expected.get(), powers.get(), n);
			fps_detail::bostan_detail::inverse_twisted(plan, input, actual, powers.get(), n);
			for (int i = 0; i < n; ++i) assert(actual[i] % N::mod == expected[i] % N::mod);
			assert(munmap(source, readable + 2 * page) == 0 && munmap(target, readable + 2 * page) == 0);
		}
	}
}
#endif

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (fps_detail::NTT::vectorized()) check_inverse_twisted();
#endif
}
