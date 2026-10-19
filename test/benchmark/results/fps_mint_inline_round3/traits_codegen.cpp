#include <vector>
#include <memory>
#include <new>
#include <type_traits>
#include <cstdio>
#include "math/mint.hpp"
using T=fps_detail::Mint;
constexpr T zero;
static_assert(zero.v==0 && T{}.v==0);
static_assert(sizeof(T)==sizeof(unsigned) && alignof(T)==alignof(unsigned));
static_assert(std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T>);
static_assert(!std::is_aggregate_v<T> && !std::is_trivial_v<T> && !std::is_trivially_default_constructible_v<T>);
static_assert(std::is_default_constructible_v<T> && std::is_nothrow_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T> && std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T> && std::is_trivially_move_assignable_v<T> && std::is_trivially_destructible_v<T>);
static_assert(noexcept(T()) && noexcept(T{}));
T global_zero;
[[gnu::noinline]] T make_default() {T a;return a;}
[[gnu::noinline]] std::vector<T> make_vector(std::size_t n) {return std::vector<T>(n);}
[[gnu::noinline]] void resize_vector(std::vector<T>& v,std::size_t n) {v.resize(n);}
int main() {
 assert(make_default().v==0 && global_zero.v==0);
 auto a=make_vector(513);for(auto x:a)assert(x.v==0);
 a[0]=T::word(2*T::mod()-1);resize_vector(a,1025);assert(a[0].v==2*T::mod()-1);for(std::size_t i=1;i<a.size();++i)assert(a[i].v==0);
 auto b=std::make_unique<T[]>(129);for(int i=0;i<129;++i)assert(b[i].v==0);
 void* raw=::operator new(sizeof(T)*257);auto* p=static_cast<T*>(raw);std::uninitialized_default_construct_n(p,257);for(int i=0;i<257;++i)assert(p[i].v==0);std::destroy_n(p,257);::operator delete(raw);
 std::puts("{\"sizeof\":4,\"alignof\":4,\"standard_layout\":true,\"trivially_copyable\":true,\"aggregate\":false,\"trivial\":false,\"trivially_default_constructible\":false,\"default_constructible\":true,\"nothrow_default_constructible\":true,\"constexpr_zero\":true,\"default_initialization\":\"zero\",\"resize_prefix\":\"preserved\"}");
}
