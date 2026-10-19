#include "fps/polynomial_gcd.hpp"
#include "test/fps_io.hpp"
#include <chrono>
#include <cstdio>
int main(int argc,char**argv){
 FastInput in;FastOutput out;int n=0,m=0;in.read(n);in.read(m);auto a=read_fps(in,n),b=read_fps(in,m);
 const auto begin=std::chrono::steady_clock::now();
 #if MODE == 0
 auto [q,r]=a.divmod(b);
 const auto end=std::chrono::steady_clock::now();
 out.write(q.size());out.write(' ');out.println(r.size());write_fps(out,q);write_fps(out,r);
 #elif MODE == 1
 auto r=polynomial_inv_mod(std::move(a),std::move(b));const auto end=std::chrono::steady_clock::now();
 if(!r)out.println(-1);else {out.println(r->size());write_fps(out,*r);}
 #elif MODE == 2
 auto r=polynomial_gcd(std::move(a),std::move(b));const auto end=std::chrono::steady_clock::now();out.println(r.size());write_fps(out,r);
 #else
 auto r=a.div_series(b,n);
 const auto end=std::chrono::steady_clock::now();out.println(r.size());write_fps(out,r);
 #endif
 std::fprintf(stderr,"%.6f\n",std::chrono::duration<double,std::milli>(end-begin).count());
}
