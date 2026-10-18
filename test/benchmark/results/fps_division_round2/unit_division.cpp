#include "fps/fps.hpp"
#include <random>
#include <new>
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <sys/mman.h>
#include <unistd.h>

static void check_output() {
	using namespace fps_detail::blocked_poly;
	const std::size_t page = std::size_t(sysconf(_SC_PAGESIZE));
	for (std::size_t n : {0, 1, 7, 8, 9, 31, 32, 33, 63, 64, 65, 127, 128, 129, 511, 512, 513, 65537}) {
		const std::size_t bytes = n * sizeof(unsigned), readable = (bytes + page - 1) / page * page;
		void* mapping = nullptr;
		unsigned* values = nullptr;
		if (n) {
			mapping = mmap(nullptr, readable + page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			assert(mapping != MAP_FAILED);
			auto* guard = static_cast<char*>(mapping) + readable;
			assert(mprotect(guard, page, PROT_NONE) == 0);
			values = ::new (guard - bytes) unsigned[n];
			for (std::size_t i = 0; i < n; ++i) values[i] = i % 3 ? unsigned(i * 7123671 % fps_mint::mod()) : fps_mint::mod() - 1;
			assert(mprotect(mapping, readable, PROT_READ) == 0);
		}
		for (unsigned factor : {0u, 1u, 17u, 301989884u, 932051910u}) {
			std::vector<fps_mint> out(2, fps_mint(17));
			out.reserve(n + 2);
			division_detail::append_words(out, values, n, factor);
			assert(out.size() == n + 2 && out[0] == fps_mint(17) && out[1] == fps_mint(17));
			for (std::size_t i = 0; i < n; ++i)
				assert(out[i + 2].v % fps_mint::mod() == std::uint64_t(values[i]) * factor % fps_mint::mod());
		}
		if (n) assert(munmap(mapping, readable + page) == 0);
	}
}
#endif

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (fps_detail::NTT::vectorized()) check_output();
#endif
	std::mt19937 random(928374);
	for (int n : {129, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025, 4097}) {
		for (int m : {2, 31, 64, 65, 127, 128, 129, n / 4, n / 2, n}) {
			if (m > n) continue;
			fps a(n), b(m);
			for (auto& x : a) x = random();
			for (auto& x : b) x = random();
			b[0] = b.back() = 3;
			for (auto* f : {&a, &b}) for (auto& x : *f) if (x.v < mint::mod()) x.v += mint::mod();
			auto original_a = a, original_b = b;
			auto [q, r] = a.divmod(b);
			auto product = q * b + r;
			fps_detail::Operations<mint>::trim(product);
			assert(product == a && r.size() < b.size());
			for (int degree : {129, 257, 513, n}) {
				auto quotient = a.div_series(b, degree);
				auto restored = (quotient * b).pre(degree), expected = a.pre(degree);
				restored.resize(degree), expected.resize(degree);
				assert(restored == expected && quotient.size() == std::size_t(degree));
			}
			a.push_back(0), b.push_back(0);
			auto trimmed = a.divmod(b);
			assert(trimmed.first == q && trimmed.second == r);
			a.pop_back(), b.pop_back();
			assert(a == original_a && b == original_b);
		}
	}
}
