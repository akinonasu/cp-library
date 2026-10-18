#include "fps/fps.hpp"
#include <cassert>
#include <cstdio>
#include <random>
#include <sys/mman.h>
#include <unistd.h>

int main() {
	std::mt19937 rng(731);
	for (int n : {1, 7, 8, 9, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 513, 1025, 4097, 16385})
		for (int size : {0, 1, 7, 8, 9, 31, n / 3, n, n + 17}) {
			fps f(size), q(size);
			for (int i = 0; i < size; ++i) {
				f[i] = fps_mint::word(2 * fps_mint::mod() - 1 - rng() % 10000);
				q[i] = f[i] * i;
				q[i].v = q[i].v % fps_mint::mod() + fps_mint::mod();
			}
			if (size) f[0] = q[0] = 0;
			const auto saved_f = f, saved_q = q;
			std::vector<fps_mint> a, b;
			fps_detail::blocked_poly::native_exp(f.data(), f.size(), a, n, false);
			fps_detail::blocked_poly::native_exp(q.data(), q.size(), b, n, true);
			assert(a == b);
			for (int i = 0; i < size; ++i) assert(f[i].v == saved_f[i].v && q[i].v == saved_q[i].v);
		}
	const auto page = std::size_t(sysconf(_SC_PAGESIZE));
	for (int size : {0, 1, 7, 8, 9, 31, 32, 33, 63, 64, 65, 257}) {
		void* region = mmap(nullptr, 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(region != MAP_FAILED);
		auto* ptr = reinterpret_cast<fps_mint*>(static_cast<char*>(region) + page - size * sizeof(fps_mint));
		for (int i = 0; i < size; ++i) ::new (static_cast<void*>(ptr + i)) fps_mint(fps_mint::word(2 * fps_mint::mod() - 1 - rng() % 10000));
		if (size) ptr[0] = 0;
		fps copied;
		for (int i = 0; i < size; ++i) copied.push_back(ptr[i]);
		assert(mprotect(region, page, PROT_READ) == 0);
		assert(mprotect(static_cast<char*>(region) + page, page, PROT_NONE) == 0);
		std::vector<fps_mint> a, b;
		fps_detail::blocked_poly::native_exp(ptr, size, a, 1025);
		fps_detail::blocked_poly::native_exp(copied.data(), copied.size(), b, 1025);
		assert(a == b);
		assert(munmap(region, 2 * page) == 0);
	}
	std::puts("exp import derivative/lazy/tail/readonly checks passed");
}
