#include "components/fps_inverse_blocked.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <random>
static void require(bool ok,int line=__builtin_LINE()){if(!ok){std::fprintf(stderr,"line %d\n",line);std::abort();}}
int main(){using namespace fps_detail;std::mt19937 rng(1442026);const auto page=sysconf(_SC_PAGESIZE);for(std::size_t n:{0,1,7,8,9,31,32,33,63,64,65,127,128,129,500000}){std::size_t bytes=n*sizeof(unsigned),readable=(bytes+page-1)/page*page;auto*base=static_cast<unsigned char*>(mmap(nullptr,readable+page,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));require(base!=MAP_FAILED);auto*in=reinterpret_cast<unsigned*>(base+readable-bytes);for(std::size_t i=0;i<n;++i)in[i]=rng()%(2*Mint::mod());require(!mprotect(base,readable+page,PROT_READ));require(!mprotect(base+readable,page,PROT_NONE));for(unsigned scale:{0u,1u,65536u,932051910u,Mint::mod()-1})for(int prefix:{0,1,7,64}){std::vector<Mint>out(prefix,Mint(7));blocked_poly::append_scaled({in,n},out,scale);require(out.size()==n+prefix);for(int i=0;i<prefix;++i)require(out[i]==Mint(7));for(std::size_t i=0;i<n;++i){auto expected=blocked_ntt::detail::multiply_mod(in[i],scale);require(out[i+prefix].v==expected);}}require(!munmap(base,readable+page));}std::puts("append counts/prefix/scales/read-only/no-padding: pass");}
