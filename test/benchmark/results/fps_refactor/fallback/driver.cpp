#include "fps/fps.hpp"
#include <cstdio>
#include <random>
#include <chrono>
#include <cstring>
using M = fps_mint;
static unsigned long long checksum = 0;
int main(int argc, char **argv) {
 std::mt19937 rng(171739);
 if (argc > 1) {
  for (int n : {97,128,129,160,192,224,256,257,320,384,511,512,1024}) for (int input : {33,63,n}) {
   fps f(input);for(auto &v:f)v=M(rng());f[0]=1;
   int iterations=std::max(1000,1000000/n);
   auto start=std::chrono::steady_clock::now();
   for(int rep=0;rep<iterations;++rep){auto g=f.inv(n);checksum+=g[n-1].val();}
   double ns=std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-start).count()/iterations;
   std::printf("%d %d %.6f\n",n,input,ns);
  }
  std::fprintf(stderr,"%llu\n",checksum);return 0;
 }
 for(int n : {0,1,2,17,31,32,33,63,64,65,96,97,127,128,129,160,192,224,255,256,257,320,384,511,512,513,1024,1025})
 for(int input : {1,2,12,32,33,63,64,65,127,256,513}) {
  fps f(input);for(auto &v:f)v=M(rng());f[0]=1;
  auto g=f.inv(n);fps expected(n);if(n)expected[0]=1;
  for(int i=1;i<n;++i)for(int j=1;j<=i&&j<input;++j)expected[i]-=f[j]*expected[i-j];
  if(g!=expected){std::printf("inverse fail %d %d\n",n,input);return 1;}
  auto log=f.log(n);fps lexpected(n);
  for(int i=1;i<n;++i){for(int j=1;j<=i&&j<input;++j)lexpected[i]+=f[j]*M(j)*expected[i-j];lexpected[i]/=M(i);}
  if(log!=lexpected){std::printf("log fail %d %d\n",n,input);return 1;}
  for(auto v:g)checksum=checksum*0x9e3779b97f4a7c15ULL+v.val();
  for(auto v:log)checksum=checksum*0x9e3779b97f4a7c15ULL+v.val();
 }
 std::printf("correct %llu\n",checksum);
}
