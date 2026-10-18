#include "fps/fps.hpp"
#include "components/fps_exp_blocked.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <random>
static void require(bool ok,int line=__builtin_LINE()){if(!ok){std::fprintf(stderr,"line %d\n",line);std::abort();}}
int main(){
 using namespace fps_detail;using namespace blocked_poly;
 std::mt19937 rng(7142026);
 const std::size_t page=sysconf(_SC_PAGESIZE);
 for(std::size_t n:{1,2,31,32,33,63,64,65,95,96,97,127,128,129,191,192,193,255,256,257,383,384,385,511,512,513,1023,1024,1025,4097,16385,262144,262145,393217,500000}){
  for(std::size_t input:{std::size_t(1),std::size_t(7),std::size_t(33),n/3+1,n,n+17}){
   const std::size_t pages=(input*sizeof(Mint)+page-1)/page,bytes=(pages+2)*page;
   auto* base=static_cast<unsigned char*>(mmap(nullptr,bytes,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));require(base!=MAP_FAILED);
   require(!mprotect(base+page,pages*page,PROT_READ|PROT_WRITE));
   auto* f=reinterpret_cast<Mint*>(base+page+pages*page-input*sizeof(Mint));
   for(std::size_t i=0;i<input;++i)f[i]=Mint::word(2*Mint::mod()-1-rng()%1000);
   const std::uint32_t e=1000000000000000000ULL%Mint::mod();const Mint c=f[0].pow(1000000000000000000LL);
   std::vector<unsigned> words(std::min(input,n));for(std::size_t i=0;i<words.size();++i)words[i]=detail::mint_scalar(f[i]);
   require(!mprotect(base+page,pages*page,PROT_READ));
   std::vector<Mint> out;native_power(f,input,out,n,e,c);require(out.size()==n);
   const int lg=power_log(n);Arena arena(Transform::words(lg)+Arena::footprint(n)+Arena::footprint(power_scratch(n)));const Transform t(arena,lg);auto g=arena.take(n),scratch=arena.take(power_scratch(n));
   power(t,words,e,detail::mint_scalar(c),g,scratch);
   for(std::size_t i=0;i<n;++i)require(out[i]==Mint::word(g[i]));
   for(std::size_t i=0;i<words.size();++i)require(words[i]==detail::mint_scalar(f[i]));
   require(!munmap(base,bytes));
  }
 }
 std::puts("guard-page const/unaligned/no-padding short and full input: pass");
}
