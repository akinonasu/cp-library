#include "leaf_ifma.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <random>
#include <cstdlib>
#include <memory>
using namespace leaf_ifma_probe;
static void require(bool ok,int line=__builtin_LINE()){if(!ok){std::fprintf(stderr,"line %d\n",line);std::abort();}}
int main(){static_assert(alignof(Window)==32);require(__builtin_cpu_supports("avx512f")&&__builtin_cpu_supports("avx512ifma"));std::mt19937 rng(19512026);std::uint64_t max_sum=0,max_mont=0,max_lo=0,max_hi=0;unsigned max_word=0,max_out=0;const std::size_t page=sysconf(_SC_PAGESIZE);auto* mem=static_cast<unsigned char*>(mmap(nullptr,2*page,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));require(mem!=MAP_FAILED);auto* b=reinterpret_cast<unsigned*>(mem+page-32);require(!mprotect(mem+page,page,PROT_NONE));
 void* allocation=std::aligned_alloc(64,128);require(allocation);auto* wp=std::construct_at(reinterpret_cast<Window*>(static_cast<unsigned char*>(allocation)+32));require(reinterpret_cast<std::uintptr_t>(wp)%64==32);
 for(int trial=0;trial<10000;++trial){Window& w=*wp;for(int i=0;i<16;++i){w.word[i]=trial<32?(i<8&&trial%2?kP:kP-1):unsigned(rng()%kP);}w.word[16]=0;for(int i=0;i<8;++i)b[i]=trial<32?kP-1:unsigned(rng()%kP);alignas(32)unsigned old[8],got[8],via[8],g1[8],g2[8];store(old,leaf_product(w,b));store(got,call(w,b));store(via,dispatch(w,b));store(g1,call_g1(w,b));store(g2,call_g2(w,b));for(int i=0;i<8;++i){unsigned __int128 sum=0;for(int j=0;j<8;++j)sum+=static_cast<unsigned __int128>(w.word[8+i-j])*b[j];std::uint64_t low=0,high=0;for(int j=0;j<8;++j){auto z=std::uint64_t(w.word[8+i-j])*b[j];low+=z&((1ULL<<52)-1);high+=z>>52;}require(static_cast<unsigned __int128>(low)+(static_cast<unsigned __int128>(high)<<52)==sum);max_lo=std::max(max_lo,low);max_hi=std::max(max_hi,high);auto q=unsigned(sum)*fps_detail::blocked_ntt::kernels::kNI;auto mont=(sum+static_cast<unsigned __int128>(q)*kP)>>32;unsigned raw=unsigned(mont);if(raw>=2*kP)raw-=2*kP;require(old[i]==got[i]&&got[i]==via[i]&&got[i]==g1[i]&&got[i]==g2[i]&&got[i]==raw&&got[i]<2*kP);max_sum=std::max(max_sum,std::uint64_t(sum));max_mont=std::max(max_mont,std::uint64_t(mont));max_out=std::max(max_out,got[i]);max_word=std::max(max_word,w.word[i]);}}
 std::destroy_at(wp);std::free(allocation);require(!mprotect(mem,page,PROT_READ));Window w{};for(int i=0;i<16;++i)w.word[i]=kP-1;alignas(32)unsigned x[8];store(x,call(w,b));require(!munmap(mem,2*page));std::printf("pass trials=10000 lanes=80000 alignof_window=32 actual_window_mod64=32 max_input=%u max_sum=%llu max_mont=%llu max_output=%u max_lo=%llu max_hi=%llu lo_carry52=%llu guard_b=8words\n",max_word,(unsigned long long)max_sum,(unsigned long long)max_mont,max_out,(unsigned long long)max_lo,(unsigned long long)max_hi,(unsigned long long)(max_lo>>52));
}
