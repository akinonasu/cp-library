#include "snapshot/fps/fps.hpp"
#include "overlap.hpp"
#include <chrono>
#include <iostream>
#include <random>
int main() {
	std::mt19937 rng(289451);
	for (unsigned m : {33, 100, 1000, 4096, 16000}) for (unsigned ratio : {16, 32, 64, 128}) {
		const unsigned n=m*ratio;
		if(n>524288) continue;
		std::vector<fps_detail::Mint> a(n),b(m);
		for(auto& x:a)x=fps_detail::Mint::raw(rng()%x.mod());
		for(auto& x:b)x=fps_detail::Mint::raw(rng()%x.mod());
		for(unsigned trial=0;trial<10;++trial) {
			unsigned order[2]={0,1}; if(rng()&1)std::swap(order[0],order[1]);
			for(unsigned method:order) {
				const auto start=std::chrono::steady_clock::now();
				auto out=method?round4_overlap::multiply(a,b,std::max(64u,4*std::bit_ceil(m))):fps_detail::multiply_detail::multiply(a,b);
				const auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start).count();
				std::cout<<m<<' '<<n<<' '<<trial<<' '<<method<<' '<<elapsed<<' '<<out[n/2].val()<<'\n';
			}
		}
	}
}
