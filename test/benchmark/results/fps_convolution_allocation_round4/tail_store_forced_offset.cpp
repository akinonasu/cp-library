#include "components/fps_convolution.hpp"
#include <cstdlib>
#include <new>
#include <random>
#include <sys/mman.h>
#include <unistd.h>

static void require(bool value) { if (!value) std::abort(); }

int main() {
	if (!__builtin_cpu_supports("avx2")) return 0;
	using namespace fps_detail::multiply_detail;
	using M = fps_detail::Mint;
	std::mt19937 random(20261042);
	const std::size_t page = std::size_t(sysconf(_SC_PAGESIZE));
	for (const auto& [n, m] : {std::pair{257, 257}, {511, 512}, {512, 512}, {4097, 4097}, {131072, 131072}, {524288, 1000}}) {
		require(Product::fits(n, m));
		std::vector<M> a(n), b(m);
		for (auto& value : a) value = M::word(M::mod() + random() % M::mod());
		for (auto& value : b) value = M::word(M::mod() + random() % M::mod());
		const auto expected = run<Product>(a, b);
		const std::size_t len = std::bit_ceil(a.size() + b.size() - 1);
		for (std::size_t offset = 0; offset < 8; ++offset) {
			const std::size_t words = len + 16 + offset, bytes = words * sizeof(M), readable = (bytes + page - 1) / page * page;
			void* region = mmap(nullptr, readable + 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			require(region != MAP_FAILED);
			auto* const end = static_cast<char*>(region) + page + readable;
			auto* const out = ::new (static_cast<void*>(end - bytes)) M[words];
			auto* const borrowed = out + offset;
			require(reinterpret_cast<std::uintptr_t>(borrowed) % 32 == 0);
			require(mprotect(region, page, PROT_NONE) == 0 && mprotect(end, page, PROT_NONE) == 0);
			{
				Product plan(a.size(), b.size(), reinterpret_cast<std::uint32_t*>(borrowed));
				copy_input_mint(a, borrowed);
				copy_input(b, plan.b(), len);
				plan.multiply(out);
				for (std::size_t i = 0; i < expected.size(); ++i) require(out[i] == expected[i] && out[i].v < M::mod());
				for (std::size_t i = len + offset; i < words; ++i) require(out[i].v == 0);
			}
			require(out[0] == expected[0]);
			require(munmap(region, readable + 2 * page) == 0);
		}
	}
}
