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
		for (auto append : {division_detail::append_words})
			for (unsigned factor : {0u, 1u, 17u, 301989884u, 932051910u}) {
				std::vector<fps_mint> out(2, fps_mint(17));
				out.reserve(n + 2);
				append(out, values, n, factor);
				assert(out.size() == n + 2 && out[0] == fps_mint(17) && out[1] == fps_mint(17));
				for (std::size_t i = 0; i < n; ++i)
					assert(out[i + 2].v % fps_mint::mod() == std::uint64_t(values[i]) * factor % fps_mint::mod());
			}
		if (n) assert(munmap(mapping, readable + page) == 0);
	}
}

static void check_input() {
	using fps_detail::blocked_poly::native_divide;
	const std::size_t page = std::size_t(sysconf(_SC_PAGESIZE));
	for (int n : {129, 195, 255, 256, 257, 511, 512, 513, 1024, 1025, 4097}) {
		const std::size_t bytes = std::size_t(n) * sizeof(fps_mint), readable = (bytes + page - 1) / page * page;
		void* mapping = mmap(nullptr, readable + 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(mapping != MAP_FAILED);
		auto* guard = static_cast<char*>(mapping) + page + readable;
		assert(mprotect(mapping, page, PROT_NONE) == 0 && mprotect(guard, page, PROT_NONE) == 0);
		auto* values = ::new (guard - bytes) fps_mint[n];
		for (int i = 0; i < n; ++i) values[i] = fps_mint::word(2 * fps_mint::mod() - 1 - i % 7);
		const fps a(values, values + n);
		assert(mprotect(static_cast<char*>(mapping) + page, readable, PROT_READ) == 0);
		for (int m : {3, 31, 64, 65, 131, n / 4, n / 2, n}) {
			if (m > n) continue;
			fps b(m);
			for (int i = 0; i < m; ++i) b[i] = fps_mint::word(2 * fps_mint::mod() - 1 - i % 11);
			b.back() = 3;
			fps q, r;
			native_divide(values, n, b.data(), b.size(), q, r);
			auto restored = q * b + r;
			fps_detail::Operations<fps_mint>::trim(restored);
			assert(restored == a);
			for (int i = 0; i < n; ++i) assert(values[i].v == a[i].v);
		}
		assert(munmap(mapping, readable + 2 * page) == 0);
	}
}

static void check_alias() {
	using fps_detail::blocked_poly::native_divide;
	for (int n : {257, 1025}) {
		fps a(n), b(n / 2 + 7);
		for (int i = 0; i < n; ++i) a[i] = fps_mint::word(2 * fps_mint::mod() - 1 - i % 7);
		for (int i = 0; i < int(b.size()); ++i) b[i] = fps_mint::word(2 * fps_mint::mod() - 1 - i % 11);
		fps q, r;
		native_divide(a.data(), a.size(), b.data(), b.size(), q, r);
		const auto appended = [](fps prefix, const fps& suffix) {
			prefix.insert(prefix.end(), suffix.begin(), suffix.end());
			return prefix;
		};
		const auto preserved = [](const fps& out, const fps& prefix) {
			return out.size() >= prefix.size() &&
				std::equal(prefix.begin(), prefix.end(), out.begin(), [](fps_mint x, fps_mint y) { return x.v == y.v; });
		};
		auto x = a, y = b;
		native_divide(x.data(), x.size(), y.data(), y.size(), x, y);
		assert(x == appended(a, q) && y == appended(b, r) && preserved(x, a) && preserved(y, b));
		x = a, y = b;
		native_divide(x.data(), x.size(), y.data(), y.size(), y, x);
		assert(x == appended(a, r) && y == appended(b, q) && preserved(x, a) && preserved(y, b));
		x = a, y = b;
		native_divide(x.data(), x.size(), y.data(), y.size(), x, x);
		assert(x == appended(appended(a, q), r) && y == b && preserved(x, a) && preserved(y, b));
	}
}
#endif

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (fps_detail::NTT::vectorized()) check_output(), check_input(), check_alias();
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
