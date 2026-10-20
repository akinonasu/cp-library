#include "components/fps_blocked.hpp"
#include <cstdio>
#include <random>
using namespace fps_detail::blocked_poly;
using namespace fps_detail::blocked_poly::detail;
int main() {
	std::mt19937 rng(20261029);
	for (std::size_t h : {3u, 4u, 16u, 32u, 64u, 127u, 256u, 1024u, 16384u}) {
		const std::size_t n = 16 * h;
		Arena arena(2 * Arena::footprint(n + 16));
		auto a = arena.take(n + 16), b = arena.take(n + 16);
		for (auto mode : {Half::kLower, Half::kUpper, Half::kBoth}) for (unsigned scale : {0u, 1u, kP-1, 932051910u}) {
			if (mode == Half::kBoth && h % 2) continue;
			for (std::size_t i=0;i<n;++i) a[i]=b[i]=rng()%(2*kP);
			for (std::size_t i=n;i<n+16;++i) a[i]=b[i]=0xdeadbeef;
			inverse_top2(reinterpret_cast<Vec*>(a.data()),h,mode,scale);
			for (std::size_t i=0;i<8*h;++i) {
				const auto lo = mode == Half::kUpper ? b[i] : (std::uint64_t(b[i])+b[i+8*h])*scale%kP;
				const auto hi = mode == Half::kLower ? b[i+8*h] : (std::uint64_t(b[i])+2*kP-b[i+8*h])*scale%kP;
				if(a[i]!=lo||a[i+8*h]!=hi) std::abort();
			}
			for (std::size_t i=n;i<n+16;++i) if(a[i]!=b[i]) std::abort();
		}
	}
	std::puts("all radix2 inverse halves, odd-size fallback halves, scales and canaries passed");
}
