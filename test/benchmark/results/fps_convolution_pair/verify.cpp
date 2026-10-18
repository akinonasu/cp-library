#include "components/fps_convolution.hpp"
#include <cstdio>
#include <random>

int main() {
	using namespace fps_detail::multiply_detail;
	using namespace fps_detail::multiply_detail::ntt;
	std::mt19937 rng(20261021);
	const std::size_t table = detail::table_words(20);
	auto roots = fps_detail::allocate_fps_words(table);
	std::fill_n(roots.get(), table, 0u);
	detail::build_table(roots.get(), (1u << 20) / 16, detail::kRoots[0]);
	for (std::size_t h : {4u,16u,64u,256u,1024u,4096u,16384u}) for (std::size_t k : {1u,2u,3u,17u}) {
		const std::size_t size = 32 * h;
		auto a = fps_detail::allocate_fps_words(size + 16), b = fps_detail::allocate_fps_words(size + 16);
		auto c = fps_detail::allocate_fps_words(size + 16), d = fps_detail::allocate_fps_words(size + 16);
		for (std::size_t j = 0; j < size; ++j) a[j] = c[j] = rng() % (4u * detail::kP), b[j] = d[j] = rng() % (4u * detail::kP);
		for (auto* p : {a.get(),b.get(),c.get(),d.get()}) std::fill_n(p + size, 16, 0xdeadbeefu);
		const auto *x = roots.get() + detail::slot(k), *y = roots.get() + detail::slot(2*k);
		kernels::forward_pair(reinterpret_cast<detail::Vec*>(a.get()),reinterpret_cast<detail::Vec*>(b.get()),h,x,y);
		kernels::forward(reinterpret_cast<detail::Vec*>(c.get()),h,x,y);
		kernels::forward(reinterpret_cast<detail::Vec*>(d.get()),h,x,y);
		for (std::size_t j = 0; j < size; ++j) if (a[j] % detail::kP != c[j] % detail::kP || b[j] % detail::kP != d[j] % detail::kP) std::abort();
		for (auto* p : {a.get(),b.get(),c.get(),d.get()}) for (std::size_t j = size; j < size+16; ++j) if (p[j] != 0xdeadbeefu) std::abort();
	}
	std::puts("paired forward range, lazy values, and canary passed");
}
