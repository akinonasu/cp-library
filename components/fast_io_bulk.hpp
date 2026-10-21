#ifndef FAST_IO_BULK_HPP
#define FAST_IO_BULK_HPP
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#include <limits>
#include <span>
#include <utility>
#include "fast_io.hpp"
#define À std::size_t
#define Á std::
#define Â const auto
#define Ã const
#define Ä unsigned
#define Å reinterpret_cast<
#define Æ _mm256_set1_epi32
#define Ç invalid
#define È count
#define É uint32_t
#define Ê fast_io_detail
#define Ë value
#define Ì Fields
#define Í return
#define Î values
#define Ï FastOutput::quads
#define Ñ =_mm256_extract_epi32
#define Ò kCapacity
#define Ó static
#define Ô after_separator
#define Õ _mm_loadu_si128
#define Ö template<class Emit
#define Ø scratch
#define Ù .data()
#define Ú done
#define Û _mm256_madd_epi16
#define Ü out
#define Ý 100000000
#define Þ __m128i*>
#define ß _mm256_setzero_si256();
#define à _mm256_set1_epi8
#define á bytes
#define â Á memcpy
#define ã Ã char*
#define ä throw Á runtime_error("output failed");
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace Ê{struct BulkInput{alignas(32)Ó inline constexpr auto rows=[]{Á array<Á array<Ä char,16>,11>r{};for(int n=0;n<=10;++n)for(int i=0;i<16;++i)r[n][i]=i<16-n?128:i-(16-n);Í r;}();Ó constexpr À kChunk=1<<17,Ò=kChunk/4+66;Ó Ä separators(__m256i á){Í Ä(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(á,à(' ')),à(' '))));}[[gnu::always_inline]]Ó __m256i two(ã&p,__m256i&Ç){Â á=_mm256_loadu_si256(Å Ã __m256i*>(p));Ã Ä sep=separators(á),a=Á countr_zero(sep),b=Á countr_zero(sep&(sep-1));if(a<1||a>10||b<=a+1||b-a-1>10)[[unlikely]]{Ç=Æ(-1);p+=Á min(b+1,32u);Í ß}Â window=_mm256_inserti128_si256(á,Õ(Å Ã Þ(p+a+1)),1);Â control=_mm256_set_m128i(Õ(Å Ã Þ(rows[b-a-1]Ù)),Õ(Å Ã Þ(rows[a]Ù)));Â digits=_mm256_shuffle_epi8(_mm256_sub_epi8(window,à('0')),control);Ç=_mm256_or_si256(Ç,_mm256_subs_epu8(digits,à(9)));p+=b+1;Í Û(_mm256_maddubs_epi16(digits,_mm256_set1_epi16(0x010a)),Æ(0x00010064));}Ó Á É one(ã&p,bool&bad){Â á=Õ(Å Ã Þ(p));Â spaces=_mm_cmpeq_epi8(_mm_max_epu8(á,_mm_set1_epi8(' ')),_mm_set1_epi8
(' '));Ã Ä n=Á countr_zero(Ä(_mm_movemask_epi8(spaces))|0x10000u);if(n<1||n>10)[[unlikely]]{bad=true;p+=Á min(n+1,16u);Í 0;}Â digits=_mm_shuffle_epi8(_mm_sub_epi8(á,_mm_set1_epi8('0')),Õ(Å Ã Þ(rows[n]Ù)));Â Ç=_mm_subs_epu8(digits,_mm_set1_epi8(9));bad|=!_mm_testz_si128(Ç,Ç);Â groups=_mm_madd_epi16(_mm_maddubs_epi16(digits,_mm_set1_epi16(0x010a)),_mm_set1_epi32(0x00010064));Â halves=_mm_madd_epi16(_mm_packus_epi32(groups,groups),_mm_set1_epi32(0x00012710));Ã Á uint64_t Ë=Á uint64_t(Ä(_mm_cvtsi128_si32(halves)))*Ý+Ä(_mm_extract_epi32(halves,1));bad|=Ë>0xffffffffULL;p+=n+1;Í Á É(Ë);}Ó ã Ô(ã p){Ã Ä mask=separators(_mm256_loadu_si256(Å Ã __m256i*>(p)));Í mask?p+Á countr_zero(mask)+1:nullptr;}Ó À lockstep(ã(&streams)[4],ã Ã(&end)[4],Ä*Ã(&Ü)[4],__m256i&Ç){ã p[4]={streams[0],streams[1],streams[2],streams[3]};À Ú=0;for(;;){Á ptrdiff_t left=end[0]-p[0];for(int k=1;k<4;++k)left=Á min(left,end[k]-p[k]);Â steps=Á min(left/33,Á ptrdiff_t((Ò-Ú)/2));if(steps<=0)break;for(auto j=steps;j;--j,Ú+=2){Â g0=
two(p[0],Ç),g1=two(p[1],Ç),g2=two(p[2],Ç),g3=two(p[3],Ç);Â k=Æ(0x00012710);Â h01=_mm256_castsi256_ps(Û(_mm256_packus_epi32(g0,g1),k));Â h23=_mm256_castsi256_ps(Û(_mm256_packus_epi32(g2,g3),k));Â high=_mm256_castps_si256(_mm256_shuffle_ps(h01,h23,0x88));Â low=_mm256_castps_si256(_mm256_shuffle_ps(h01,h23,0xdd));Â overflow=_mm256_or_si256(_mm256_cmpgt_epi32(high,Æ(42)),_mm256_and_si256(_mm256_cmpeq_epi32(high,Æ(42)),_mm256_cmpgt_epi32(low,Æ(94967295))));Ç=_mm256_or_si256(Ç,overflow);Â Ë=_mm256_add_epi32(_mm256_mullo_epi32(high,Æ(Ý)),low);Â q=_mm256_permutevar8x32_epi32(Ë,_mm256_setr_epi32(0,4,1,5,2,6,3,7));Â q01=_mm256_castsi256_si128(q),q23=_mm256_extracti128_si256(q,1);_mm_storel_epi64(Å Þ(Ü[0]+Ú),q01);_mm_storeh_pd(Å double*>(Ü[1]+Ú),_mm_castsi128_pd(q01));_mm_storel_epi64(Å Þ(Ü[2]+Ú),q23);_mm_storeh_pd(Å double*>(Ü[3]+Ú),_mm_castsi128_pd(q23));}}for(int k=0;k<4;++k)streams[k]=p[k];Í Ú;}Ö>Ó À chunk_blocks(ã&p,ã limit,À È,Ä*Ã(&Ü)[4],Emit&&emit){Â á=Á min({kChunk,2*(È-32),À(limit-p)-64}
)&~À(3);Ã À stream=á/4;ã p1=Ô(p+stream),*p2=Ô(p+2*stream);ã p3=Ô(p+3*stream),*last=Ô(p+á);if(!p1||!p2||!p3||!last)Í 0;ã s[4]={p,p1,p2,p3},*Ã end[4]={p1,p2,p3,last};auto Ç=ß Â Ú=lockstep(s,end,Ü,Ç);bool bad=!_mm256_testz_si256(Ç,Ç);À n[4];for(int k=0;k<4;++k){n[k]=Ú;while(s[k]<end[k]&&n[k]<Ò)Ü[k][n[k]++]=one(s[k],bad);bad|=s[k]!=end[k];}Â parsed=n[0]+n[1]+n[2]+n[3];if(bad||parsed>È)Í 0;for(int k=0;k<4;++k)emit(Ü[k],n[k]);p=last;Í parsed;}Ó À chunk(ã&p,ã limit,Ä*dst,À È,Ä*Ø){Ä*Ã Ü[4]={dst,Ø,Ø+Ò,Ø+2*Ò};bool first=true;Ä*next=dst;Í chunk_blocks(p,limit,È,Ü,[&](Ã Ä*Î,À n){if(!first)â(next,Î,n*sizeof(Ä));first=false;next+=n;});}Ö>Ó void read_blocks(FastInput&in,À È,Emit emit){Á unique_ptr<Ä[]>Ø;À Ú=0;bool chunks=true;while(Ú<È){
			const char *p = in.current;
while(p!=in.end&&static_cast<Ä char>(*p)<=' ')++p;while(chunks&&È-Ú>=1024&&in.end-p>=1088){if(!Ø)Ø=Á make_unique_for_overwrite<Ä[]>(4*Ò);Ä*Ã Ü[4]={Ø.get(),Ø.get()+Ò,Ø.get()+2*Ò,Ø.get()+3*Ò};Â parsed=chunk_blocks(p,in.end,È-Ú,Ü,emit);if(!parsed){chunks=false;break;}Ú+=parsed;}Ä tail[64];À used=0;while(Ú+2<=È&&in.end-p>=32){ã next=p;auto Ç=ß Â groups=two(next,Ç);if(!_mm256_testz_si256(Ç,Ç))break;Â halves=Û(_mm256_packus_epi32(groups,groups),Æ(0x00012710));Ã Ä h0 Ñ(halves,0),l0 Ñ(halves,1);Ã Ä h1 Ñ(halves,4),l1 Ñ(halves,5);if(h0>42||(h0==42&&l0>94967295)||h1>42||(h1==42&&l1>94967295))break;tail[used++]=h0*Ý+l0;tail[used++]=h1*Ý+l1;if(used==64){emit(tail,used);used=0;}Ú+=2;p=next;}if(used)emit(tail,used);
			in.current = p;
if(Ú<È){Ä Ë=0;in.read(Ë);emit(&Ë,1);++Ú;}}}Ó void read(FastInput&in,Ä*Ü,À È){read_blocks(in,È,[&](Ã Ä*Î,À n){â(Ü,Î,n*sizeof(Ä));Ü+=n;});}};}
#pragma GCC pop_options
#endif
namespace Ê{struct BulkOutput{Ó char*small(char*Ü,Á É Ë){Ä n=1+(Ë>=10)+(Ë>=100)+(Ë>=1000);if constexpr(Á endian::native==Á endian::little){Á É word;â(&word,Ï[Ë]Ù,4);word>>=(4-n)*8;â(Ü,&word,4);}else â(Ü,Ï[Ë]Ù+4-n,n);Í Ü+n;}Ó char*number(char*Ü,Á É Ë){if(Ë>=Ý){Ü=small(Ü,Ë/Ý);Ë%=Ý;â(Ü,Ï[Ë/10000]Ù,4);â(Ü+4,Ï[Ë%10000]Ù,4);Í Ü+8;}if(Ë>=10000){Ü=small(Ü,Ë/10000);â(Ü,Ï[Ë%10000]Ù,4);Í Ü+4;}Í small(Ü,Ë);}template<À Ì,class Read>
#if defined(__GNUC__) || defined(__clang__)
[[gnu::noinline]]
#endif
Ó void records(FastOutput&Ü,À È,Read read){constexpr auto á=11*Ì;constexpr auto block=Á max<À>(1,(1<<16)/á);for(À base=0;base<È;){Â n=Á min(È-base,block);char*begin=Ü.reserve(n*á),*p=begin;for(À i=0;i<n;++i){auto Î=read(base+i);for(À j=0;j<Ì;++j){p=number(p,Á É(Î[j]));*p++=j+1==Ì?'\n':' ';}}Ü.length+=p-begin;base+=n;}}Ó void write(FastOutput&Ü,Á string_view text){if(text.size()<=(1<<16))Í Ü.write(text);if(Ü.length&&!Ü.flush())ä À Ú=0;while(Ú<text.size()){Â written=Á fwrite(text Ù+Ú,1,text.size()-Ú,Ü.file);if(!written)ä Ú+=written;}}};}namespace Ê{Ö,À...I>void emit_record(Emit&emit,Ã Á É*Î,Á index_sequence<I...>){emit(Î[I]...);}}Ö>void read_unsigned_blocks(FastInput&input,À È,Emit emit){
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(__builtin_cpu_supports("avx2")){Ê::BulkInput::read_blocks(input,È,emit);Í;}
#endif
Á É Î[64];while(È){Â n=Á min<À>(È,64);for(À i=0;i<n;++i)input.read(Î[i]);emit(Î,n);È-=n;}}inline void read_unsigned(FastInput&input,Á span<Á É>Ü){read_unsigned_blocks(input,Ü.size(),[&](Ã Á É*Î,À È){â(Ü Ù,Î,È*sizeof(Á É));Ü=Ü.subspan(È);});}template<À Ì,class Emit>void read_unsigned_records(FastInput&input,À È,Emit emit){static_assert(Ì>0);if(È>Á numeric_limits<À>::max()/Ì)throw Á length_error("input size");Á array<Á É,Ì>pending;À used=0;constexpr auto indices=Á make_index_sequence<Ì>();read_unsigned_blocks(input,È*Ì,[&](Ã Á É*Î,À n){À i=0;if(used){while(used<Ì&&i<n)pending[used++]=Î[i++];if(used==Ì){Ê::emit_record(emit,pending Ù,indices);used=0;}}for(;i<n&&n-i>=Ì;i+=Ì)Ê::emit_record(emit,Î+i,indices);while(i<n)pending[used++]=Î[i++];});}template<À Ì,class Read>void write_unsigned_records(FastOutput&output,À È,Read read){static_assert(Ì>0&&Ì<=Á numeric_limits<À>::max()/11);Ê::BulkOutput::records<Ì>(output,È,Á move(read));}
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
#undef Ê
#undef Ë
#undef Ì
#undef Í
#undef Î
#undef Ï
#undef Ñ
#undef Ò
#undef Ó
#undef Ô
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
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif
