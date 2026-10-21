#ifndef CP_COMPONENTS_FPS_IO_HPP
#define CP_COMPONENTS_FPS_IO_HPP
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#include <vector>
#include "fast_io_bulk.hpp"
#include "fps_output.hpp"
#include "../math/mint.hpp"
#define À [[gnu::always_inline]]inline
#define Á __m256i
#define Â shuffle
#define Ã const
#define Ä __attribute__((target("avx2")))
#define Å _mm256_mul_epu32
#define Æ _mm256_srli_epi64
#define Ç reinterpret_cast<
#define È std::size_t
#define É =_mm256_blend_epi32
#define Ë Lanes
#define Ì Constants
#define Í return
#define Î constexpr
#define Ï fps_io_fields::detail::
#define Ð Divided
#define Ñ count
#define Ò Ã Á
#define Ó std::uint32_t
#define Ô Ã auto
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fps_io_fields{namespace detail{struct alignas(32)Ë{Ó lane[8];};Î Ë all(Ó x){Í{{x,x,x,x,x,x,x,x}};}Î Ë all16(Ó x){Í all(x*0x10001);}Î int Z=0x80;Î Ë Â(Ã int(&b)[16]){Ë t{};for(int i=0;i<32;++i)t.lane[i/4]|=Ó(b[i%16]&0xFF)<<8*(i%4);Í t;}struct Ì{Ë div10=all(429496730);Ë div10e5=all(1407374884);Ë pack=all(1-10000*65536u);Ë ten=all(10);Ë tail=all(' '<<8|'0');Ë by100=all16(5243),hundred=all16(100);Ë by10=all16(6554);Ë tens_out=all16(2559);Ë ones=all(~0u),zero=all(0x30303030),space=all(0x20202020);Ë chunk0[2]={Â({0,1,2,3,4,5,6,7,Z,Z,8,9,10,11,12,13}),Â({Z,Z,Z,Z,Z,Z,Z,Z,0,1,Z,Z,Z,Z,Z,Z})};Ë chunk1[3]={Â({14,15,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z}),Â({Z,Z,4,5,Z,Z,Z,Z,Z,Z,Z,Z,8,9,Z,Z}),Â({Z,Z,Z,Z,0,1,2,3,4,5,6,7,Z,Z,8,9})};Ë chunk2[3]={Â({10,11,12,13,14,15,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z}),Â({Z,Z,Z,Z,Z,Z,12,13,Z,Z,Z,Z,Z,Z,Z,Z}),Â({Z,Z,Z,Z,Z,Z,Z,Z,0,1,2,3,4,5,6,7})};Ë chunk3[3]={Â({0,1,Z,Z,Z,Z,Z,Z,Z,Z,4,5,Z,Z,Z,Z}),Â({Z,Z,8,9,10,11,12,13,14,15,Z,Z,Z,Z,Z,Z}),Â({Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,0,1,2,3})};Ë chunk4[2
]={Â({4,5,6,7,Z,Z,8,9,10,11,12,13,14,15,Z,Z}),Â({Z,Z,Z,Z,8,9,Z,Z,Z,Z,Z,Z,Z,Z,12,13})};};inline Î Ì kConstants{};À Á load(Ã Ë&l){Í _mm256_load_si256(Ç Ò*>(&l));}À Á Â(Á x,Ã Ë&control){Í _mm256_shuffle_epi8(x,load(control));}struct Ð{Á halves,tail;};À Ð divide(Á v,Á odd,Ã Ì&k){Ò w É(Æ(Å(v,load(k.div10)),32),Å(odd,load(k.div10)),0xAA);Ò h É(Æ(Å(v,load(k.div10e5)),47),Æ(Å(odd,load(k.div10e5)),15),0xAA);Ò halves=_mm256_add_epi32(_mm256_slli_epi32(w,16),_mm256_mullo_epi32(h,load(k.pack)));Ò tail=_mm256_sub_epi32(_mm256_add_epi32(v,load(k.tail)),_mm256_mullo_epi32(w,load(k.ten)));Í{halves,tail};}À Á two_digits(Á z,Ã Ì&k){Ò tens=_mm256_mulhi_epu16(z,load(k.by10));Í _mm256_sub_epi16(_mm256_slli_epi16(z,8),_mm256_mullo_epi16(tens,load(k.tens_out)));}À Á text(Á digits,Ã Ì&k){Ò upto=_mm256_xor_si256(digits,_mm256_add_epi64(digits,load(k.ones)));Í _mm256_blendv_epi8(_mm256_or_si256(digits,load(k.zero)),load(k.space),upto);}À void digits(Ã Ð&d,Á&lo,Á&hi,Ã Ì&k){Ò hundreds=_mm256_srli_epi16(_mm256_mulhi_epu16
(d.halves,load(k.by100)),3);Ò rest=_mm256_sub_epi16(d.halves,_mm256_mullo_epi16(hundreds,load(k.hundred)));lo=text(two_digits(_mm256_unpacklo_epi16(hundreds,rest),k),k);hi=text(two_digits(_mm256_unpackhi_epi16(hundreds,rest),k),k);}À void store(char*p,Ã Ð&lo,Ã Ð&hi,Ã Ì&k){Á d01,d23,d45,d67;Ô either=[&](Á a,Á b)Ä{Í _mm256_or_si256(a,b);};digits(lo,d01,d23,k);digits(hi,d45,d67,k);Ò c0=either(Â(d01,k.chunk0[0]),Â(lo.tail,k.chunk0[1]));Ò c1=either(either(Â(d01,k.chunk1[0]),Â(lo.tail,k.chunk1[1])),Â(d23,k.chunk1[2]));Ò c2=either(either(Â(d23,k.chunk2[0]),Â(lo.tail,k.chunk2[1])),Â(d45,k.chunk2[2]));Ò c3=either(either(Â(hi.tail,k.chunk3[0]),Â(d45,k.chunk3[1])),Â(d67,k.chunk3[2]));Ò c4=either(Â(d67,k.chunk4[0]),Â(hi.tail,k.chunk4[1]));Ô whole=[p](int c,Á chunk)Ä{_mm256_storeu_si256(Ç Á*>(p+64+16*c),chunk);};Ô low=[p](int c,Á chunk)Ä{_mm_store_si128(Ç __m128i*>(p+16*c),_mm256_castsi256_si128(chunk));};whole(4,c4),whole(3,c3),whole(2,c2),whole(1,c1),whole(0,c0);low(0,c0),low(1,c1),low(2,c2),low(
3,c3),low(4,c4);}À void divide16(Ã Ó*x,Ð&lo,Ð&hi,Ã Ì&k){Ô at=[x](int i)Ä{Í _mm256_loadu_si256(Ç Ò*>(x+i));};Ô both=[x](int i)Ä{Í _mm256_broadcastsi128_si256(_mm_loadu_si128(Ç Ã __m128i*>(x+i)));};Ò l É(at(0),both(8),0xF0),h É(at(8),both(4),0x0F);lo=divide(l,Æ(l,32),k);hi=divide(h,Æ(h,32),k);}inline void format(char*p,Ã Ó*x,È Ñ,Ã Ì&k){Ð dl,dh;divide16(x,dl,dh,k);for(È i=16;i<Ñ;i+=16,p+=160){Ð nl,nh;divide16(x+i,nl,nh,k);store(p,dl,dh,k);dl=nl,dh=nh;}store(p,dl,dh,k);}}inline Î È kBlock=25600;inline Î È kTextBytes=10*kBlock;}namespace fps_detail{À Á io_values(Á x){Ô p=_mm256_set1_epi32(Mint::mod()),ni=_mm256_set1_epi32(998244351);auto even=_mm256_and_si256(x,_mm256_set1_epi64x(0xffffffffULL));auto odd=Æ(x,32);even=_mm256_add_epi64(even,Å(Å(even,ni),p));odd=_mm256_add_epi64(odd,Å(Å(odd,ni),p));Ô value É(Æ(even,32),odd,0xaa);Í _mm256_min_epu32(value,_mm256_sub_epi32(value,p));}inline void read_coefficients(FastInput&input,std::vector<Mint>&out,È n){if(!n)Í;if(n>out.max_size()-out.size())throw
 std::length_error("fps input");prepare_fps_output(out,out.size()+n);fast_io_detail::BulkInput::read_blocks(input,n,[&](Ã unsigned*values,È Ñ){append_fps_words<false>(out,values,Ñ);});}inline void write_coefficients(FastOutput&output,Ã Mint*in,È n){alignas(32)unsigned values[25600];alignas(32)char text[256000];Ô&k=Ï kConstants;for(È i=0;i<n;i+=25600){Ô Ñ=std::min<È>(25600,n-i),full=Ñ/16*16;È j=0;for(;j+8<=Ñ;j+=8){alignas(32)unsigned words[8];std::memcpy(words,in+i+j,sizeof(words));_mm256_storeu_si256(Ç Á*>(values+j),io_values(_mm256_load_si256(Ç Ò*>(words))));}for(;j<Ñ;++j)values[j]=in[i+j].val();if(full)Ï format(text,values,full,k);if(full<Ñ){alignas(32)unsigned rest[16]={};alignas(16)char last[160];std::copy_n(values+full,Ñ-full,rest);Ï format(last,rest,16,k);std::memcpy(text+10*full,last,10*(Ñ-full));}if(i+Ñ==n)text[10*Ñ-1]='\n';fast_io_detail::BulkOutput::write(output,std::string_view(text,10*Ñ));}if(!n)output.write('\n');}}
#pragma GCC pop_options
#endif
#undef À
#undef Á
#undef Â
#undef Ã
#undef Ä
#undef Å
#undef Æ
#undef Ç
#undef È
#undef É
#undef Ë
#undef Ì
#undef Í
#undef Î
#undef Ï
#undef Ð
#undef Ñ
#undef Ò
#undef Ó
#undef Ô
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif
