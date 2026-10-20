#include "components/fps_bostan.hpp"
#include "components/fps_workspace.hpp"
#include <random>
int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
 using N=fps_detail::NTT;
 if (!N::vectorized()) return 0;
 std::mt19937 rng(92837);
 for(int n=8;n<=(1<<18);n*=2) {
  N plan(n,false);
  auto input=fps_detail::allocate_fps_words(n),actual=fps_detail::allocate_fps_words(n),expected=fps_detail::allocate_fps_words(n),powers=fps_detail::allocate_fps_words(n);
  for(int i=0;i<n;++i) input[i]=expected[i]=rng()%(2*N::mod),powers[i]=rng()%(2*N::mod);
  plan.fft<true>(expected.get(),n);N::product(expected.get(),expected.get(),powers.get(),n);
  fps_detail::bostan_detail::inverse_twisted(plan,input.get(),actual.get(),powers.get(),n);
  for(int i=0;i<n;++i) assert(actual[i]%N::mod==expected[i]%N::mod);
 }
#endif
}
