#include "fps/polynomial_gcd.hpp"
#include "test/fps_io.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
int main(int argc, char** argv) {
	FastInput in; FastOutput out; int n=0, m=0; in.read(n); in.read(m);
	auto a=read_fps(in,n), b=read_fps(in,m);
	using namespace fps_detail::blocked_poly;
	using namespace fps_detail::blocked_poly::gcd_detail;
	const int direct=argc>1?std::atoi(argv[1]):64;
	const auto begin=std::chrono::steady_clock::now();
	Arena arena(inverse_mod_words(n,m)+Arena::footprint(n)+Arena::footprint(m)+Arena::footprint(m));
	auto f=arena.take(n), g=arena.take(m), h=arena.take(m);
	for(int i=0;i<n;++i) f[i]=std::min(a[i].v,a[i].v-fps_mint::mod());
	for(int i=0;i<m;++i) g[i]=std::min(b[i].v,b[i].v-fps_mint::mod());
	const std::ptrdiff_t degree=m-1;
	auto re=arena.take(degree);
	const auto rs=remainder_size(f.size(),g.size());
	divide(arena,f,g,arena.take(quotient_size(f.size(),g.size())),re.first(rs));
	std::ptrdiff_t d=std::ptrdiff_t(rs)-1;
	while(d>=0&&re[d]==0)--d;
	std::ptrdiff_t size=-1;
	if(d>=0) {
		HalfGcd gcd(arena,degree,direct); auto matrix=gcd.matrix(degree,0);
		gcd.jump(g.data(),degree,re.data(),d,degree,matrix,0x2);
		if(std::ptrdiff_t(matrix.progress)==degree) {
			const auto c=std::uint32_t((std::uint64_t(matrix.at0[0])*g[0]+std::uint64_t(matrix.at0[1])*re[0])%kP);
			const auto scale=mod_inverse(c);
			std::size_t len=matrix.size[1]; while(len&&matrix.entry[1][len-1]==0)--len;
			for(std::size_t i=0;i<len;++i)h[i]=mod_mul(matrix.entry[1][i],scale);
			size=std::ptrdiff_t(len);
		}
	}
	fps answer;
	if(size>=0) { fps_detail::prepare_fps_output(answer,size); division_detail::append_words(answer,h.data(),size,932051910); }
	const auto end=std::chrono::steady_clock::now();
	if(size<0)out.println(-1);else{out.println(answer.size());write_fps(out,answer);}
	std::fprintf(stderr,"%.6f\n",std::chrono::duration<double,std::milli>(end-begin).count());
}
