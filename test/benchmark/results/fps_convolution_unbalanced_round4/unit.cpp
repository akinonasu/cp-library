#include "overlap.hpp"
#include <cassert>
#include <random>
#include <iostream>
int main() {
	std::mt19937 rng(381247);
	for (unsigned m : {1, 2, 31, 32, 33, 63, 64, 99, 100, 127, 128, 255, 1000}) {
		for (unsigned n : {1, 63, 64, 65, 257, 999, 8193}) {
			std::vector<fps_detail::Mint> a(n), b(m), expected(n + m - 1);
			for (auto& v : a) { v = fps_detail::Mint::raw(rng() % v.mod()); v.v %= v.mod(); if(rng() & 1) v.v += v.mod(); }
			for (auto& v : b) { v = fps_detail::Mint::raw(rng() % v.mod()); v.v %= v.mod(); if(rng() & 1) v.v += v.mod(); }
			for (unsigned i = 0; i < n; ++i) for(unsigned j=0;j<m;++j) expected[i+j] += a[i]*b[j];
			for (unsigned factor : {1, 2, 4, 8}) {
				const auto len = std::max(64u, std::bit_ceil(std::min(n,m) * 2)) * factor;
				auto result = round4_overlap::multiply(a,b,len);
				if(result != expected) { std::cerr << "m="<<m<<" n="<<n<<" len="<<len<<"\n"; for(unsigned k=0;k<result.size();++k) if(result[k]!=expected[k]) { std::cerr<<k<<" got="<<result[k].val()<<" word="<<result[k].v<<" expected="<<expected[k].val()<<" word="<<expected[k].v<<"\n"; break; } return 1; }
			}
		}
	}
}
