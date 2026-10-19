#include "components/fps_division_blocked.hpp"
#include "test/fps_io.hpp"
#include <chrono>
#include <cstdio>
#include <random>
int main() {
 FastInput in;int n=0,m=0;in.read(n);in.read(m);auto af=read_fps(in,n),ag=read_fps(in,m);
 using namespace fps_detail::blocked_poly;using namespace division_detail;
 const std::size_t k=quotient_size(n,m),d=remainder_size(n,m);
 Arena inputs(Arena::footprint(n)+Arena::footprint(m)+2*Arena::footprint(k)+2*Arena::footprint(d));
 auto f=inputs.take(n),g=inputs.take(m),q=inputs.take(k),r=inputs.take(d),expectedq=inputs.take(k),expectedr=inputs.take(d);
 for(int i=0;i<n;++i)f[i]=input_word(af[i]);for(int i=0;i<m;++i)g[i]=input_word(ag[i]);
 DivisionPlan default_plan(n,m);Arena work(default_plan.words());division_detail::divide(work,default_plan,std::span<const unsigned>(f),std::span<const unsigned>(g),expectedq,expectedr);
 std::vector<DivisionPlan> plans;
 for(std::size_t s=32;;s*=2) {
  for(auto how:{Remainder::kDirect,Remainder::kWrap,Remainder::kSplit,Remainder::kTail}) {
   DivisionPlan plan(n,m,s,how);
   if(std::isfinite(plan.quotient.ns+plan.remainder.ns))plans.push_back(plan);
  }
  if(s>=k)break;
 }
 std::printf("{\"n\":%d,\"m\":%d,\"default\":{\"s\":%zu,\"remainder\":%d},\"plans\":[",n,m,default_plan.quotient.s,int(default_plan.remainder.method));
 std::vector<std::vector<double>> times(plans.size());std::vector<int> order(plans.size());std::iota(order.begin(),order.end(),0);std::mt19937 rng(20261103);
 for(int rep=0;rep<10;++rep) {
  std::shuffle(order.begin(),order.end(),rng);
  for(int i:order) {
   const auto start=std::chrono::steady_clock::now();
   {Arena a(plans[i].words());division_detail::divide(a,plans[i],std::span<const unsigned>(f),std::span<const unsigned>(g),q,r);}
   const auto end=std::chrono::steady_clock::now();
   assert(std::equal(q.begin(),q.end(),expectedq.begin())&&std::equal(r.begin(),r.end(),expectedr.begin()));
   if(rep)times[i].push_back(std::chrono::duration<double,std::milli>(end-start).count());
  }
 }
 for(std::size_t i=0;i<plans.size();++i) {
  const auto& p=plans[i];std::printf("%s{\"s\":%zu,\"blocks\":%zu,\"remainder\":%d,\"predicted_ns\":%.4f,\"samples_ms\":[",i?",":"",p.quotient.s,p.quotient.blocks,int(p.remainder.method),p.quotient.ns+p.remainder.ns);
  for(std::size_t j=0;j<times[i].size();++j)std::printf("%s%.6f",j?",":"",times[i][j]);std::printf("]}");
 }
 std::puts("]}");
}
