
#include <cstddef>
#include <sys/mman.h>
#ifdef FPS_FORCE_MMAP_FAILURE
static void* fps_failed_mmap(void*, std::size_t, int, int, int, off_t) { return MAP_FAILED; }
#define mmap fps_failed_mmap
#endif
#include "components/fps_blocked.hpp"
#include <cassert>
#include <cstdio>
#undef mmap
int main() {
 using namespace fps_detail;
 using namespace fps_detail::blocked_poly;
 Arena arena(2 * 1024 * 1024);
#ifdef FPS_FORCE_MMAP_FAILURE
 assert(!arena.zero_initialized());
#else
 assert(arena.zero_initialized());
#endif
 for(int lg : {6, 8, 18, 20}) {
  auto prior=arena.take(64);
  std::fill(prior.begin(),prior.end(),0xdeadbeefu);
  Transform transform(arena,lg);
  const std::size_t table=blocked_ntt::detail::table_words(lg),entries=(std::size_t(1)<<lg)/16;
  auto expected=allocate_fps_words(2*table);
  std::fill_n(expected.get(),2*table,0u);
  blocked_ntt::detail::build_table(expected.get(),entries,blocked_ntt::detail::kRoots[0]);
  blocked_ntt::detail::build_table(expected.get()+table,entries,blocked_ntt::detail::kRoots[1]);
  assert(std::equal(expected.get(),expected.get()+table,transform.roots()));
  assert(std::equal(expected.get()+table,expected.get()+2*table,transform.inverse_roots()));
  assert(std::all_of(prior.begin(),prior.end(),[](unsigned x){return x==0xdeadbeefu;}));
 }
 Arena small(4096);
 assert(!small.zero_initialized());
 Transform t(small,8);
 std::puts("safety passed");
}
