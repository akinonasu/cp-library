#include "components/fps_division_blocked.hpp"
#include <chrono>
#include <cstdio>
#include <random>
#pragma GCC target("avx2")
#pragma GCC optimize("O3")
using fps_detail::Mint;
using namespace fps_detail::blocked_poly;
using namespace fps_detail::blocked_poly::division_detail;
template<std::size_t B>
void append_batch(std::vector<fps_detail::Mint>& out,const std::uint32_t* values,std::size_t n,std::uint32_t factor) {
 const Factor scale(factor);std::array<Mint,B> batch;std::size_t i=0;
 const auto chunk=[&](std::size_t count) {
  if(factor==1) std::memcpy(static_cast<void*>(batch.data()),values+i,count*sizeof(Mint));
  else {
#pragma GCC unroll 8
 for(std::size_t j=0;j<count;j+=8) {const Vec word=times(load_any(values+i+j),scale);std::memcpy(static_cast<void*>(batch.data()+j),&word,sizeof(word));}
 }
  out.insert(out.end(),batch.begin(),batch.begin()+count);
 };
 for(;i+B<=n;i+=B)chunk(B);
 for(;i+64<=n;i+=64)chunk(64);
 if(i<n)append_words(out,values+i,n-i,factor);
}
int main() {
 using Append=void(*)(std::vector<Mint>&,const std::uint32_t*,std::size_t,std::uint32_t);
 const Append variants[]={append_division_words,append_batch<128>,append_batch<256>,append_batch<512>,append_batch<1024>,append_batch<2048>};
 const int batches[]={64,128,256,512,1024,2048};std::mt19937 rng(20261104);std::vector<unsigned> values(500000);for(auto& v:values)v=rng()%Mint::mod();
 const std::pair<int,int> cases[]={{25488,248522},{50140,412906},{362953,92221},{403411,25838},{110188,389812}};
 std::printf("[");bool firstcase=true;
 for(auto [k,d]:cases) {
  if(!firstcase)std::printf(",");firstcase=false;std::vector<double> times[6];std::array<int,6> order={0,1,2,3,4,5};
  for(int rep=0;rep<44;++rep) {
   std::shuffle(order.begin(),order.end(),rng);
   for(int v:order) {
    auto start=std::chrono::steady_clock::now();
    std::vector<Mint> q,r;fps_detail::prepare_fps_output(q,k);fps_detail::prepare_fps_output(r,d);
    variants[v](q,values.data(),k,fps_detail::blocked_ntt::detail::kR);variants[v](r,values.data()+k,d,1);
    auto end=std::chrono::steady_clock::now();
    assert(q.size()==std::size_t(k)&&r.size()==std::size_t(d));
    for(int i=0;i<k;++i)assert(q[i].v%Mint::mod()==std::uint64_t(values[i])*fps_detail::blocked_ntt::detail::kR%Mint::mod());
    for(int i=0;i<d;++i)assert(r[i].v==values[k+i]);
    if(rep>=3)times[v].push_back(std::chrono::duration<double,std::milli>(end-start).count());
   }
  }
  std::printf("{\"k\":%d,\"d\":%d,\"variants\":[",k,d);
  for(int v=0;v<6;++v) {std::printf("%s{\"batch\":%d,\"samples_ms\":[",v?",":"",batches[v]);for(std::size_t j=0;j<times[v].size();++j)std::printf("%s%.6f",j?",":"",times[v][j]);std::printf("]}");}std::printf("]}");
 }
 std::puts("]");
}
