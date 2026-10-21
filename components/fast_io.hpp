#ifndef FAST_IO_HPP
#define FAST_IO_HPP
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#define À std::
#define Á value
#define Â make_unique_for_overwrite<char[]>
#define Ã return
#define Ä length
#define Å if constexpr
#define Æ char
#define Ç FastOutput
#define È template<class T>
#define É capacity
#define Ê FastInput
#define Ë offset
#define Ì void write
#define Í buffer.get()
#define Î À size_t
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
namespace fast_io_detail{struct BulkInput;struct BulkOutput;}class Ê{friend struct fast_io_detail::BulkInput;static constexpr Î É=1<<16;FILE*file=nullptr;À unique_ptr<Æ[]>buffer;
	const char *current = "", *end = current;
À string scratch;void*mapping=nullptr;Î mapped_size=0;bool refill(){if(!file)Ã false;Î n=À fread(Í,1,É,file);
		current = buffer.get();
		end = current + n;
Ã n;}static const Æ*scan(const Æ*p,const Æ*end){while(end-p>=8){À uint64_t x;À memcpy(&x,p,8);if((x-0x2121212121212121ULL)&~x&0x8080808080808080ULL)break;p+=8;}while(p!=end&&static_cast<unsigned Æ>(*p)>' ')++p;Ã p;}public:explicit Ê(FILE*input=stdin){
#if defined(__unix__) || defined(__APPLE__)
struct stat status;int fd=fileno(input);auto Ë=ftello(input);if(fd>=0&&Ë>=0&&!fstat(fd,&status)&&S_ISREG(status.st_mode)&&status.st_size>Ë){mapped_size=status.st_size;mapping=mmap(nullptr,mapped_size,PROT_READ,MAP_PRIVATE,fd,0);if(mapping!=MAP_FAILED){
				current = static_cast<const char *>(mapping) + offset;
end=static_cast<const Æ*>(mapping)+mapped_size;Ã;}mapping=nullptr;}
#endif
file=input;buffer=À Â(É);}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
Ê(const Ê&)=delete;Ê&operator=(const Ê&)=delete;~Ê(){
#if defined(__unix__) || defined(__APPLE__)
if(mapping)munmap(mapping,mapped_size);
#endif
}À string_view token(){while(true){
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
if(!refill())Ã{};}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
scratch.assign(begin,end);while(refill()){
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
}Ã scratch;}È bool read(T&Á){Å(À is_integral_v<T>&&!À is_same_v<T,bool>){while(true){
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
if(!refill())Ã false;}Å(À is_unsigned_v<T>&&sizeof(T)==4&&À endian::native==À endian::little){
				if (end - current >= 11) {
À uint64_t x;
					std::memcpy(&x, current, 8);
if(!(((x+0x4646464646464646ULL)|(x-0x3030303030303030ULL))&0x8080808080808080ULL)){x&=0x0f0f0f0f0f0f0f0fULL;x=(x*10+(x>>8))&0x00ff00ff00ff00ffULL;x=(x*100+(x>>16))&0x0000ffff0000ffffULL;À uint64_t v=À uint32_t(x*10000+(x>>32));int n=8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
Á=T(v);Ã true;}}}}T parsed{};
			auto result = std::from_chars(current, end, parsed);
if(result.ec==À errc{}&&(result.ptr!=end?static_cast<unsigned Æ>(*result.ptr)<=' ':!file)){
				current = result.ptr;
Á=parsed;Ã true;}}auto s=token();if(s.empty())Ã false;Å(À is_same_v<T,bool>){assert(s=="0"||s=="1");Á=s[0]=='1';}else Å(À is_integral_v<T>){auto result=À from_chars(s.data(),s.data()+s.size(),Á);assert(result.ec==À errc{}&&result.ptr==s.data()+s.size());(void)result;}else Á=T(s);Ã true;}};class Ç{friend struct fast_io_detail::BulkOutput;inline static constexpr auto quads=[]{À array<À array<Æ,4>,10000>table{};for(int i=0;i<10000;++i)table[i]={Æ('0'+i/1000),Æ('0'+i/100%10),Æ('0'+i/10%10),Æ('0'+i%10)};Ã table;}();FILE*file;À unique_ptr<Æ[]>buffer=À Â(1<<16);Î É=1<<16,Ä=0;Æ*reserve(Î n){if(n>É-Ä){if(!flush())throw À runtime_error("output failed");if(n>É){Î next=À max(n,2*É);buffer=À Â(next);É=next;}}Ã Í+Ä;}public:explicit Ç(FILE*output=stdout):file(output){}Ç(const Ç&)=delete;Ç&operator=(const Ç&)=delete;~Ç(){flush();}bool flush(){Î Ë=0;while(Ë<Ä){auto n=À fwrite(Í+Ë,1,Ä-Ë,file);if(!n){À memmove(Í,Í+Ë,Ä-Ë);Ä-=Ë;Ã false;}Ë+=n;}Ä=0;Ã À fflush(file)==0;}Ì(Æ c){*reserve(1)=c;++Ä;}Ì(__uint128_t
 Á){Æ digits[40];Æ*end=digits+sizeof(digits),*begin=end;do{*--begin=Æ('0'+Á%10);Á/=10;}while(Á);write(À string_view(begin,end-begin));}Ì(__int128_t Á){__uint128_t magnitude=__uint128_t(Á);if(Á<0)write('-'),magnitude=0-magnitude;write(magnitude);}Ì(À string_view s){Æ*out=reserve(s.size());if(!s.empty())À memcpy(out,s.data(),s.size());Ä+=s.size();}È requires À is_integral_v<T>Ì(T Á){Æ*out=reserve(32);Å(À is_same_v<T,bool>)*out='0'+Á,++Ä;else Å(À is_unsigned_v<T>&&sizeof(T)==4){Æ*p=out;if(Á>=100000000){auto high=Á/100000000,low=Á%100000000;p=À to_chars(p,p+2,high).ptr;À memcpy(p,quads[low/10000].data(),4);À memcpy(p+4,quads[low%10000].data(),4);p+=8;}else if(Á>=10000){p=À to_chars(p,p+4,Á/10000).ptr;À memcpy(p,quads[Á%10000].data(),4);p+=4;}else p=À to_chars(p,p+4,Á).ptr;Ä+=p-out;}else Ä+=À to_chars(out,out+32,Á).ptr-out;}È requires requires(const T&Á,Æ*out){Á.decimal_capacity();Á.to_chars(out);}Ì(const T&Á){Æ*out=reserve(Á.decimal_capacity());Ä+=Á.to_chars(out)-out;}È void println(const T
&Á){write(Á);write('\n');}};
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
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif
