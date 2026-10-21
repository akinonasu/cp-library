#ifndef CP_FPS_LIGHT_BACKEND_HPP
#define CP_FPS_LIGHT_BACKEND_HPP
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include "../math/mint.hpp"
#define À std::size_t
#define Á ;fps_light_detail::transform
#define Â fps_light_detail::
#define Ã std::
#define Ä result
#define Å return
#define Ç LightFormalPowerSeries
#define È const
#define É reinterpret_cast<
#define Ê __attribute__((target("avx2")))
#define Ë inverse
#define Ì error
#define Í this->
#define Ï friend F operator
#define Ð operator
#define Ñ begin()
#define Ò .resize
#define Ó _mm256_blend_epi32
#define Õ __m256i
#define Ö static_cast<
#define Ø _mm256_set1_epi32
#define Ù =_mm256_loadu_si256
#define Ú ;for
#define Û ;if constexpr(!Ã is_same_v<T,mint>)Å from_native
#define Ü È int n=precision(deg);if(!n)Å{};
#define Ý T sum=0 Ú(int j=1;j<=i&&À(j)<Í size();++j)sum+=
#define Þ À i=0;i<
#define ß ;Â pointwise(Ì,Ë)Á(Ì,true)
#define à [j]*Ä[i-j];Ä[i]=
#define á initial
#define â mint
#define ã _mm256_storeu_si256
#define ä assert
#define å if constexpr
#define æ value
#define ç ;Â pointwise
#define è vectorized
#define é product
#define ê _mm256_srli_epi64
#define ë À i=
#define ì Ã vector<T>
#define î template<class T
#define ï length
#define ð auto
#define ñ shrink()
#define ò multiply
#define ó int
#define ô Í size()
#define õ [i]
#define ö ::mod()
#define ø Å Ä;
#define ù ;i<n;++i
#define ú .data()
#define û _mm256_mul_epu32
#define ü ;++i
#define ý _mm256_permute2x128_si256
#define þ .size()
#define ÿ y=_mm256_shuffle_epi32(x,0xb1);x=Ó(sum_vec(x,y),diff_vec(y,x),0xaa);
#define Ā È F&
#define ā T(0)
#define Ă T æ=À(i)<ô?(*this)õ:ā Ú(ó j=1;j<=i&&À(j)<g þ;++j)æ-=g à æ*
#define ă {T æ=À(i)<f þ?f õ:ā Ú(ó j=1;j<i;++j)æ-=Ä à æ*factor;}
#define Ą Ö unsigned long long>
#define ą empty()
#define Ć forward
#define ć first
#define Ĉ template<bool Inverse>
#define ĉ work=newton_precision
#define Ċ reciprocal
#define ċ _mm256_sub_epi32
#define Č fixed_product
#define č denominator
#define Ď Ä.insert(Ä.Ñ,shift,ā);ø
#define ď frequency
#define Đ ó i=
#define đ Ì.Ñ
#define Ē using F=Ç<T>;
#define ē difference
#define Ĕ _mm256_mullo_epi32
#define ĕ roots
#define Ė *this
#define ė ð x Ù(É È Õ*>(a+i));
#define Ę Ã move
#define ę inline void
#define Ě Ã vector<â>
#define ě work
#define Ĝ _mm256_min_epu32
#define ĝ _mm256_add_epi32
#define Ğ [[maybe_unused]]bool è
#define ğ simd_available()
#define Ġ ;Ã fill
#define ġ Ã min
#define Ģ div_series
#define ģ É È __m128i*>
#define Ĥ if(ô<g þ)Í resize(g þ)Ú(Þ g þ ü)
#define ĥ Å Ė
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
namespace fps_light_detail{static_assert(sizeof(â)==sizeof(unsigned)&&Ã is_trivially_copyable_v<â>);inline bool ğ{
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
Å __builtin_cpu_supports("avx2");
#else
Å false;
#endif
}Ĉ void scalar_stage(â*a,À n,À h,È â*ĕ){for(À start=0;start<n;start+=h*2)for(À j=0;j<h;++j){â x=a[start+j],y=a[start+j+h];å(Inverse)y*=ĕ[j];a[start+j]=x+y;a[start+j+h]=x-y;å(!Inverse)a[start+j+h]*=ĕ[j];}}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
Ê inline Õ é(Õ x,Õ y){È ð p=Ø(â ö);ð even=û(x,y);ð odd=û(ê(x,32),ê(y,32));ð q=Ĕ(Ĕ(x,y),Ø(998244351));even=ê(_mm256_add_epi64(even,û(q,p)),32);odd=_mm256_add_epi64(odd,û(ê(q,32),p));Å Ó(even,odd,0xaa);}Ê inline Õ Č(Õ x,Õ w,Õ q){ð even=û(x,q);ð odd=û(ê(x,32),ê(q,32));q=Ó(ê(even,32),odd,0xaa);Å ċ(Ĕ(x,w),Ĕ(q,Ø(â ö)));}Ê ę pointwise_avx2(â*a,È â*b,À n){ë 0 Ú(;i+8<=n;i+=8){ė ð y Ù(É È Õ*>(b+i));ã(É Õ*>(a+i),é(x,y));}for(ù)a õ*=b õ;}Ê ę scale_avx2(â*a,À n,â factor){ë 0;unsigned æ=factor.val();È ð f=Ø(æ);È ð q=Ø((Ą(æ)<<32)/â ö)Ú(;i+8<=n;i+=8){ė ã(É Õ*>(a+i),Č(x,f,q));}for(ù)a õ*=factor;}Ê inline Õ sum_vec(Õ x,Õ y){ð z=ĝ(x,y);Å Ĝ(z,ċ(z,Ø(2*â ö)));}Ê inline Õ diff_vec(Õ x,Õ y){ð z=ċ(x,y);Å Ĝ(z,ĝ(z,Ø(2*â ö)));}Ĉ Ê void leaf8(â*a,À n,È â*ĕ){alignas(32)unsigned values[8],quotients[8]Ú(Đ 0;i<8 ü){values õ=ĕ õ.val();quotients õ=(Ą(values õ)<<32)/â ö;}È ð w2=_mm256_broadcastq_epi64(_mm_loadl_epi64(ģ(values+2)));È ð w4=_mm256_broadcastsi128_si256(_mm_loadu_si128(ģ(values+4)));È ð q2=_mm256_broadcastq_epi64
(_mm_loadl_epi64(ģ(quotients+2)));È ð q4=_mm256_broadcastsi128_si256(_mm_loadu_si128(ģ(quotients+4)))Ú(Þ n;i+=8){ė å(!Inverse){ð y=ý(x,x,1);x=Ó(sum_vec(x,y),Č(diff_vec(y,x),w4,q4),0xf0);y=_mm256_shuffle_epi32(x,0x4e);x=Ó(sum_vec(x,y),Č(diff_vec(y,x),w2,q2),0xcc);ÿ}else{ð ÿ x=Ó(x,Č(x,w2,q2),0xcc);y=_mm256_shuffle_epi32(x,0x4e);x=Ó(sum_vec(x,y),diff_vec(y,x),0xcc);x=Ó(x,Č(x,w4,q4),0xf0);y=ý(x,x,1);x=Ó(sum_vec(x,y),diff_vec(y,x),0xf0);}ã(É Õ*>(a+i),x);}}Ê ę expand_roots(â*out,È â*previous,À n,â step){È ð factor=Ø(step.v)Ú(Þ n;i+=8){ð x Ù(É È Õ*>(previous+i));ð y=é(x,factor);ð low=_mm256_unpacklo_epi32(x,y),high=_mm256_unpackhi_epi32(x,y);ã(É Õ*>(out+2*i),ý(low,high,0x20));ã(É Õ*>(out+2*i+8),ý(low,high,0x31));}}Ĉ Ê void vector_stage(â*a,À n,À h,È â*ĕ){È ð p=Ø(2*â ö)Ú(À start=0;start<n;start+=h*2)for(À j=0;j<h;j+=8){ð*ć=É Õ*>(a+start+j);ð*last=É Õ*>(a+start+j+h);ð x Ù(ć),y Ù(last);ð w Ù(É È Õ*>(ĕ+j));å(Inverse)y=é(y,w);ð sum=ĝ(x,y),ē=ċ(x,y);sum=Ĝ(sum,ċ(sum,p));ē=Ĝ(ē,ĝ(ē,p));å(!Inverse)ē=é(ē
,w);ã(ć,sum);ã(last,ē);}}
#endif
struct Roots{Ě Ć{0,1},Ë{0,1};void prepare(À n){À begin=Ć þ;if(begin>=n)Å;Ć Ò(n),Ë Ò(n)Ú(À h=begin;h<n;h*=2){â step=â(3).pow((â ö-1)/(h*2));
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
if(ğ&&h>=16)expand_roots(Ć ú+h,Ć ú+h/2,h/2,step);else
#endif
for(À j=0;j<h/2;++j){Ć[h+2*j]=Ć[h/2+j];Ć[h+2*j+1]=Ć[h/2+j]*step;}Ë[h]=1 Ú(À j=1;j<h;++j)Ë[h+j]=-Ć[2*h-j];}}};ę pointwise(Ě&a,È Ě&b,Ğ=ğ){ä(a þ==b þ);
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
if(è)Å pointwise_avx2(a ú,b ú,a þ);
#endif
for(Þ a þ ü)a õ*=b õ;}ę scale(Ě&a,â factor,Ğ=ğ){
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
if(è)Å scale_avx2(a ú,a þ,factor);
#endif
for(ð&x:a)x*=factor;}Ĉ void stage(â*a,À n,À h,È â*ĕ,Ğ){
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
if(è&&h>=8)Å vector_stage<Inverse>(a,n,h,ĕ);
#endif
scalar_stage<Inverse>(a,n,h,ĕ);}ę transform(Ě&a,bool Ë=false){À n=a þ;ä(Ã has_single_bit(n)&&n<=(À(1)<<23));static Roots ĕ;ĕ.prepare(n);bool è=false;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
è=__builtin_cpu_supports("avx2");
#endif
if(Ë){À h=1;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
if(è&&n>=8)leaf8<true>(a ú,n,ĕ.Ë ú),h=8;
#endif
for(;h<n;h*=2)stage<true>(a ú,n,h,ĕ.Ë ú+h,è);scale(a,â(n).inv());}else{À h=n/2 Ú(;h>=8;h/=2)stage<false>(a ú,n,h,ĕ.Ć ú+h,è);
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
if(è&&n>=8)Å leaf8<false>(a ú,n,ĕ.Ć ú);
#endif
for(;h;h/=2)stage<false>(a ú,n,h,ĕ.Ć ú+h,è);}}inline Ě ò(È Ě&a,È Ě&b){if(a.ą||b.ą)Å{};[[maybe_unused]]constexpr À limit=À(1)<<23;ä(a þ<=limit&&b þ<=limit&&a þ+b þ-1<=limit);À ï=a þ+b þ-1;if(ġ(a þ,b þ)<=32){Ě Ä(ï)Ú(Þ a þ ü)for(À j=0;j<b þ;++j)Ä[i+j]+=a õ*b[j];ø}À n=Ã bit_ceil(ï);Ě x=a;x Ò(n);transform(x);if(&a==&b){pointwise(x,x);}else{Ě y=b;y Ò(n);transform(y);pointwise(x,y);}transform(x,true);x Ò(ï);Å x;}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
î=â>struct Ç:ì{static_assert(T ö==998244353);using base=ì;using F=Ç;using base::base;Ç()=default;Ç(base a):base(Ę(a)){}F&Ð=(base a){base::Ð=(Ę(a));ĥ;}F&Ð=(Ã initializer_list<T>a){base::Ð=(a);ĥ;}static F ò(Ā a,Ā b){å(Ã is_same_v<T,â>)Å Â ò(a,b);else{Ě x(a þ),y Ú(Þ a þ ü)x õ=â::raw(a õ.val());if(&a!=&b){y Ò(b þ)Ú(Þ b þ ü)y õ=â::raw(b õ.val());}ð z=Â ò(x,&a==&b?x:y);F Ä(z þ)Ú(Þ z þ ü)Ä õ=T::raw(z õ.val());ø}}F Ð-()È{F r=Ė Ú(T&x:r)x=-x;Å r;}F&Ð+=(Ā g){Ĥ(Ė)õ+=g õ;ĥ;}F&Ð-=(Ā g){Ĥ(Ė)õ-=g õ;ĥ;}F&Ð+=(T x){if(Í ą)Í resize(1);(Ė)[0]+=x;ĥ;}F&Ð-=(T x){ĥ+=-x;}F&Ð*=(Ā g){ĥ=ò(Ė,g);}F&Ð/=(Ā g){ĥ=Ģ(g);}F&Ð%=(Ā g){ĥ=divmod(g).second;}F&Ð*=(T x){for(T&v:Ė)v*=x;ĥ;}F&Ð/=(T x){ĥ*=x.inv();}Ï+(F a,Ā b){Å a+=b;}Ï-(F a,Ā b){Å a-=b;}Ï*(Ā a,Ā b){Å ò(a,b);}Ï/(Ā a,Ā b){Å a.Ģ(b);}Ï%(Ā a,Ā b){Å a.divmod(b).second;}Ï+(F a,T b){Å a+=b;}Ï-(F a,T b){Å a-=b;}Ï*(F a,T b){Å a*=b;}Ï/(F a,T b){Å a/=b;}Ï+(T a,F b){Å b+=a;}Ï-(T a,Ā b){Å-b+a;}Ï*(T a,F b){Å b*=a;}F&Ð<<=(ó d){ä(d>=0);È ð n=ô;if(À(d)>=n)ĥ=F(n);Í insert(Í Ñ,d,ā);Í resize
(n);ĥ;}F&Ð>>=(ó d){ä(d>=0);È ð n=ô;Í erase(Í Ñ,Í Ñ+ġ(n,À(d)));Í resize(n);ĥ;}Ï<<(F a,ó d){Å a<<=d;}Ï>>(F a,ó d){Å a>>=d;}F pre(ó n)È{ä(n>=0);Å F(Í Ñ,Í Ñ+ġ(À(n),ô));}void ñ{while(!Í ą&&Í back()==ā)Í pop_back();}void trim(){ñ;}T eval(T x)È{T Ä=0 Ú(ð it=Í rbegin();it!=Í rend();++it)Ä=Ä*x+*it;ø}F diff()È{F Ä(ô)Ú(ë 1;i<ô ü)Ä[i-1]=(Ė)õ*T(i);ø}F integral()È{ä(ô<T ö);F Ä(ô);È ð&inv=inverses(Ö ó>(ô))Ú(ë 1;i<ô ü)Ä õ=(Ė)[i-1]*inv õ;ø}void onemul(ó d,T c){ä(d>=0);if(!d){Ė*=T(1)+c;Å;}for(ë ô;i>À(d);--i)(Ė)[i-1]+=(Ė)[i-1-d]*c;}void onediv(ó d,T c){ä(d>=0);if(!d){Ė/=T(1)+c;Å;}for(ë d;i<ô ü)(Ė)õ-=(Ė)[i-d]*c;}F inv(ó deg=-1)È{Ü ä(!Í ą&&(Ė)[0]!=ā)Û(native().inv(n));else{È ó ĉ(n),á=ġ(ě,32);F Ä(á);È T Ë=(Ė)[0].inv();Ä[0]=Ë Ú(Đ 1;i<á ü){Ý(Ė)à-sum*Ë;}for(ó k=á;k<ě;k*=2)inverse_step(Ė,Ä,ġ(k*2,ě));Ä Ò(n)Ú(Đ ě ù){Ý(Ė)à-sum*Ë;}ø}}F Ģ(Ā g,ó deg=-1)È{Ü ä(!g.ą&&g[0]!=ā)Û(native().Ģ(g.native(),n));else{if(n<=32){F Ä(n);È T Ë=g[0].inv()Ú(Đ 0 ù){Ă Ë;}ø}È ó ĉ(n),k=Ã bit_floor(unsigned(ě-1)),ï=k*2;F Ë=g.inv(k),Ä=pre(k)
;È T constant=Ë[0];Ë Ò(ï),Ä Ò(ï)Á(Ë)Á(Ä)ç(Ä,Ë)Á(Ä,true)Ġ(Ä.Ñ+k,Ä.end(),ā);F quotient=Ä,Ì=g.pre(ě);Ì Ò(ï)Á(quotient)Á(Ì)ç(Ì,quotient)Á(Ì,true)Ġ(đ,đ+k,ā)Ú(Đ k;i<ě ü)Ì õ=(À(i)<ô?(Ė)õ:ā)-Ì õ Ġ(đ+ě,Ì.end(),ā)Á(Ì)ß;Ä Ò(n);Ã copy(đ+k,đ+ě,Ä.Ñ+k)Ú(Đ ě ù){Ă constant;}ø}}F log(ó deg=-1)È{Ü ä(!Í ą&&(Ė)[0]==T(1))Û(native().log(n));else{F Ä=pre(n).diff().pre(n-1).Ģ(pre(n),n-1);Ä Ò(n);Å Ä.integral();}}F exp(ó deg=-1)È{Ü ä(Í ą||(Ė)[0]==ā)Û(native().exp(n));else{È ó ĉ(n),á=ġ(ě,32);È ð&inv=inverses(n);F Ä(á);Ä[0]=T(1)Ú(Đ 1;i<á ü){Ý T(j)*(Ė)à sum*inv õ;}if(ě==n&&á==n)ø F Ċ=Ä.inv(á)Ú(ó k=á;k<ě;k*=2){È ó ï=k*2,m=ġ(ï,ě);if(Ċ þ<À(k))inverse_step(Ä,Ċ,k);F ď=Ä,Ë=Ċ,Ì(ï);ď Ò(ï),Ë Ò(ï)Á(ď)Á(Ë)Ú(Đ 1;i<m&&À(i)<ô ü)Ì[i-1]=(Ė)õ*T(i)Á(Ì)ç(Ì,ď)Á(Ì,true)Ġ(đ,đ+k-1,ā)Ġ(đ+m-1,Ì.end(),ā)Á(Ì)ß Ú(Đ m-1;i>=k;--i)Ì õ=Ì[i-1]*inv õ Ġ(đ,đ+k,ā)Ġ(đ+m,Ì.end(),ā)Á(Ì)ç(Ì,ď)Á(Ì,true);Ä Ò(m);Ã copy(đ+k,đ+m,Ä.Ñ+k);}Ä Ò(n)Ú(Đ ě ù){Ý T(j)*(Ė)à sum*inv õ;}ø}}F sqrt(ó deg=-1)È{È ó n=precision(deg);if(!n)Å{}Û(native().sqrt(n));else{ó ć=0,last=
Ö ó>(ġ(ô,À(n)));while(ć<last&&(Ė)[ć]==ā)++ć;if(ć==last)Å F(n);if(ć&1)Å{};ð constant=scalar_sqrt((Ė)[ć]);if(!constant)Å{};È ó shift=ć/2,target=n-shift,ĉ(target),á=ġ(ě,32);F f(Í Ñ+ć,Í Ñ+last),Ä(á);È T half=T(2).inv(),factor=(T(2)**constant).inv();Ä[0]=*constant Ú(Đ 1;i<á ü)ă if(á==target){Ď}F Ċ=Ä.inv(á)Ú(ó k=á;k<ě;k*=2){È ó ï=k*2,m=ġ(ï,ě);F Ì;if(Ċ þ<À(k))inverse_step(Ä,Ċ,k,&Ì);if(Ì.ą){Ì=Ä Á(Ì);}F Ë=Ċ ç(Ì,Ì)Á(Ì,true);Ì Ò(ï),Ë Ò(ï)Ú(Đ 0;i<m-k ü){T low=À(i)<f þ?f õ:ā;T high=À(k+i)<f þ?f[k+i]:ā;Ì õ=(low+high-Ì õ)*half;}Ã fill(đ+m-k,Ì.end(),ā)Á(Ì)Á(Ë)ß;Ä Ò(m);Ã copy_n(đ,m-k,Ä.Ñ+k);}Ä Ò(target)Ú(Đ ě;i<target ü)ă Ď}}F pow(long long k)È{ä(k>=0);È ó n=precision(-1);if(!n)Å{};if(!k){F Ä(n);Ä[0]=1;ø}if(k==1)ĥ;ó ć=0;while(ć<n&&(Ė)[ć]==ā)++ć;if(ć==n||ć>(n-1)/k)Å F(n);È ó shift=Ö ó>(ć*k),len=n-shift;È T lead=(Ė)[ć];F f(Í Ñ+ć,Í end());f Ò(len);f/=lead;F Ä=(f.log(len)*T(k)).exp(len)*lead.pow(k);Ď}Ã pair<F,F>divmod(Ā divisor)È{F a=Ė,b=divisor;a.ñ,b.ñ;ä(!b.ą);if(a þ<b þ)Å{{},Ę(a)};ä(a þ<=(À(1)<<22));È ó qn
=Ö ó>(a þ-b þ+1),m=Ö ó>(b þ);if(m<=32||À(m)*qn<=2048){F q(qn);È T Ë=b.back().inv()Ú(Đ qn-1;i>=0;--i){T æ=q õ=a[i+m-1]*Ë Ú(ó j=0;j<m;++j)a[i+j]-=æ*b[j];}a Ò(m-1);a.ñ,q.ñ;Å{Ę(q),Ę(a)};}F ra(a.rbegin(),a.rend()),rb(b.rbegin(),b.rend());F q=ra.Ģ(rb,qn);Ã reverse(q.Ñ,q.end());F r=a-ò(q,b);r Ò(m-1);r.ñ,q.ñ;Å{Ę(q),Ę(r)};}private:static ó newton_precision(ó n){È ó prefix=Ã bit_floor(unsigned(n));Å prefix>=4096&&n-prefix<=4?prefix:n;}using Native=Ç<â>;Native native()È{Native Ä(ô)Ú(Þ ô ü)Ä õ=â::raw((Ė)õ.val());ø}static F from_native(È Native&a){F Ä(a þ)Ú(Þ a þ ü)Ä õ=T::raw(a õ.val());ø}static void inverse_step(Ā f,F&Ä,ó n,F*ď=nullptr){È ó k=Ö ó>(Ä þ),ï=k*2;ä(k>0&&Ã has_single_bit(unsigned(k))&&k<n&&n<=ï);F Ì=f.pre(n),Ë=Ä;Ì Ò(ï),Ë Ò(ï)Á(Ì);if(ď)*ď=Ì Á(Ë)ß Ġ(đ,đ+k,ā)Á(Ì)ß;Ä Ò(n)Ú(Đ k ù)Ä õ=-Ì õ;}ó precision(ó deg)È{ä(ô<=À(Ã numeric_limits<ó>::max()));ó n=deg<0?Ö ó>(ô):deg;ä(0<=n&&n<=(1<<22));Å n;}static È ì&inverses(ó n){static ì inv{ā,T(1)}Ú(Đ Ö ó>(inv þ)ù)inv.push_back(T(T ö-T ö/i)*inv[T ö%i]);Å
 inv;}static Ã optional<T>scalar_sqrt(T a){if(a==ā)Å ā;if(a.pow((T ö-1)/2)!=T(1))Å Ã nullopt;constexpr ó q=119;T x=a.pow((q+1)/2),t=a.pow(q),c=T(3).pow(q);ó m=23;while(t!=T(1)){Đ 1;T next=t*t;while(i<m&&next!=T(1))next*=next,++i;ä(i<m);T b=c.pow(1LL<<(m-i-1));x*=b;c=b*b;t*=c;m=i;}Å T::raw(ġ(x.val(),T ö-x.val()));}};using light_fps=Ç<>;using LightFPS=light_fps;
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
namespace fps_light_detail{î>struct ProductTree{Ē À size=1,count;Ã vector<F>é;explicit ProductTree(È ì&xs):count(xs þ){ä(count<(À(1)<<23));while(size<count)size<<=1;é Ò(2*size)Ú(Þ size ü)é[size+i]=i<count?F{-xs õ,T(1)}:F{T(1)}Ú(ë size;--i;)é õ=é[2*i+1]þ==1?é[2*i]:F::ò(é[2*i],é[2*i+1]);}ì evaluate(Ā f)È{ì Ä(count);if(!count)ø ð remainder=[](Ā a,Ā b){Å a þ<b þ?a:a.divmod(b).second;};ð visit=[&](ð&&self,À node,Ā æ)->void{if(æ.ą)Å;if(node>=size){Ä[node-size]=æ[0];Å;}self(self,node*2,remainder(æ,é[node*2]));if(é[node*2+1]þ>1)self(self,node*2+1,remainder(æ,é[node*2+1]));};F æ=f;æ.ñ;visit(visit,1,remainder(æ,é[1]));ø}};}namespace light_fps_algorithms{î>Ç<T>taylor_shift(Ç<T>f,Ã type_identity_t<T>c){Ē È À n=f þ;if(n<=1||c==ā)Å f;ä(n<T ö);F factorial(n,T(1)),Ë(n,T(1)),a(n),b(n)Ú(ë 1 ù)factorial õ=factorial[i-1]*T(i);Ë[n-1]=factorial[n-1].inv()Ú(ë n-1;i;--i)Ë[i-1]=Ë õ*T(i);T power=1 Ú(Þ n ü){a[n-1-i]=f õ*factorial õ;b õ=power*Ë õ;power*=c;}a=F::ò(a,b)Ú(Þ n ü)f õ=a[n-1-i]*Ë õ;Å f;}î>T bostan_mori(
Ç<T>p,Ç<T>q,long long k){Ē ä(k>=0&&!q.ą&&q[0]!=ā);p.ñ;q.ñ;while(k&&!p.ą&&q þ>1){F opposite=q Ú(ë 1;i<opposite þ;i+=2)opposite õ=-opposite õ;F numerator=F::ò(p,opposite),č=F::ò(q,opposite);p Ò((numerator þ+1-À(k&1))/2)Ú(Þ p þ ü)p õ=numerator[2*i+À(k&1)];q Ò((č þ+1)/2)Ú(Þ q þ ü)q õ=č[2*i];p.ñ;q.ñ;k>>=1;}Å Ą(k)<p þ?p[À(k)]/q[0]:ā;}î>T linear_recurrence(È ì&á,È ì&coefficients,long long k){Ē ä(k>=0&&á þ==coefficients þ);È À n=á þ;if(Ą(k)<n)Å á[À(k)];if(!n)Å ā;F q(n+1);q[0]=T(1)Ú(Þ n ü)q[i+1]=-coefficients õ;F p=F::ò(F(á.Ñ,á.end()),q).pre(Ö ó>(n));Å bostan_mori(Ę(p),Ę(q),k);}î>ì multipoint_eval(È Ç<T>&f,È ì&xs){if(f þ<=32||xs þ<=32){ì Ä;Ä.reserve(xs þ)Ú(T x:xs)Ä.push_back(f.eval(x));ø}Å Â ProductTree<T>(xs).evaluate(f);}î>Ç<T>interpolate(È ì&xs,È ì&ys){Ē ä(xs þ==ys þ);È À n=xs þ;if(!n)Å{};Â ProductTree<T>tree(xs);F derivative=tree.é[1].diff();derivative.ñ;ì č=tree.evaluate(derivative),prefix(n+1,T(1))Ú(Þ n ü){ä(č õ!=ā);prefix[i+1]=prefix õ*č õ;}T Ë=prefix[n].inv();Ã vector<F>æ(2*tree.size)Ú(
ë n;i--;){æ[tree.size+i]=F{ys õ*Ë*prefix õ};Ë*=č õ;}for(ë tree.size;--i;){æ õ=F::ò(æ[2*i],tree.é[2*i+1])+F::ò(æ[2*i+1],tree.é[2*i]);æ[2*i]=F();æ[2*i+1]=F();}æ[1]Ò(n);Å Ę(æ[1]);}}
#undef À
#undef Á
#undef Â
#undef Ã
#undef Ä
#undef Å
#undef Ç
#undef È
#undef É
#undef Ê
#undef Ë
#undef Ì
#undef Í
#undef Ï
#undef Ð
#undef Ñ
#undef Ò
#undef Ó
#undef Õ
#undef Ö
#undef Ø
#undef Ù
#undef Ú
#undef Û
#undef Ü
#undef Ý
#undef Þ
#undef ß
#undef à
#undef á
#undef â
#undef ã
#undef ä
#undef å
#undef æ
#undef ç
#undef è
#undef é
#undef ê
#undef ë
#undef ì
#undef î
#undef ï
#undef ð
#undef ñ
#undef ò
#undef ó
#undef ô
#undef õ
#undef ö
#undef ø
#undef ù
#undef ú
#undef û
#undef ü
#undef ý
#undef þ
#undef ÿ
#undef Ā
#undef ā
#undef Ă
#undef ă
#undef Ą
#undef ą
#undef Ć
#undef ć
#undef Ĉ
#undef ĉ
#undef Ċ
#undef ċ
#undef Č
#undef č
#undef Ď
#undef ď
#undef Đ
#undef đ
#undef Ē
#undef ē
#undef Ĕ
#undef ĕ
#undef Ė
#undef ė
#undef Ę
#undef ę
#undef Ě
#undef ě
#undef Ĝ
#undef ĝ
#undef Ğ
#undef ğ
#undef Ġ
#undef ġ
#undef Ģ
#undef ģ
#undef Ĥ
#undef ĥ
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif
