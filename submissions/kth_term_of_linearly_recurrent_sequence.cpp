//https://qandori.github.io/cp-library/fps/linear_recurrence.hpp.html
#line 1 "test/fps_linear_recurrence.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#line 1 "fps/linear_recurrence.hpp"


#line 1 "components/fps_backend.hpp"


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <memory>
#include <new>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 7 "math/mint.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct Mint {
	using mint = Mint;
	unsigned v = 0;
	static constexpr unsigned mod() { return 998244353; }
	static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	Mint operator+() const { return *this; }
	Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	Mint operator+(Mint b) const { return Mint(*this) += b; }
	Mint operator-(Mint b) const { return Mint(*this) -= b; }
	Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 27 "components/fps_backend.hpp"
#define Õ std::size_t
#define Ö std::uint32_t
#define Ø std::
#define Ù const
#define Ú return
#define Û .size()
#define Ü __attribute__((target("avx2")))
#define Þ _mm256_mul_epu32
#define ß Arena::footprint
#define à .data()
#define á blocked_ntt::detail::
#define â detail::
#define ã blocked_ntt::detail
#define ä size
#define å _mm256_set1_epi32
#define æ constexpr
#define ç fps_detail::
#define è data()
#define é _mm256_sub_epi32
#define ê detail
#define ì blocked_ntt::
#define í inline void
#define î length
#define ð _mm256_add_epi64
#define ñ _mm256_srli_epi64
#define ò inline
#define ó static_cast<int>
#define ô inverse_roots
#define õ reinterpret_cast<
#define ö int i=0;i<
#define ø ;for
#define ù first
#define ú static_cast<
#define ü .begin()
#define ý int
#define þ count
#define ā namespace
#define Ă begin()
#define ă result
#define Ą template
#define ą _mm256_min_epu32
#define Ć for
#define ć inverse
#define Ĉ void
#define ĉ [[gnu::always_inline]]
#define ċ static
#define Č Transform
#define č words
#define Ď FormalPowerSeries
#define ď stride
#define Đ multiply_mod
#define đ _mm256_setzero_si256()
#define Ē =0;
#define ē blocked_poly
#define Ĕ [i]
#define ĕ broadcast
#define Ė .subspan
#define ė ;++
#define Ę using
#define ę NTT::
#define Ě mint
#define ě Arena
#define Ĝ vectorized()
#define ĝ Ø span<Ö>
#define Ğ Ø span<Ù Ö>
#define ğ Ø span<
#define Ġ Ù Ö
#define ġ if æ(Ø is_same_v<T,Ě>)
#define Ģ Ù Vec
#define ģ Ù Õ
#define Ĥ Ø is_same_v<
#define ĥ even=ð(even,Þ(Þ(even,ni),p));odd=ð(odd,Þ(Þ(odd,ni),p));
#define Ħ Ø min
#define ħ Ø vector<
#define Ĩ if(terms==6)update(Ø integral_constant<ý,6>{});else if(terms==5)update(Ø integral_constant<ý,5>{});else if(terms==4)update(Ø integral_constant<ý,4>{});else if(terms==3)update(Ø integral_constant<ý,3>{});else if(terms==2)update(Ø integral_constant<ý,2>{});else update(Ø integral_constant<ý,1>{});
#define ĩ ;Ù
#define Ī Ø countr_zero
#define ī unsigned
#define Ĭ arena.take
#define ĭ Vec
#define Į ė i
#define į out
#define İ columns
#define ı forward
#define ĳ value
#define Ĵ _mm256_blend_epi32
#define ĵ Ø copy_n
#define Ķ tables
#define ķ roots
#define ĸ scratch
#define Ĺ if æ
#define ĺ ;if
#define Ļ window
#define ļ coefficients
#define Ľ _mm256_permutevar8x32_epi32
#define Ŀ Õ j Ē j
#define ŀ block
#define Ł _mm256_permute2x128_si256
#define ł auto
#define Ń t Ē t<4 ė t
#define ń reduce
#define Ņ Ö*
#define ņ .take
#define Ň _Pragma("GCC target(\"avx2\")")
#define ň _mm256_add_epi32
#define ŉ Ø uint64_t
#define Ŋ Ø max
#define ŋ arena
#define Ō load
#define ō inverse_roots_
#define Ŏ product
#define ŏ _mm256_setr_epi32
#define Ő Ą<class
#define ő recursion.visit
#define Œ Ù Factor
#define œ load8
#define Ŕ montgomery
#define ŕ operator
#define Ŗ inverse_product_sum
#define ŗ store8
#define Ř .resize
#define ř Ø ptrdiff_t
#define Ś false
#define ś take
#define Ŝ recursion
#define ŝ else
#define ş weights
#define Š store
#define š matrix
#define ţ levels
#define Ť alignas(32)
#define ť quotient
#define Ŧ even
#define ŧ scale
#define Ũ class
#define ũ degree
#define Ū ;Ú
#define ū store_unaligned
#define Ŭ [0]
#define ŭ times
#define Ů Õ i=0;i<
#define ů canonical
#define Ű ç ę
#define ű .empty()
#define Ų offset
#define ų ė j
#define Ŵ ċ Ĉ
#define ŵ differentiated
#define Ŷ cyclic_product
#define Ÿ factor
#define Ź multiply
#define ź import_work
#define Ż ý i=
#define ż Ø fill
#define Ž load_unaligned
#define ž append_fps_words<true>
#define ſ inverse_root_quotients
#define ƀ transform
#define Ɓ [j]
#define Ƃ .end()
#define ƃ scratch_words
#define Ƅ true>
#define ƅ Ø move
#define Ɔ true
#define Ƈ Ĕ=
#define ƈ Ø is_trivially_copyable_v<
#define Ɖ t.inverse_product
#define Ƌ error
#define ƌ shift
#define ƍ bool
#define Ǝ static_assert
#define Ə roots_
#define Ɛ _mm256_shuffle_epi32
#define Ƒ twice
#define ƒ =0 ø
#define Ɠ Ġ*
#define Ɣ frequency
#define ƕ products
#define Ɩ [[gnu::noinline]]
#define Ƙ Ę O=ç Operations<T>
#define ƙ prepare_fps_output
#define ƚ sizeof
#define ƛ struct
#define Ɯ Ù U*
#define Ɲ ;i+=8
#define ƞ Ę ā ê
#define Ɵ half
#define Ơ terms
#define ơ assert
#define Ƣ root_quotients
#define ƣ ĩ Factor
#define Ƥ kernels::
#define ƥ fixed_mul8
#define Ʀ +8*
#define Ƨ Half::kUpper
#define ƨ Half::kLower
#define Ʃ target
#define ƪ add
#define ƫ [k]
#define Ƭ Bottom
#define ƭ Factor
#define Ʈ ::č
#define Ư a Û
#define ư [2*i+1]
#define Ʊ ntt_inverse
#define Ʋ ħ Ě>
#define Ƴ values
#define ƴ Ĺ(!Ĥ T,Ě>)
#define Ƶ Layout::
#define ƶ Ù Č&t
#define Ʒ à+
#define Ƹ plan.
#define ƹ [1]
#define ƺ division_detail
#define ƻ Factors{ĭ w,q;};ò ĭ ŭ(ĭ x,Ù Factors&w){Ģ Ŧ=ñ(Þ(x,w.q),32);Ģ odd=Þ(ñ(x,32),ñ(w.q,32));Ģ q=Ĵ(Ŧ,odd,0xAA)Ū é(_mm256_mullo_epi32(x,w.w),_mm256_mullo_epi32(q,ĕ(kP)));}ò
#define Ƽ base
#define ƽ ;i+=
#define ƾ ģ h=nv/
#define ƿ export_work
#define ǀ Õ n
#define ǁ deg
#define ǂ output
#define Ǆ ForwardBottom
#define ǅ Operations<T>
#define ǆ O::
#define Ǉ Recursion
#define ǈ long long
#define ǉ Õ j Ē j<
#define Ǌ tree
#define ǋ =e ø(ö 8 Į){I8 z=œ(Ļ+8-i),r=å(ļ Ĕ);e=ð(e,Þ(z,r));o=ð(o,Þ(ñ(z,32),r));}I8 ni=å(998244351);e=ð(e,Þ(Þ(e,ni),p));o=ð(o,Þ(Þ(o,ni),p));x=Ĵ(ñ(e,32),o,0xaa);x=ą
#define ǌ Matrix
#define Ǎ ĩ ł
#define ǎ LaneLayout::
#define ǐ ł update=[&](ł number){æ ý þ=decltype(number)::ĳ ĺ(j==0)ǆ Ą accumulate_products<þ,Ƅ(č(Ƌ),a,b,î);ŝ ǆ Ą accumulate_products<þ,Ś>(č(Ƌ),a,b,î);}ĺ(Ơ==7)update(Ø integral_constant<ý,7>{});ŝ Ĩ
#define Ǒ ċ æ ƍ kForward=
#define Ǔ æ ł kInverseScales=[]{Ø array<Ø array<Ö,ì kMaxLog+1>,2>s{}ø(ý lg=3;lg<=ì kMaxLog ė lg){s Ŭ[lg]=á power(Ö(1)<<(lg-3),kP-2);s ƹ[lg]=á Đ(s Ŭ[lg],kR);}Ú s;}();ò
#define ǔ inverse_columns
#define Ǖ ø(Õ c Ē c<h;c+=2*width)
#define ǖ Ø array<
#define Ǘ StandardLayout
#define ǘ T::mod()-(T::mod()-1)/
#define Ǚ diff
#define ǚ stack
#define Ǜ Ņ Ù
#define ǜ pairs
#define ǝ Ĵ(ñ(Ŧ,32),odd,0xAA)
#define Ǟ derivative
#define ǟ ;while
#define Ǡ work
#define ǡ ĺ(Ī(nv)%2==0)
#define Ǣ Ĉ ŕ()(Ņ į,Õ þ,Õ ù)Ù
#define ǣ Ù Ě
#define Ǥ cache
#define ǥ [2]
#define Ǧ ą(x,é(x,p))
#define ǧ ý n
#define Ǩ transformed
#define ǩ Ø abort()
#define Ǫ transform_
#define ǫ ĩ ĝ
#define Ǭ ĩ Č t(ŋ,lg);ł
#define ǭ ĥ I8 ĳ=Ĵ(ñ(Ŧ,32),odd,0xaa);ĳ=ą(ĳ,é(ĳ,pp));
#define Ǯ windows
#define ǯ len
#define ǰ this->size()
#define Ǳ f Û
#define ǲ į Û
#define ǳ Ć(Õ j=c;j<c+width ų)
#define Ǵ Ø memcpy
#define ǵ Remainder
#define Ƕ this->
#define Ƿ nullptr
#define Ǹ power
#define ǹ [j+h]
#define Ǻ Ø numeric_limits<double>::infinity()
#define ǻ Õ(1)<<
#define Ǽ leaf_product
#define ǽ blocks
#define Ǿ =_mm256_unpackhi_epi64
#define ǿ =_mm256_unpackhi_epi32
#define Ȁ ţ.level
#define ȁ prepare(ù,Ļ Ŭ)ø(ǉ þ ų,į+=32)
#define Ȃ fixed_butterfly8<ć
#define ȃ .val()
#define Ȅ ò ĭ
#define ȅ ;Ę ntt::â
#define Ȇ ;t.ı
#define ȇ pointwise
#define Ȉ Half::kBoth
#define ȉ ĩ ý
#define Ȋ Tables
#define ȋ kInverseScales
#define Ȍ level
#define ȍ .release(mark)
#define Ȏ odd
#define ȏ &&Ű Ĝ
#define Ȑ count_
#define ȑ .ù
#define Ȓ T(0)
#define ȓ ý j Ē j<
#define Ȕ Õ k
#define ȕ odd_lanes
#define Ȗ composition_poly
#define ȗ InverseProductBottom
#define Ș /16;Ə=Ĭ(2*n8)à;Ø fill_n(Ə,2*n8,0u);ō=Ə+n8;á build_table(Ə,entries,á kRoots Ŭ);á build_table(ō,entries,á kRoots ƹ)
#define ș Ĉ ŕ()(Ņ a,Õ þ,Õ ù)Ù
#define Ț õ ĭ*>
#define ț ;i<n Į
#define Ȝ Ʒ i
#define ȝ ė k
#define Ȟ leaves
#define ȟ Ù ý
#define Ƞ Õ þ
#define ȡ ;ģ
#define Ȣ last
#define ȣ TreeTransform
#define Ȥ ProductBottom
#define ȥ Ż 0 ø
#define Ȧ Ū ă;
#define ȧ Ņ į
#define Ȩ __attribute__((target("avx2"),always_inline))
#define ȩ Ø copy
#define Ȫ Õ c Ē c<
#define ȫ kVectors
#define Ȭ PolynomialHalfGCD
#define ȭ identity
#define Ȯ O Ʈ
#define ȯ index
#define Ȱ t.ķ()
#define ȱ Ģ ni=ĕ(ì Ƥ kNI)
#define Ȳ ø(Õ Ń)
#define ȳ Ć(Õ j=c+width;j<c+2*width ų)v Ɓ=v ǹ=
#define ȴ sizes
#define ȵ inverse_scratch
#define ȶ ȡ n1 ǥ ǥ={{lo,mid},{mid,hi}}ø(ý side Ē side<2 ė side)
#define ȷ ç NTT t(ä);Ű CompositionWords storage(ú ī ý*>(::ŕ new[](8*ä*ƚ(ī ý),Ø align_val_t(64))));ł*p=storage.get()
#define ȸ g Û
#define ȹ ;ć Ř(î);ntt_half(ć)ĩ Word norm=ǘ î;ƀ(1).ŧ(č(ć),î,norm);ł storage=ç allocate_fps_words((2*þ-1)*k4);Word*
#define Ⱥ public:
#define Ȼ Ď<T>
#define ȼ Value
#define Ƚ kInverse
#define Ⱦ private:
#define ȿ ȡ m=projection_length(n)ȉ lg=Ī(m)ĩ Ȋ Ķ(ĸ ȑ(Ȋ Ʈ(m)),m);Carve carve(ĸ Ė(ß(Ȋ Ʈ(m))))ǫ q=carve ņ(4*m)
#define ɀ allocate_fps_words
#define Ɂ qs[8]ø(ȓ 8 ų){ý k=i/8+j;ws Ɓ=(k&1)?mod-ķ[k/2]:ķ[k/2];qs Ɓ=(k&1)?~Ƣ[k/2]:Ƣ[k/2];}inverse_leaf_products8<8
#define ɂ _mm256_loadu_si256
#define Ƀ Ě::mod()
#define Ʉ kComposeBase
#define Ʌ half_residual
#define Ɇ inverse_with
#define ɇ forward_upper
#define Ɉ InverseBottom
#define ɉ repeat_index
#define Ɋ [j+3*h]
#define ɋ [j+2*h]
#define Ɍ if(n>=8&&Ĝ)
#define ɍ ĭ x
#define Ɏ Source
#define ɏ mul8
#define ɐ ĝ į
#define ɑ remainder
#define ɒ precision
#define ɓ Word*
#define ɔ [2*i]
#define ɕ if(j+1<þ)prepare(ù+j+1,Ļ[(j+1)&1])
#define ɖ out_length
#define ɗ divide_by_index
#define ɘ geometric_scale
#define ə <Ś>
#define ɚ [8]
#define ɛ [4]
#define ɜ ŏ(0,1,2,3,4,5,6,7)
#define ɝ Č::Pair
#define ɞ ǆ trim
#define ɟ ĕ(kP)
#define ɠ Č Ʈ(lg)+ß
#define ɡ kChains
#define ɢ QuotientPlan
#define ɣ log_block(n)
#define ɤ kLanes
#define ɥ Window Ļ ǥ
#define ɦ _mm256_castsi256_si128
#define ɧ t.ô()
#define ɨ a Ĕ
#define ɩ >=8&&Ĝ
#define ɪ ::word
#define ɫ ø(ö 4 Į)
#define ɬ Ø bit_ceil
#define ɭ bump ņ(ǯ)
#define ɮ power_projection
#define ɯ step
#define ɰ <=(1<<23)
#define ɱ available
#define ɲ fixed_mul
#define ɳ ;Ŧ=ð(Ŧ,Þ(x,y));Ȏ=ð(Ȏ,Þ(ñ(x,32),ñ(y,32)));
#define ɴ Ø align_val_t(64)
#define ɵ Õ i
#define ɶ log_length
#define ɷ radix4
#define ɸ difference
#define ɹ Þ(ñ(x,32),ñ(y,32))
#define ɺ Ù V&w=ć?ô:ķ ĩ V&
#define ɻ _mm256_unpacklo_epi64
#define ɼ rest
#define ɽ word
#define ɾ add8
#define ɿ æ Õ
#define ʀ á Đ
#define ʁ minus
#define ʂ sum
#define ʃ ē::
#define ʄ stack_
#define ʅ Window
#define ʆ :[a]"+r"(a):[end]"r"(end)
#define ʇ Ø fill_n
#define ʈ transpose
#define ʉ multiply_detail
#define ʊ ü+ř
#define ʋ ø(ǉ h ų)
#define ʌ Ø bit_width
#define ʍ Õ j
#define ʎ inner
#define ʏ multiply_constant
#define ʐ check_length
#define ʑ coefficient
#define ʒ ntt_half
#define ʓ upper
#define ʔ transpose_roots()
#define ʕ convolution_tile8
#define ʖ =å
#define ʗ low
#define ʘ Ŋ<Õ>
#define ʙ Ą<ƍ kNegative>[[gnu::noinline,gnu::flatten]]
#define ʚ ý v Ē v<
#define ʛ node
#define ʜ prefix
#define ʝ from
#define ʞ ŏ(7,6,5,4,3,2,1,0)
#define ʟ storage
#define ʠ Ĺ(Ƭ::kForward)
#define ʡ Ħ<ý>
#define ʢ Ć(ý k Ē k<4 ȝ)
#define ʣ ą(x,é(x,pp))
#define ʤ ñ(x,32)
#define ʥ à Ʀ
#define ʦ Ù ł
#define ʧ Ù F&
#define ʨ PointTree
#define ʩ multiply_lanes
#define ʪ block_
#define ʫ subtract
#define ʬ double
#define ʭ input
#define ʮ entry
#define ʯ convolution_tile
#define ʰ a à
#define ʱ .progress
#define ʲ ;i+8<=
#define ʳ +2*
#define ʴ ȑ(m)
#define ʵ Č::kMinLog
#define ʶ Ȕ=1;k<
#define ʷ ü+
#define ʸ compressed
#define ʹ projection_scratch
#define ʺ even_terms
#define ʻ tables_
#define ʼ residual_sum
#define ʽ reverse_difference
#define ʾ transform_ns
#define ʿ fill_windows
#define ˀ root
#define ˁ reverse
#define ˆ .inv()
#define ˇ inverse_cube_quotients
#define ˈ *this
#define ˉ Ɓ=
#define ˊ Ę ā ã;Ġ ŧ=inverse_scale
#define ˋ ý k Ē k<
#define ˌ Ü Ŵ
#define ˍ montgomery_sums
#define ˎ evaluation_poly
#define ˏ constant
#define ː dst
#define ˑ long_division
#define ˠ ni ʖ(998244351)
#define ˡ I8 p ʖ(mod)
#define ˢ ğ ǣ>
#define ˣ composition_forward_y
#define ˤ ;Ę ǆ
#define ˬ Ő T>
#define ˮ ĉ í
#define Ͱ prefix_
#define ͱ CompositionBottom
#define Ͳ _mm256_srli_si256
#define ͳ I8 x=
#define Ͷ [&](ł ɷ,ʍ,Õ q,Ģ*d,Ģ*)
#define ͷ ȡ mid=lo+(hi-lo)/2;Ǜ left=free_;Ǜ l8=left Ʀ(mid-lo);free_=l8 Ʀ(hi-mid);
#define ͻ t Ƈ _mm256_unpacklo_epi32(r Ĕ,r[i+1])
#define ͼ ;Ę
#define ͽ size_(i6 Û){}Õ mark()Ù{Ú top_;}Ĉ release(Õ mark){top_=mark;}Ņ ś(ǀ)
#define Ϳ Ⱥ explicit
#define Ά Ǝ(ƚ(Ě)==ƚ(Ö)&&ƈ Ě>)
#define Έ Ū r;
#define Ή š.a
#define Ί ò Õ
#define Ό Õ m
#define Ύ TransformStore
#define Ώ ProductTree
#define ΐ load_leaves
#define Α div_series
#define Β load_any
#define Γ log_derivative
#define Δ lg_max
#define Ε fill_window
#define Ζ scale_words
#define Η tail
#define Θ =Ĭ(m)ø(Ů n Į)f Ƈ Ħ(ɨ.v,ɨ.v-Ƀ)ø(Ů m Į)g Ƈ Ħ(b Ĕ.v,b Ĕ.v-Ƀ)
#define Ι ƾ 4;ʠ forward_top4(in,v,h,Ə)Ȳ ő(a ʥ t*h,h,t);Ĺ(Ƭ::Ƚ)inverse_top4
#define Κ Ć(Õ Ń)
#define Λ õ Ø uintptr_t>
#define Μ lead
#define Ν DivisionPlan
#define Ξ kSqrtBase
#define Ο source(in,ƌ,ǲ)
#define Π Half ǂ
#define Ρ inverse_partial8
#define Σ small8<
#define Τ Group
#define Υ [i+1]
#define Φ ;scaled_leaves<kNegative>
#define Χ mid=Ǌ.split(lo,hi)
#define Ψ _mm256_storeu_si256
#define Ω ý Ų=ʁ?n:0 ø(ˋ þ ȝ)accumulate(ː,a ƫ+Ų,b ƫ+Ų,c ƫ+Ų,n,ʁ);
#define Ϊ ȡ nv=Ư/8;ł*v=Ț(ʰ)ǡ
#define Ϋ ǁ=ó(ǰ);ơ(ǁ>=0)ĺ(ǁ==0)Ú{};ơ
#define ά Ű Ą accumulate_wide<þ
#define έ ą(y,é(y,p))
#define ή Ą<ƍ
#define ί ý î
#define ΰ Ù Ȋ&Ķ
#define α product_
#define β reduce_sums(Ŧ ƫ,Ȏ ƫ)
#define γ exp_length(n)
#define δ visit(a Ʀ t*h,h,4*k+t);
#define ε capacity
#define ζ ==0
#define η b Û
#define θ .emplace_back
#define ι ::raw
#define κ ;Ģ
#define λ Ż 1;i<
#define μ divisor
#define ν undo_montgomery
#define ξ row
#define ο mod
#define π Ƌ(n);a Ř(n);ntt(a);inverse_extend(h,a,Ǥ);Ǥ=h;Ǥ Ř(n);ntt(Ǥ);
#define ρ ŗ(a+j+i,x0);ŗ(a+j+ď+i,x1);ŗ(a+j ʳ ď+i,x2);ŗ(a+j+3*ď+i,x3);
#define ς mid
#define σ full=Ħ(n,Ǳ>0?Ǳ-1:0)/8*8;Indices ȯ(1)ø(Ů full Ɲ,ȯ.next())ū
#define τ _mm256_cmpgt_epi32
#define υ inverse_h1
#define φ slot
#define χ rows
#define ψ second
#define ω convolution_stage8
#define ϊ left
#define ϋ stage_pair
#define ό _mm256_mullo_epi32
#define ύ a Ʀ
#define ώ inverse_cube_roots
#define Ϗ ;ɓ q=Ǥ+(i-1)*k4;ĵ(č(g)+Ų-ŀ,ŀ,q);ʒ(q,î);Ű scale_normalize
#define ϐ ;a.trim();q.trim()Ū{ƅ(q),ƅ(a)};
#define ϑ Ǚ(amb,cmd)
#define ϒ ƪ(amb,cmd)
#define ϓ ƪ(ab,cd)
#define ϔ ī ý
#define ϕ ý ä=1 ǟ(ä<ǁ)ä<<=1
#define ϖ d_deg
#define ϗ twice_mod
#define Ϙ visit
#define ϙ InverseProductSumBottom
#define Ϛ leaf_weight
#define ϛ table_words
#define Ϝ stage_pair8
#define ϝ Ʀ t
#define ϟ ;I8 z=œ(a ƫ+Ų+i);Ŧ=ð(Ŧ,Þ(z,x));Ȏ=ð(Ȏ,Þ(ñ(z,32),ʤ));
#define Ϡ ȓ 2 ų
#define ϡ ȑ(2*m)
#define Ϣ Ǘ::î(Ö(m8))
#define ϣ Ƅ(a,n/4,n,k);
#define Ϥ Ő Ƭ>Ĉ
#define ϥ ĭ f ɛ
#define Ϧ ā ê
#define ϧ ó(ǰ)
#define Ϩ Ù Ƭ&b9
#define ϩ Ø has_single_bit
#define Ϫ Õ h
#define ϫ inverse_start_
#define Ϭ InterpolationTree
#define ϭ values_
#define Ϯ coefficient_value
#define ϯ kDivisionTerms
#define ϰ inverse_
#define ϱ from_lower_source
#define ϲ kExpBase
#define ϳ first_touched_
#define ϴ storage_
#define ϵ kModulus
#define Ϸ break;
#define ϸ __m256i
#define Ϲ b Ĕ
#define Ϻ %[a]
#define ϻ inverse_twists
#define ϼ ;case
#define Ͻ kP-
#define Ͼ ;i<
#define Ͽ sums
#define Ѐ [3]
#define Ё 2*ä
#define Ђ ;ʒ(Ƌ);ȇ(Ƌ,Ƌ,ć,î);Ʊ(Ƌ);
#define Ѓ ;Ĺ
#define Є =[&s](ɍ)Ü{Ú ń(ŭ(x,s),kP);}
#define Ѕ ĝ ĸ
#define І Þ(ñ(a,32),ñ(b,32))
#define Ї Õ lo
#define Ј append_division_words
#define Љ k5 Û
#define Њ _mm256_cvtepu32_epi64
#define Ћ {x1=ą(x1,é(x1,p));x2=ą(x2,é(x2,p));x3=ą(x3,é(x3,p));}
#define Ќ Ǚ(ab,cd)
#define Ѝ ũ(lo,hi)
#define Ў ù+
#define Џ ó(ȸ)
#define А input_word
#define Б alignment
#define В ;I8
#define Г Ŭ=
#define Д œ(a+i)
#define Е ö n Į
#define Ж graeffe_scale
#define З quotient_size
#define И vectors
#define Й [2*i+j]
#define К ȥ(ʲ n Ɲ)
#define Л ǚ ņ
#define М Ɠ ķ
#define Н ĺ(ǁ<0)ǁ=
#define О Ğ f
#define П ĕ(2*kP)
#define Р Ć(ǉ h ų)
#define С Ć(ý t Ē t<8 ė t)
#define Т Ć(ö 4 Į)
#define У ø(ʶ 4 ȝ)
#define Ф Coefficient
#define Х touched_
#define Ц join_windows_strided
#define Ч ;Indices ȯ
#define Ш =Ħ
#define Щ {Ú Ħ(x.v,x.v-Ƀ);}
#define Ъ Ǎ fold=[r](ĭ s){Ú ð(Þ(ñ(s,32),r),Ĵ(s,đ,0xAA));}
#define Ы Ɠ Ȟ=a+32*g κ f ɛ={Ō(Ȟ),Ō(Ȟ+8),Ō(Ȟ+16),Ō(Ȟ+24)};ʿ
#define Ь ĩ Node ʛ{Ƶ ʂ(ϊ.ũ,l8.ũ),Ƶ lead_product(ϊ.Μ,l8.Μ)}
#define Э Factors entries(Ɠ n8,Ȕ){Ɠ e=n8+φ(k)Ū{Ō(e),Ō(e+8)};}
#define Ю ;ý ä=1;ħ Ƽ>Ɣ;ł Ǌ=ç Ώ<T>::build(xs,ä,ŀ,Ɔ,&Ɣ)
#define Я +ß
#define а Ū;
#define б ģ m8=(m+7)/8*8
#define в mark=ǚ.mark();
#define г o0 Ѐ={{tc Ѐ,tc Ѐ},{tc ƹ,Ƒ Ŭ},{tc ǥ,Ƒ ǥ}};t.Ŗ
#define д first_transform
#define е scaled
#define ж powers
#define з r[i+4]=Ł(u Ĕ,u[i+4],0x31);
#define и >=0;--
#define й ;ě ŋ
#define к =Ĭ(n)
#define л Ʋ&į
#define м ŝ if
#define н ĭ Ŧ
#define о ī ǈ
#define п Õ hi
#define р _mm256_castsi256_ps
#define с order
#define т lanes
#define у ==1
#define ф =Ĭ
#define х ļ(p,č,u);Ǜ q=Л(ʎ);t_.ı({p+č-ʎ,ʎ},0,{q,ʎ});
#define ц ĩ ProjectionBottom b9{&Ķ,p à,q à,p à};Ġ f8=á power(kP/2+1,31);Ö c=1 ø(ý s=2;s+3<lg ė s){ģ ď=2*(m>>s),rows=Õ(1)<<s;ģ pad=Õ(Ī(ď/16))ĺ(s==2){forward_pruned(p,rows*ď,pad,Ķ,Ǆ{Ķ.ķ});forward_pruned(q,(rows+1)*ď,pad,Ķ,b9);}if(s+4<lg){ģ ŀ=ď/16;Ġ next=á Đ(á Đ(á Đ(c,c),f8),Ö(ŀ));Ġ p8=kInverseScales Ŭ[s+4];Doubling<ForwardBottom>(p,ŀ,Ķ,ForwardBottom{Ķ.ķ}).run(p8,0);Doubling<ProjectionBottom>(q,ŀ,Ķ,b9).run(p8,next+next>=kP?next+next-kP:next+next);c=next;}ŝ{Ġ g8=á power(c,kP-2);Ġ ŧ=á Đ(graeffe_scale(lg+1),á Đ(g8,g8));inverse_pruned(q.ù(2*m),pad-1,Ķ,InverseBottom{Ķ.ô,q à},ŧ);inverse_pruned(p.ù(2*m),pad-1,Ķ,InverseBottom{Ķ.ô,p à},ŧ);next_level(q,ď/2,2*rows);}}projection_last_levels(t,Ķ,q,p,w,a);
#define ч Ė(m,m)
#define ш ú Ĉ*>
#define щ ø(ǉ nv ų)
#define ъ ö 2 Į
#define ы Ù PairWeights ş(*Ķ,k);ĭ
#define ь Ø swap
#define э middle_lanes
#define ю memory_
#define я Factors
#define ѐ prepare
#define ё inv
#define ђ ƫ=
#define ѓ į+
#define є short_len
#define ѕ F r(ˈ);r
#define і ;ŝ
#define ї ξ ǥ Ɓ
#define ј ʖ(Ƒ)
#define љ Ť Ö
#define њ ĺ(x>=Ƒ)x-=Ƒ
#define ћ Ķ.ô
#define ќ ĭ*Ù end=a+h;
#define ѝ ħ T>
#define ў ý ǁ=-1
#define џ ý(Ư)
#define Ѡ Ú ă;
#define ѡ top_store_
#define Ѣ PointSlots
#define ѣ pointwise_products
#define Ѥ mark=ʄ.mark()
#define ѥ tails
#define Ѧ subtrees
#define ѧ inverse_direct
#define Ѩ _mm256_store_si256
#define ѩ chunk
#define Ѫ prepare_sparse
#define ѫ F ă(ǁ)
#define Ѭ :Ú f.Ą ŕ()<
#define ѭ mul
#define Ѯ [&]
#define ѯ [t]
#define Ѱ [[gnu::flatten]]Ĉ
#define ѱ [2*ʛ+1]
#define Ѳ cube_quotients
#define ѳ ;ő(ʰ,h,0);ő(a ʥ h,h,1)
#define Ѵ ;ł
#define ѵ {x1=ƥ(x1,r0,q0);x2=ƥ(x2,r,rq);x3=ƥ(x3,r3,q3);}
#define Ѷ ĩ ȼ top=Ƶ top(ʛ.ũ,ʛ.Μ,î);Ƶ ʫ(c,top);Ƶ
#define ѷ fold
#define Ѹ q Û
#define ѹ r Ƈ Ł(u Ĕ,u[i+4],0x20)
#define Ѻ ϥ={Ō(a),Ō(a+8),Ō(a+16),Ō(a+24)};forward_h1
#define ѻ ƞ ȡ
#define Ѽ Ę F=Ȼ
#define ѽ Ć(ö 8 Į)
#define Ѿ â Stack&ǚ
#define ѿ Õ k Ē k<
#define Ҁ mod_mul
#define ҁ product_sum
#define Ҋ interpolate
#define ҋ BaseInterpolation
#define Ҍ cross_lanes
#define ҍ lane_store_
#define Ҏ projection_length
#define ҏ kParity
#define Ґ minus_twice
#define ґ montgomery_reduce
#define Ғ Convolution
#define ғ Transformed
#define Ҕ kMaxLog
#define ҕ width
#define Җ transform_inplace
#define җ combine
#define Ҙ _mm_cvtsi128_si32
#define ҙ y=ą(y,é(y,pp));
#define Қ if(a.ȭ)Ú b ĺ(b.ȭ)Ú a;ǌ ă;ă.ƌ=a.ƌ+b.ƌ
#define қ U*a
#define Ҝ =ŭ
#define ҝ ȟ m=ó(ķ Û)ĩ T ɯ=T(3).pow((T::mod()-1)/(2*m))
#define Ҟ Ǉ Ŝ(Ə,ō,Ǆ{Ə})ȡ nv=ǲ/8;ł*v=õ ĭ*>(į à)ĺ(Ī(nv)%2==0){Ć(ǉ nv ų)v Ɓ=src(j)Ū ő(į à,nv,1);}ƾ 2 ƣ z(Ə ƹ,Ə[9])ø(ǉ h ų){Ģ a=src(j),zb=ŭ(src(j+h),z);v Ɓ=ƪ(a,zb),v[j+h]=diff(a,zb);}ő(į à,h,2);ő(į ʥ h,h,3);
#define ҟ Ɯ Ù*
#define Ҡ Ɠ b
#define ҡ Ű scale_copy
#define Ң Ņ a
#define ң ý k
#define Ҥ Ù Ƽ&
#define ҥ Ù Ʋ&
#define Ҧ inverse_small
#define ҧ pruned
#define Ҩ combine_pairs
#define ҩ cyclic
#define Ҫ power_scratch
#define ҫ full=g6/8*8 Ч(Ў 1)ø(Ȫ full;c+=8,ȯ.next())
#define Ҭ fft ə
#define ҭ _mm256_permute4x64_epi64
#define Ү ==Ȓ
#define ү +16
#define Ұ %kP
#define ұ [h3]"r"(96*h)
#define Ҳ T::mod()
#define ҳ ĭ*
#define Ҵ ;ŗ
#define ҵ Ў 1
#define Ҷ ξ ǥ Ĕ
#define ҷ ː+i
#define Ҹ ʚ 4 ė v
#define ҹ ɚ ø
#define Һ ǆ ź
#define һ Ɠ a
#define Ҽ Ģ x=
#define ҽ ĝ a
#define Ҿ Ě ɪ
#define ҿ ę U*
#define Ӏ Ć(ý Ń)
#define Ӂ Ą<ý
#define ӂ ā fps_detail
#define Ӄ ý ď
#define ӄ æ Ö
#define Ӆ Õ t Ē t
#define ӆ projection_first_levels
#define Ӈ N::œ
#define ӈ typename
#define Ӊ cyclic_ns
#define ӊ continue
#define Ӌ v ǹ
#define ӌ +i*(n/4)
#define Ӎ .at0
#define ӎ *ß
#define ӏ ;Ø reverse_copy
#define Ӑ ;_mm256_maskstore_epi32
#define ӑ =ʗ
#define Ӓ ʗ(ƪ(ŭ(Ǚ(f0,f1),w.y),ŭ(Ǚ(f2,f3),w.z)))
#define ӓ [v]
#define Ӕ Ѓ(!kPlaces)keep_(lo,hi,Ğ(į,w*ɖ))Ū ʛ;
#define ӕ Ͻ 2
#define Ӗ ;j+=8
#define ӗ Ǉ(Ȱ,ɧ,InverseSquareBottom{Ȱ,ɧ,gt à})
#define Ә ǆ ntt
#define ә Ř(ǁ)
#define Ӛ ø(Ů k9 Ɲ)
#define ӛ Õ p Ē p<
#define Ӝ factorial_
#define ӝ gcd_detail
#define Ӟ direct_limit
#define ӟ need
#define Ӡ build_levels
#define ӡ recursion_
#define Ӣ product_leaf
#define ӣ graeffe_leaf
#define Ӥ negate
#define ӥ direct
#define Ӧ inverse_top2
#define ӧ asm volatile
#define Ө factorials
#define ө polynomial
#define Ӫ in Ĕ.v
#define ӫ a Ƈ
#define Ӭ [P2]"m"(k2P)
#define ӭ [h]"r"(32*h)
#define Ӯ [2*ʛ]
#define ӯ .insert
#define Ӱ .back()
#define ӱ .push_back
#define Ӳ .get()
#define ӳ cube_roots
#define Ӵ 2*k
#define ӵ ;ƴ Ú ǆ ƿ
#define Ӷ =ś(ǯ)
#define ӷ =Ō
#define Ӹ T(1)
#define ӹ ǭ Ĺ(þ==7)ĳ=ą(ĳ,é(ĳ,pp))Ѓ(!ù)ĳ=
#define Ӻ ø(ɵ=2 Ͼ ȸ Į)ё Ƈ Đ(Ͻ kP/Ö(i),ё[kP%i]);g
#define ӻ ;k8=ѭ(k8,in(2*İ));U d4=ƪ(k8,k8)ø(ö 2*İ Į)
#define Ӽ r ʖ(ļ ƫ Ĕ);e ђ ð(e ƫ,Þ(z,r));o ђ ð
#define ӽ free_=ю;build(1,lo,hi);descend(1,lo,hi,w);
#define Ӿ nv=ǲ/8 Ѵ*v=Ț(į à)
#define ӿ >(a,b,k)а
#define Ԁ ;ū(ѓ c,ń(é(ƪ(d,ɟ),Ž(ѓ ä+c)),kP));
#define ԁ Š(į ϝ,f ѯ);
#define Ԃ ĸ Ė
#define ԃ į Ʒ
#define Ԅ ğ Ù
#define ԅ ò Ö
#define Ԇ Õ nv
#define ԇ evaluate_direct
#define Ԉ xs Û
#define ԉ projection_work
#define Ԋ Carve
#define ԋ forward_columns
#define Ԍ below
#define ԍ cur
#define Ԏ *ď
#define ԏ <Ƅ
#define Ԑ Ć(Ů 8 Į)x Ƈ Ō(q Ʒ 16*(r+i));ʈ(x);
#define ԑ Ć(Õ r Ē r<e1;r+=8)
#define Ԓ ø(ý s Ē s<ɤ ė s)
#define ԓ fact
#define Ԕ if(ǰ<ȸ)Ƕ resize(ȸ)ø(Ů ȸ Į)(ˈ)Ĕ
#define ԕ n-1
#define Ԗ ;ĵ
#define ԗ Ć(ʍ=0,g=k*(nv/(4*h));j<nv;j+=4*h,++g)
#define Ԙ ;ķ Ř(2*m)ø(ö m Į)ķ[m+i]=ķ Ĕ*ɯ;
#define Ã "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc"
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ӂ{ƛ NTT{Ę T=Ě;typedef ϔ U __attribute__((may_alias))ͼ V=ħ ϔ>;ċ æ U ο=998244353,Ƒ=2*ο,one=301989884;V ķ,ô,twists,ϻ;V Ƣ,ſ;mutable V ӳ,ώ,Ѳ,ˇ;Ĉ prepare_cubes(ǧ)Ù{ý begin=ӳ Û ĺ(begin>=n)Ú;ӳ Ř(n);ώ Ř(n);Ѳ Ř(n);ˇ Ř(n)ø(ң=begin;k<n ȝ){U x=(о)ķ ƫ*ķ[Ӵ]%ο;U y=(о)ô ƫ*ô[Ӵ]%ο;ӳ ђ x;ώ ђ y;Ѳ ђ ť(x);ˇ ђ ť(y);}}ċ U ń(о x){Ú(x+(о)(U(x)*998244351u)*ο)>>32;}ċ U ѭ(U a,U b){Ú ń((о)a*b);}ċ U ƪ(U a,U b){U c=a+b Ū c<Ƒ?c:c-Ƒ;}ċ U sub(U a,U b){U c=a-b Ū c<Ƒ?c:c+Ƒ;}ċ U in(U a){Ú ѭ(a,932051910);}ċ U į(U a){a=ń(a)Ū a<ο?a:a-ο;}ċ U ť(U r){Ú((о)r<<32)/ο;}ċ U ɲ(U a,U r,U q){Ú a*r-U(((о)a*q)>>32)*ο;}NTT(ý ä,ƍ c8=Ɔ):ķ(ä,one),ô(ä,one),twists(c8?ä:0,one),ϻ(c8?ä:0,one),Ƣ(ä),ſ(ä){Ć(ý h=1;h<ä;h<<=1){U r=in(T(3).pow((ο-1)/(4*h))ȃ);U ir=in(T(3).pow(ο-1-(ο-1)/(4*h))ȃ);U t=in(T(3).pow((ο-1)/(2*h))ȃ);U it=in(T(3).pow(ο-1-(ο-1)/(2*h))ȃ);U a=in(ο-(ο-1)/h),b=a ø(ö h Į){ķ[h+i]=ѭ(ķ Ĕ,r);ô[h+i]=ѭ(ô Ĕ,ir)ĺ(c8)twists[h+i]=a,ϻ[h+i]=b;a=ѭ(a,t);b=ѭ(b,it);}}Ć(ö ä Į){Ƣ Ƈ(ķ Ĕ<ο?ķ Ĕ:ķ Ĕ-ο)*998244351u;ſ Ƈ(ô Ĕ<ο?ô Ĕ:ô Ĕ-ο)*998244351u;ķ Ƈ į(ķ Ĕ);ô Ƈ
 į(ô Ĕ)ĺ(c8)twists Ƈ į(twists Ĕ),ϻ Ƈ į(ϻ Ĕ);}}
#if defined(__x86_64__) || defined(__i386__)
ċ ƍ Ĝ{
#if defined(__AVX2__)
Ú Ɔ;
#else
ċ Ù ƍ ɱ=__builtin_cpu_supports("avx2")Ū ɱ;
#endif
}Ę I8=ϸ;Ü ċ I8 œ(Ɯ a){Ú ɂ(õ Ù I8*>(a));}ˌ ŗ(қ,I8 x){Ψ(õ I8*>(a),x);}Ü ċ I8 ɏ(I8 a,I8 b){ͳ Þ(a,b)В y=І В r ʖ(998244351),p ʖ(ο);x=ð(x,Þ(Þ(x,r),p));y=ð(y,Þ(Þ(y,r),p))Ū Ĵ(ʤ,y,0xaa);}Ü ċ I8 ƥ(I8 a,I8 r,I8 q){ͳ ñ(Þ(a,q),32)В y=Þ(ñ(a,32),q)В t=Ĵ(x,y,0xaa)Ū é(ό(a,r),ό(t,å(ο)));}Ü ċ I8 fixed_even8(I8 a,I8 r,I8 q){I8 t=ñ(Þ(a,q),32)Ū _mm256_sub_epi64(Þ(a,r),Þ(t,å(ο)));}Ü ċ I8 ɾ(I8 a,I8 b){ͳ ň(a,b)Ū ą(x,é(x,å(Ƒ)));}Ü ċ I8 sub8(I8 a,I8 b){ͳ é(a,b)Ū ą(x,ň(x,å(Ƒ)));}Ӂ ď>Ü ċ I8 rates8(Ù V&Ƴ,ң){Ĺ(ď у)Ú Њ(_mm_loadu_si128(õ Ù __m128i*>(Ƴ Ʒ 4*k)))Ѓ(ď==2)Ú ҭ(Њ(_mm_loadl_epi64(õ Ù __m128i*>(Ƴ Ʒ Ӵ))),0x50)Ѓ(ď==4)Ú å(Ƴ ƫ);}Ӂ ď,ƍ ć>Ü I8 small8(I8 z,ң)Ù{Ĺ(ď==4){if(k ζ){ͳ Ł(z,z,0x00),y=Ł(z,z,0x11)Ū Ĵ(ɾ(x,y),sub8(x,y),0xf0);}}ɺ quot=ć?ſ:Ƣ В r=rates8<ď>(w,k),q=rates8<ď>(quot,k)Ѓ(ć){Ĺ(ď==2)z=Ɛ(z,0xd8)Ѓ(ď==4)z=Ľ(z,ŏ(0,4,1,5,2,6,3,7))В y=ñ(z,32),x=ɾ(z,y)В d=fixed_even8(sub8(z,y),r,q);z=Ĵ(x,_mm256_slli_epi64(d,32),0xaa);}ŝ{Ĺ(ď у)z=Ɛ(z,0xb1)Ѓ(ď==2)z=Ɛ(z,0x72)Ѓ(ď==4)z=Ľ(z,ŏ(4,0,5,1,6,2,7,3))В y=fixed_even8(z,r,q);ͳ Ĵ(ñ(
z,32),z,0xaa);y=Ĵ(y,é(å(Ƒ),_mm256_slli_epi64(y,32)),0xaa);z=ɾ(x,y);}Ĺ(ď==2)z=Ɛ(z,0xd8)Ѓ(ď==4)z=Ľ(z,ŏ(0,2,4,6,1,3,5,7))Ū z;}ή ć>Ü Ĉ fft8(қ,ί,ý i2=0)Ù{if(!i2)i2=î Ѓ(!ć){ý h=i2/2 ø(;h>=16;h>>=2)radix4_8 ə(a,h/2,î)ĺ(h==8){if(i2>=32)Ć(ö î Ɲ){ͳ Д,p ј Ҵ(a+i,Ǧ);}stage8 ə(a,h,î);}}Ć(ȓ î Ӗ){I8 z=œ(a+j)Ѓ(!ć)if((i2>=32)&&(__builtin_ctz((ϔ)i2)&1)){I8 p ј;z=ą(z,é(z,p));}Ĺ(ć)z=Σ 4,Ƅ(Σ 2,Ƅ(Σ 1,Ƅ(z,j/8),j/8),j/8)і z=Σ 1,Ś>(Σ 2,Ś>(Σ 4,Ś>(z,j/8),j/8),j/8)Ҵ(a+j,z);}Ĺ(ć){ý h=8 ø(;4*h<=i2;h<<=2)radix4_8 ԏ(a,h,î)ĺ(h<i2)stage8 ԏ(a,h,î);}}ή ć>Ü Ĉ radix4_8(қ,Ӄ,ί)Ù{ɺ q=ć?ſ:Ƣ;prepare_cubes(î/(4 Ԏ))ĩ V&w3=ć?ώ:ӳ ĩ V&q3s=ć?ˇ:Ѳ В p ј В im ʖ(w ƹ),iq ʖ(q ƹ)ø(ý j=0,k Ē j<î;j+=4 Ԏ,++k){I8 r ʖ(w ƫ),rq ʖ(q ƫ)В r0 ʖ(w[Ӵ]),q0 ʖ(q[Ӵ])В r3 ʖ(w3 ƫ),q3 ʖ(q3s ƫ)ø(ö ď Ɲ){I8 x0=œ(a+j+i),x1=œ(a+j+ď+i)В x2=œ(a+j ʳ ď+i),x3=œ(a+j+3 Ԏ+i)Ѓ(ć){I8 u=ɾ(x0,x1),v=sub8(x0,x1)В s=ɾ(x2,x3)В t=ƥ(é(ň(x2,p),x3),im,iq);x0=ɾ(u,s);x1=ň(v,t);x2=é(ň(u,p),s);x3=é(ň(v,p),t)ĺ(k)ѵ ŝ Ћ}ŝ{x0=ą(x0,é(x0,p))ĺ(!k)Ћ if(k)ѵ I8 u=ɾ(x0,x2),v=sub8(x0,x2),s=ɾ(x1,x3)В t
=ƥ(é(ň(x1,p),x3),im,iq);x0=ň(u,s);x1=é(ň(u,p),s);x2=ň(v,t);x3=é(ň(v,p),t);}ρ}}}ή ć>Ü Ĉ stage8(қ,Ӄ,ί,ý ù=0)Ù{ɺ quot=ć?ſ:Ƣ ĺ(ď<8){Ć(ȓ î Ӗ){I8 z=œ(a+j)ĺ(ď у)z=Σ 1,ć>(z,j/8);м(ď==2)z=Σ 2,ć>(z,j/8)і z=Σ 4,ć>(z,j/8)Ҵ(a+j,z);}}ŝ{Ć(ý j=0,k=ù;j<î;j+=2 Ԏ,++k){I8 r ʖ(w ƫ),q ʖ(quot ƫ)ø(ö ď Ɲ){ͳ œ(a+j+i),y=œ(a+j+ď+i)Ѓ(!ć)if(k)y=ƥ(y,r,q)В z=sub8(x,y);x=ɾ(x,y)Ѓ(ć)if(k)z=ƥ(z,r,q)Ҵ(a+j+i,x)Ҵ(a+j+ď+i,z);}}}}ή ć,ƍ lazy=Ś>Ȩ Ŵ fixed_butterfly8(I8&x,I8&y,I8 r,I8 q,ƍ unit=Ś){Ĺ(lazy&&!ć){I8 p ј;x=Ǧ;y=unit?έ:ƥ(y,r,q)В z=é(ň(x,p),y);x=ň(x,y);y=z;}ŝ{Ĺ(!ć)if(!unit)y=ƥ(y,r,q)В z=sub8(x,y);x=ɾ(x,y);y=ć&&!unit?ƥ(z,r,q):z;}}ή ć,ƍ lazy=Ś>Ü Ĉ Ϝ(қ,Ӄ,ί,ý ù=0)Ù{ɺ q=ć?ſ:Ƣ ø(ý j=0,k=ù;j<î;j+=4 Ԏ,++k){I8 r ʖ(w ƫ),rq ʖ(q ƫ)В r0 ʖ(w[Ӵ]),q0 ʖ(q[Ӵ])В r1 ʖ(w[Ӵ+1]),q1 ʖ(q[Ӵ+1])ø(ö ď Ɲ){I8 x0=œ(a+j+i),x1=œ(a+j+ď+i)В x2=œ(a+j ʳ ď+i),x3=œ(a+j+3 Ԏ+i)Ѓ(ć){Ȃ,lazy>(x0,x1,r0,q0,k ζ);Ȃ,lazy>(x2,x3,r1,q1,Ś);Ȃ,lazy>(x0,x2,r,rq,k ζ);Ȃ,lazy>(x1,x3,r,rq,k ζ);}ŝ{Ȃ,lazy>(x0,x2,r,rq,k ζ);Ȃ,lazy>(x1,x3,r,rq,k ζ);Ȃ,lazy>(x0,x1,r0,q0,k ζ);Ȃ,lazy>(x2,
x3,r1,q1,Ś);}ρ}}}ˌ load_pairs(Ɯ a,I8&x,I8&y){I8 ȯ=ŏ(0,2,4,6,1,3,5,7)В u=Ľ(œ(a),ȯ),v=Ľ(œ(a+8),ȯ);x=Ł(u,v,0x20);y=Ł(u,v,0x31);}ˌ store_pairs(қ,I8 x,I8 y){I8 ȯ=ŏ(0,4,1,5,2,6,3,7)Ҵ(a,Ľ(Ł(x,y,0x20),ȯ))Ҵ(a+8,Ľ(Ł(x,y,0x31),ȯ));}ˌ scale_copy8(U*ː,Ɯ src,ί,U r){U l1=ť(r)В v ʖ(r),q ʖ(l1);ȥ(ʲ î Ɲ)ŗ(ҷ,ƥ(œ(src+i),v,q))ø(Ͼ î Į)ː Ƈ ɲ(src Ĕ,r,l1);}ˌ scale_normalize8(қ,ǧ,U r){U qr=ť(r)В v ʖ(r),q ʖ(qr),p ʖ(ο);К{ͳ ƥ(Д,v,q)Ҵ(a+i,Ǧ);}Ć(ț){U x=ɲ(ɨ,r,qr);ӫ x<ο?x:x-ο;}}ˌ square_fold8(қ,Ɯ q,ǧ,ƍ j1){Ć(ö n Ɲ){ͳ Д,y=œ(a+n+i)В u=ɏ(œ(q+i),œ(q+i))В v=ɏ(œ(q+n+i),œ(q+n+i));x=ɾ(x,x)Ҵ(a+i,j1?sub8(x,u):ɾ(x,u))Ҵ(a+n+i,ɾ(ɾ(y,y),v));}}ˌ scale8(қ,ί,U r){I8 v ʖ(r),q ʖ(ť(r));ȥ(ʲ î Ɲ)ŗ(a+i,ƥ(Д,v,q))ø(Ͼ î Į)ӫ ɲ(ɨ,r,ť(r));}ˌ pair_product8(қ,Ɯ b,ί){I8 ˠ,i8 ʖ(ο),p ј В ȯ=ŏ(0,2,4,6,1,3,5,7)ø(ö î Ɲ){ͳ œ(b ʳ i),y=œ(b ʳ i+8);x=Ǧ;y=έ;x=Þ(x,ʤ);y=Þ(y,ñ(y,32));x=ð(x,Þ(Þ(x,ni),i8));y=ð(y,Þ(Þ(y,ni),i8));x=Ĵ(ʤ,y,0xaa)Ҵ(a+i,Ľ(x,ȯ));}}ˌ pair_multiply8(қ,Ɯ b,ί){I8 f9=ŏ(0,0,1,1,2,2,3,3),g3=ŏ(4,4,5,5,6,6,7,7)В p ј ø(ö î Ɲ){I8 r=œ(b+i),x=œ(a ʳ i),y=œ
(a ʳ i+8);x=Ǧ;y=έ;x=Ɛ(x,0xb1);y=Ɛ(y,0xb1)Ҵ(a ʳ i,ɏ(x,Ľ(r,f9)))Ҵ(a ʳ i+8,ɏ(y,Ľ(r,g3)));}}ˌ difference8(қ,Ɯ b,Ɯ c,ί){Ć(ö î Ɲ)ŗ(a+i,sub8(œ(b+i),œ(c+i)));}
#endif
ή ć>Ĉ stage(қ,Ӄ,ί,ý ù=0)Ù{ɺ quot=ć?ſ:Ƣ;
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){stage8<ć>(a,ď,î,ù)а}
#endif
Ć(ý j=0,k=ù;j<î;j+=2 Ԏ,++k){U r=w ƫ,q=quot ƫ;ȥ(Ͼ ď Į){U x=a[j+i],y=a[j+ď+i]Ѓ(!ć)if(k)y=ɲ(y,r,q);a[j+i]=ƪ(x,y);a[j+ď+i]=ć&&k?ɲ(sub(x,y),r,q):sub(x,y);}}}ή ć>Ĉ ϋ(қ,Ӄ,ί,ý ù=0)Ù{
#if defined(__x86_64__) || defined(__i386__)
if(ď ɩ){Ϝ<ć>(a,ď,î,ù)а}
#endif
Ĺ(ć)stage ԏ(a,ď,î,2*ù),stage ԏ(a,2 Ԏ,î,ù)і stage ə(a,2 Ԏ,î,ù),stage ə(a,ď,î,2*ù);}ή ć>Ĉ fft(қ,ί,ý i2=0)Ù{if(!i2)i2=î;
#if defined(__x86_64__) || defined(__i386__)
if(i2 ɩ){fft8<ć>(a,î,i2)а}
#endif
Ĺ(ć){Ć(ý h=1;h<i2;h<<=1)stage ԏ(a,h,î);}ŝ{Ć(ý h=i2/2;h;h>>=1)stage ə(a,h,î);}}
#if defined(__x86_64__) || defined(__i386__)
ˌ product8(U*ː,Ɯ a,Ɯ b,ǧ){К{ͳ ɏ(Д,œ(b+i))Ҵ(ҷ,x);}Ć(ț)ː Ƈ ѭ(ɨ,Ϲ);}ˌ weighted8(U*ː,Ɯ src,ǧ,ý ù){Ť U e2 ҹ(ȓ 8 ų)e2 ˉ in(Ў j)В r=œ(e2),ɯ ʖ(in(8));К{ŗ(ҷ,ɏ(œ(src+i),r));r=ɾ(r,ɯ);}Ć(ț)ː Ƈ ѭ(src Ĕ,in(Ў i));}Ȩ ċ I8 inverse8(I8 a){I8 a2=ɏ(a,a),a3=ɏ(a2,a)В a7=ɏ(ɏ(a3,a3),a),a31=a7 ø(ý ɉ Ē ɉ<2 ė ɉ)a31=ɏ(a31,a31);a31=ɏ(a31,a3)В r=a7 ø(ý ɉ Ē ɉ<3 ė ɉ)r=ɏ(r,r);r=ɏ(r,a3);r=ɏ(r,r)ø(ȓ 4 ų){Ć(ý ɉ Ē ɉ<5 ė ɉ)r=ɏ(r,r);r=ɏ(r,a31);}Ć(ý ɉ Ē ɉ<3 ė ɉ)r=ɏ(r,r)Ū ɏ(r,a7);}ˌ batch_inverse8(U*ː,Ɯ src,ǧ){if(n<16){Ć(Е)ː Ƈ T ɪ(src Ĕ)ˆ.v а}ý þ=n/16*16 В p=œ(src),q=œ(src+8)Ҵ(ː,p)Ҵ(ː+8,q)ø(Ż 16 Ͼ þ ƽ 16){p=ɏ(p,œ(src+i));q=ɏ(q,œ(src+i+8))Ҵ(ҷ,p)Ҵ(ҷ+8,q);}Ť U т[16]Ҵ(т,p)Ҵ(т+8,q)ø([[maybe_unused]]ł x:т)ơ(į(x)!=0)В ć=inverse8(p),j5=inverse8(q)ø(Ż þ-16;i>=16;i-=16){ŗ(ҷ,ɏ(ć,œ(ҷ-16)))Ҵ(ҷ+8,ɏ(j5,œ(ҷ-8)));ć=ɏ(ć,œ(src+i));j5=ɏ(j5,œ(src+i+8));}ŗ(ː,ć)Ҵ(ː+8,j5)ø(Ż þ ț)ː Ƈ T ɪ(src Ĕ)ˆ.v;}ˌ evaluate_leaf8(U*ː,Ɯ ԍ,Ɯ Ǌ,ý ũ,Ɯ k5,ý þ){ˡ,pp ј;ͳ œ(ԍ);x=ʣ;x=Ǧ;Ť U Ļ[16]={},ļ ɚ={},ө ɚ,Η ɚ={}Ҵ(Ļ+8,x)Ԗ(Ǌ,Ħ(ũ,8),ļ)В y=œ(ļ);ҙ ŗ(ļ,έ)В e=đ,o ǋ(x,é(x,pp))Ҵ
(ө,Ǧ)ĺ(þ==8)x=œ(k5)і{ĵ(k5,þ,Η);x=œ(Η);}I8 ĳ=đ ø(ö 8 Į)ĳ=ɾ(ɏ(ĳ,x),å(ө Ĕ))ĺ(þ==8)ŗ(ː,ĳ)і{ŗ(Η,ĳ)Ԗ(Η,þ,ː);}}Ȩ ċ U sum_product8(I8 a,I8 b){ˡ;a=ą(a,é(a,p));b=ą(b,é(b,p))В ʂ=ð(Þ(a,b),І);ʂ=ð(ʂ,Ł(ʂ,ʂ,1));ʂ=ð(ʂ,Ɛ(ʂ,0x4e));U ă=ń((о)_mm_cvtsi128_si64(ɦ(ʂ)))Ū ă<Ƒ?ă:ă-Ƒ;}ˌ interpolate_leaf8(U*ː,Ɯ q,Ɯ k5,Ɯ ş){ͳ œ(k5),w=œ(ş),c ʖ(one)В ʂ=ɾ(w,Ł(w,w,1));ʂ=ɾ(ʂ,Ɛ(ʂ,0x4e));ʂ=ɾ(ʂ,Ɛ(ʂ,0xb1));ː[7]=Ҙ(ɦ(ʂ))ø(ң=7-1;k и k){c=ɾ(ɏ(c,x),å(q[k+1]));ː ђ sum_product8(c,w);}}ˌ evaluate_middle8(U*ϊ,U*l8,Ɯ ԍ,Ɯ ql,ý dl,Ɯ qr,ý dr){ˡ,pp ј,ˠ;Ť U Ļ[16],ļ ǥ[9]={}ø(ö 16 Ɲ){ͳ œ(ԍ+i);x=ʣ Ҵ(Ļ+i,Ǧ);}Ć(ö Ħ(dl,9)Į){U x=ql Ĕ њ ĺ(x>=ο)x-=ο;ļ Ŭ Ƈ x;}Ć(ö Ħ(dr,9)Į){U x=qr Ĕ њ ĺ(x>=ο)x-=ο;ļ ƹ Ƈ x;}I8 e ǥ={},o ǥ={}ø(ö 9 Į){I8 z=œ(Ļ+8-i),high=ñ(z,32)ø(ˋ 2 ȝ){I8 Ӽ(o ƫ,Þ(high,r));}}U*ː ǥ={ϊ,l8}ø(ˋ 2 ȝ){e ђ ð(e ƫ,Þ(Þ(e ƫ,ni),p));o ђ ð(o ƫ,Þ(Þ(o ƫ,ni),p));ͳ Ĵ(ñ(e ƫ,32),o ƫ,0xaa)Ҵ(ː ƫ,ʣ);}}ˌ inverse_sequence8(U*ː,ý ù,ǧ){Ť U g4 ҹ(ȓ 8 ų)g4 ˉ in(Ў j)В Ÿ=œ(g4),ɯ ʖ(in(8))В p=Ÿ Ҵ(ː,p)ø(Ż 8 Ͼ n Ɲ){Ÿ=ɾ(Ÿ,ɯ);p=ɏ(p,Ÿ)Ҵ(ҷ,p);}I8 ć ʖ(one)ø(U e=ο-2;e;e>>=1,p=ɏ(
p,p))if(e&1)ć=ɏ(ć,p)ø(Ż n-8;i>=8;i-=8){ŗ(ҷ,ɏ(ć,œ(ҷ-8)));ć=ɏ(ć,Ÿ);Ÿ=sub8(Ÿ,ɯ);}ŗ(ː,ć);}ˌ normalize8(қ,ǧ){ˡ;К{ͳ Д Ҵ(a+i,Ǧ);}Ć(ț)if(ɨ>=ο)ɨ-=ο;}
#endif
Ŵ weighted(U*ː,Ɯ src,ǧ,ý ù){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{weighted8(ː,src,n,ù)а}
#endif
Ć(Е)ː Ƈ ѭ(src Ĕ,in(Ў i));}Ŵ scale_normalize(қ,ǧ,U r){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{scale_normalize8(a,n,r)а}
#endif
U qr=ť(r);ȥ(ț){U x=ɲ(ɨ,r,qr);ӫ x<ο?x:x-ο;}}Ŵ square_fold(қ,Ɯ q,ǧ,ƍ j1){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{square_fold8(a,q,n,j1)а}
#endif
ȥ(ț){U u=ѭ(q Ĕ,q Ĕ),v=ѭ(q[n+i],q[n+i]);U x=ƪ(ɨ,ɨ);ӫ j1?sub(x,u):ƪ(x,u);a[n+i]=ƪ(ƪ(a[n+i],a[n+i]),v);}}Ŵ normalize(қ,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{normalize8(a,n)а}
#endif
ȥ(ț)if(ɨ>=ο)ɨ-=ο;}Ŵ Ŏ(U*ː,Ɯ a,Ɯ b,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{product8(ː,a,b,n)а}
#endif
ȥ(ț)ː Ƈ ѭ(ɨ,Ϲ);}
#if defined(__x86_64__) || defined(__i386__)
ή ć>Ȩ Ĉ ω(қ,Ӄ,ң)Ù{ɺ q=ć?ſ:Ƣ В r ʖ(w ƫ),rq ʖ(q ƫ)ø(ö ď Ɲ){ͳ Д,y=œ(a+ď+i);Ȃ,Ƅ(x,y,r,rq,k ζ)Ҵ(a+i,x)Ҵ(a+ď+i,y);}}Ȩ Ĉ leaf8(қ,Ɯ b,ң)Ù{U w=ķ[k/2],q=Ƣ[k/2]ĺ(k&1)w=ο-w,q=~q;ˡ,pp ј;ͳ œ(a),y=œ(b);x=ʣ;ҙ x=Ǧ;y=έ;Ť U Ļ[16],ļ ɚ В ʗ=ƥ(x,å(w),å(q))Ҵ(Ļ,ą(ʗ,é(ʗ,p)))Ҵ(Ļ+8,x)Ҵ(ļ,y)В e=đ,o ǋ(x,é(x,pp))Ҵ(a,Ǧ);}Ӂ þ,ƍ ψ=Ś>Ȩ Ĉ inverse_leaf_products8(қ,U*b,Ɯ ws,Ɯ qs)Ù{ˡ,pp ј;Ť U Ļ[þ][16],ļ[þ]ɚ В e[þ],o[þ]ø(ˋ þ ȝ){ͳ œ(ύ k),y=œ(b Ʀ k);x=ʣ Ѓ(!ψ)ҙ x=Ǧ Ѓ(!ψ)y=έ В ʗ=ƥ(x,å(ws ƫ),å(qs ƫ))Ҵ(Ļ ƫ,ą(ʗ,é(ʗ,p)))Ҵ(Ļ ƫ+8,x)Ҵ(ļ ƫ,y)Ѓ(!ψ)ŗ(b Ʀ k,y);e ђ o ђ đ;}ѽ Ć(ˋ þ ȝ){I8 z=œ(Ļ ƫ+8-i),Ӽ(o ƫ,Þ(ñ(z,32),r));}I8 ˠ ø(ˋ þ ȝ){ͳ e ƫ,y=o ƫ;x=ð(x,Þ(Þ(x,ni),p));y=ð(y,Þ(Þ(y,ni),p));x=Ĵ(ʤ,y,0xaa)Ҵ(ύ k,ʣ);}}Ü ċ U reverse_dot8(Ɯ a,Ɯ b,ǧ){ˡ,pp ј,ˠ В reversed=ʞ,o4=đ;ȥ(ʲ n;){I8 Ŧ=đ,Ȏ=Ŧ;ý end Ш(n/8*8,i+64)ø(Ͼ end Ɲ){ͳ Д,y=Ľ(œ(b-i-7),reversed);x=ʣ;ҙ x=Ǧ;y=έ ɳ}ǭ o4=ɾ(o4,ĳ);}o4=ɾ(o4,Ł(o4,o4,1));o4=ɾ(o4,Ɛ(o4,0x4e));o4=ɾ(o4,Ɛ(o4,0xb1));U ă=Ҙ(ɦ(o4))ø(ț){U x=ɨ,y=b[-i]њ ĺ(y>=Ƒ)y-=Ƒ ĺ(x>=ο)x-=ο ĺ(y>=ο)y-=ο;ă=ƪ(ă,ѭ(x,y));}Ѡ}ή ć>Ü Ĉ Ρ(қ,ǧ,ý i2=0)Ù{if(!i2)i2
=n Ѓ(!ć){Ӄ=i2/2 ø(;ď>=16;ď>>=2)radix4_8 ə(a,ď/2,n)ĺ(ď==8)Ć(ö n ƽ 16)ω ə(a+i,8,i/16);}ŝ{Ӄ=8 ø(;4 Ԏ<=n;ď<<=2)radix4_8 ԏ(a,ď,n)ĺ(ď<n)stage8 ԏ(a,ď,n);}}Ü Ĉ inverse_newton8(қ,U*b,ǧ)Ù{ơ(n>=64&&!(n&(ԕ))&&n ɰ&&ó(ķ Û)>=n/16)Ԗ(b,n/2,b+n/2);Ρ ə(b,n,n/2);Ρ ə(a,n)ø(ö n ƽ 64){U ws ɚ,Ɂ>(a+i,b+i,ws,qs);}Ρ ԏ(a,n);ʇ(a,n/2,U(0));Ρ ə(a,n)ø(ö n ƽ 64){U ws ɚ,Ɂ,Ƅ(a+i,b+i,ws,qs);}Ρ ԏ(a,n);U norm=ο-(ο-1)/(n/8);scale8(a+n/2,n/2,ο-(о)norm*norm%ο);}Ą<ǧ>Ȩ Ĉ ʕ(қ,U*b,ң)Ù{Ĺ(n==8)leaf8(a,b,k)і Ĺ(n==16){ω ə(a,n/2,k);ω ə(b,n/2,k);ʕ<n/2>(a,b,Ӵ);ʕ<n/2>(a+n/2,b+n/2,Ӵ+1);ω ԏ(a,n/2,k);}ŝ{Ϝ<Ś,ϣ Ϝ<Ś,Ƅ(b,n/4,n,k)ɫ ʕ<n/4>(a ӌ,b ӌ,4*k+i);Ϝ<ϣ}}Ü Ĉ convolution8(қ,U*b,ǧ,ң=0)Ù{if(n==256){ʕ<256 ӿ}if(n==128){ʕ<128 ӿ}if(n==64){ʕ<64 ӿ}Ϝ<Ś,ϣ Ϝ<Ś,Ƅ(b,n/4,n,k)ɫ convolution8(a ӌ,b ӌ,n/4,4*k+i);Ϝ<ϣ}
#endif
#if defined(__x86_64__) || defined(__i386__)
ή Ȏ>ˌ reduce_pairs8(қ,U*b,Ɯ twist,ǧ){Ć(ö n Ɲ){I8 x,y,u,v;load_pairs(a ʳ i,x,y);load_pairs(b ʳ i,u,v);x=ɏ(x,v);y=ɏ(y,u)Ѓ(Ȏ)x=ɏ(sub8(x,y),œ(twist+i))і x=ɾ(x,y)Ҵ(a+i,x)Ҵ(b+i,ɏ(u,v));}}ˌ combine8(қ,Ɯ b,Ɯ x,Ɯ y,ǧ){Ć(ö n Ɲ)ŗ(a+i,ɾ(ɏ(Д,œ(y+i)),ɏ(œ(b+i),œ(x+i))));}
#endif
ή Ȏ>Ŵ reduce_pairs(қ,U*b,Ɯ twist,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{reduce_pairs8<Ȏ>(a,b,twist,n)а}
#endif
ȥ(ț){U u=ѭ(a ɔ,b ư),v=ѭ(a ư,b ɔ);ӫ Ȏ?ѭ(sub(u,v),twist Ĕ):ƪ(u,v);b Ƈ ѭ(b ɔ,b ư);}}Ŵ җ(қ,Ɯ b,Ɯ x,Ɯ y,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{combine8(a,b,x,y,n)а}
#endif
ȥ(ț)ӫ ƪ(ѭ(ɨ,y Ĕ),ѭ(Ϲ,x Ĕ));}
#if defined(__x86_64__) || defined(__i386__)
ˌ geometric8(қ,ǧ,U ù,U ɯ){Ť U e2 ɚ;U Ǹ=one ø(ö 8 Į){e2 Ƈ ѭ(ù,Ǹ);Ǹ=ѭ(Ǹ,ɯ);}I8 r=œ(e2),l2 ʖ(Ǹ);К{ŗ(a+i,ɏ(Д,r));r=ɏ(r,l2);}ŗ(e2,r)ø(ý j Ē i<n Į,++j)ӫ ѭ(ɨ,e2 Ɓ);}
#endif
Ŵ geometric(қ,ǧ,U ù,U ɯ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{geometric8(a,n,ù,ɯ)а}
#endif
ȥ(ț){ӫ ѭ(ɨ,ù);ù=ѭ(ù,ɯ);}}
#if defined(__x86_64__) || defined(__i386__)
ˌ add_scaled8(қ,Ɯ b,ǧ,U r){U q=ť(r)В v ʖ(r),vq ʖ(q);К ŗ(a+i,ɾ(Д,ƥ(œ(b+i),v,vq)))ø(ț)ӫ ƪ(ɨ,ɲ(Ϲ,r,q));}
#endif
Ŵ add_scaled(қ,Ɯ b,ǧ,U r){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{add_scaled8(a,b,n,r)а}
#endif
ý i Ē U q=ť(r)ø(ț)ӫ ƪ(ɨ,ɲ(Ϲ,r,q));}Ĉ leaf(қ,Ɯ b,ң)Ù{U w=ķ[k/2],q=Ƣ[k/2]ĺ(k&1)w=ο-w,q=~q;U Ļ[16],ļ ҹ(ö 8 Į){U x=ɨ<ο?ɨ:ɨ-ο;U z=ɲ(x,w,q);Ļ Ƈ z<ο?z:z-ο;Ļ[8+i]=x;ļ Ƈ Ϲ<ο?Ϲ:Ϲ-ο;}Ć(ȓ 8 ų){о ʂ ƒ(ö 8 Į)ʂ+=(о)Ļ[8-i+j]*ļ Ĕ;U x=ń(ʂ)њ;a ˉ x<ο?x:x-ο;}}Ą<ǧ>Ĉ ʯ(қ,U*b,ң)Ù{Ĺ(n==8)leaf(a,b,k)і Ĺ(n==16){stage ə(a,n/2,n,k);stage ə(b,n/2,n,k);ʯ<n/2>(a,b,Ӵ);ʯ<n/2>(a+n/2,b+n/2,Ӵ+1);stage ԏ(a,n/2,n,k);}ŝ{ϋ ə(a,n/4,n,k);ϋ ə(b,n/4,n,k)ɫ ʯ<n/4>(a ӌ,b ӌ,4*k+i);ϋ<ϣ}}Ĉ convolution(қ,U*b,ǧ,ң=0)Ù{
#if defined(__x86_64__) || defined(__i386__)
if(Ĝ){convolution8(a,b,n,k)а}
#endif
if(n==256){ʯ<256 ӿ}if(n==128){ʯ<128 ӿ}if(n==64){ʯ<64 ӿ}ϋ ə(a,n/4,n,k);ϋ ə(b,n/4,n,k)ɫ convolution(a ӌ,b ӌ,n/4,4*k+i);ϋ<ϣ}
#if defined(__x86_64__) || defined(__i386__)
ˌ accumulate8(U*ː,Ɯ a,Ɯ b,Ɯ c,ǧ,ƍ ʁ){Ć(ö n Ɲ){ͳ œ(b+i),y=œ(c+i);x=ʁ?é(ň(x,å(Ƒ)),y):ň(x,y)Ҵ(ҷ,ɾ(œ(ҷ),ɏ(Д,x)));}}
#endif
#if defined(__x86_64__) || defined(__i386__)
Ӂ þ>ˌ accumulate_batch8(U*ː,ҟ a,ҟ b,ҟ c,ǧ,ƍ ʁ){Ǝ(þ>=1&&þ<=3);ˡ,pp ј,ˠ;ý Ų=ʁ?n:0 ø(ö n Ɲ){I8 Ŧ=đ,Ȏ=Ŧ;
#pragma GCC unroll 3
Ć(ˋ þ ȝ){ͳ œ(b ƫ+Ų+i),y=œ(c ƫ+Ų+i);x=ʁ?é(ň(x,pp),y):ň(x,y)ϟ}ǭ ŗ(ҷ,ɾ(œ(ҷ),ĳ));}}
#endif
#if defined(__x86_64__) || defined(__i386__)
Ӂ þ,ƍ ù,ƍ ů>ˌ accumulate_wide8(U*ː,ҟ a,ҟ b,ҟ c,ǧ,ƍ ʁ){Ǝ(þ>=1&&þ<=7);ˡ,pp ј,ˠ;ý Ų=ʁ?n:0 ø(ö n Ɲ){I8 Ŧ=đ,Ȏ=Ŧ;
#pragma GCC unroll 7
Ć(ˋ þ ȝ){ͳ œ(b ƫ+Ų+i),y=œ(c ƫ+Ų+i);x=ʁ?é(ň(x,ů?p:pp),y):ň(x,y)Ѓ(!ů)x=ʣ ϟ}ӹ ɾ(œ(ҷ),ĳ)Ҵ(ҷ,ĳ);}}
#endif
Ӂ þ,ƍ ù,ƍ ů>Ŵ accumulate_wide(U*ː,ҟ a,ҟ b,ҟ c,ǧ,ƍ ʁ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{accumulate_wide8<þ,ù,ů>(ː,a,b,c,n,ʁ)а}
#endif
Ĺ(ù)ʇ(ː,n,0u);Ω}Ӂ þ>Ŵ accumulate_batch(U*ː,ҟ a,ҟ b,ҟ c,ǧ,ƍ ʁ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{accumulate_batch8<þ>(ː,a,b,c,n,ʁ)а}
#endif
Ω}Ŵ accumulate(U*ː,Ɯ a,Ɯ b,Ɯ c,ǧ,ƍ ʁ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{accumulate8(ː,a,b,c,n,ʁ)а}
#endif
ȥ(ț)ː Ƈ ƪ(ː Ĕ,ѭ(ɨ,ʁ?sub(Ϲ,c Ĕ):ƪ(Ϲ,c Ĕ)));}Ŵ scale_copy(U*ː,Ɯ src,ί,U r){
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){scale_copy8(ː,src,î,r)а}
#endif
U q=ť(r);ȥ(Ͼ î Į)ː Ƈ ɲ(src Ĕ,r,q);}Ĉ ŧ(қ,ί,U r)Ù{
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){scale8(a,î,r)а}
#endif
ý i Ē U q=ť(r)ø(Ͼ î Į)ӫ ɲ(ɨ,r,q);}Ŵ pair_product(қ,Ɯ b,ί){
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){pair_product8(a,b,î)а}
#endif
ȥ(Ͼ î Į)ӫ ѭ(b ɔ,b ư);}Ŵ pair_multiply(қ,Ɯ b,ί){
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){pair_multiply8(a,b,î)а}
#endif
ȥ(Ͼ î Į){U x=a ɔ;a ɔ=ѭ(Ϲ,a ư);a ư=ѭ(Ϲ,x);}}Ŵ ɸ(қ,Ɯ b,Ɯ c,ί){
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){difference8(a,b,c,î)а}
#endif
ȥ(Ͼ î Į)ӫ sub(Ϲ,c Ĕ);}Ŵ ѷ(қ,ý İ){ȥ(Ͼ İ Į)ӫ a[İ+i]=ƪ(ɨ,a[İ+i]);}Ĉ project(қ,ý χ,ý İ)Ù{if(İ у){ý j ƒ(;j<χ ų)a[2*j]=a[2*j+1]=ƪ(a[2*j],a[2*j+1])а}if(İ==2){ý j ƒ(;j<χ ų){U*r=a+4*j;U x=ƪ(ƪ(r Ŭ,r ƹ),ƪ(r ǥ,r Ѐ));U y=ƪ(sub(r Ŭ,r ƹ),ɲ(sub(r ǥ,r Ѐ),ô ƹ,ſ ƹ));r Г ƪ(x,y);r ƹ=sub(x,y);y=ɲ(y,ķ ƹ,Ƣ ƹ);r ǥ=ƪ(x,y);r Ѐ=sub(x,y);}Ú;}Ć(ȓ χ ų){U*ξ=a ʳ İ*j;fft ԏ(ξ,2*İ,İ);ѷ(ξ,İ);Ҭ(ξ,2*İ,İ);}}Ĉ composition_difference(қ,Ɯ b,Ɯ c,ǧ)Ù{ɸ(a,b,c,n);}Ĉ composition_finish(U*p,Ɯ v,ý İ,U Ÿ)Ù{Ć(ö İ Į)p Ƈ ѭ(sub(ƪ(v Ĕ,v[İ+i]),ƪ(v[2*İ+i],v[3*İ+i])),Ÿ);}ƛ CompositionDelete{Ĉ ŕ()(ϔ*p)Ù{::ŕ delete[](p,ɴ);}}ͼ CompositionWords=Ø unique_ptr<ϔ[],CompositionDelete>;Ĉ ˣ(қ,ý ä,ý İ)Ù{ý h=ä ø(;h/2>İ;h>>=2)ϋ ə(a,h/2,Ё)ĺ(h>İ)stage ə(a,h,Ё);}Ĉ double_y(қ,ý χ,ý İ,ƍ ć)Ù{ȟ ä=χ*İ;ý h=2*İ ø(;h<=ä/2;h<<=2){
#if defined(__x86_64__) || defined(__i386__)
if(h ɩ)radix4_8 ԏ(a,h,Ё)і
#endif
ϋ ԏ(a,h,Ё);}if(h<=ä)stage ԏ(a,h,Ё)ø(ȓ χ ų)ŧ(a ʳ İ*j,2*İ,(ć?ϻ:twists)[χ+j]);}Ŵ projection_dot(U*p,Ɯ q,ǧ){Ć(Е){U x=p ɔ,y=p ư,a=q ɔ,b=q ư њ ĺ(y>=Ƒ)y-=Ƒ ĺ(a>=Ƒ)a-=Ƒ ĺ(b>=Ƒ)b-=Ƒ;U r=ń((о)x*b+(о)y*a);p Ƈ r<Ƒ?r:r-Ƒ;}}Ĉ ʔ{ķ.swap(ô);Ƣ.swap(ſ);ӳ.swap(ώ);Ѳ.swap(ˇ);}U reversion_projection(U*p,U*q,ý ä,Ɯ ş=Ƿ){ý ţ=__builtin_ctz((ϔ)ä);V saved(ţ,one);U k8=one,j9=one ø(Ż 1,İ=ä/2;İ>1 Į,İ>>=1){k8=ѭ(ѭ(k8,k8),in(2*İ));saved Ƈ k8;}Ć(Ż ţ-1,İ=2;i и i,İ<<=1){j9=ѭ(j9,saved Ĕ)ĺ(i)j9=ѭ(j9,in(4*İ));}U Ÿ=T ɪ(ѭ(j9,in(4*ä)))ˆ.v;Ҭ(q,Ё)ø(ö Ё Į){U x=q Ĕ;q Ƈ ƪ(x,one);q[Ё+i]=sub(x,one);}if(ş){Ć(ö ä Į){p Ƈ p[ä+i]=ş Ĕ;}ʔ;Ҭ(p,Ё,ä);ʔ ø(ö Ё Į)p[Ё+i]=sub(0,p Ĕ);projection_dot(p,q,Ё);}ŝ{Ć(ö ä Į){U x=ɲ(sub(q ư,q ɔ),ķ Ĕ,Ƣ Ĕ);p Ƈ i<ä/2?x:sub(0,x);p[ä+i]=sub(0,p Ĕ);}}k8=one ø(ý İ=ä/2,χ=2;İ>1;İ>>=1,χ<<=1){pair_product(q,q,Ё);k8=ѭ(k8,k8);project(q,χ,İ)Ԗ(q,Ё,q ʳ ä);double_y(q ʳ ä,χ,İ,Ś)ӻ q[Ё+i]=sub(q[Ё+i],d4);ˣ(q ʳ ä,ä,İ);ʔ;project(p,χ,İ)ø(ö Ё Į)p[Ё+i]=sub(0,p Ĕ);double_y(p ʳ ä,χ,İ,Ɔ);ˣ(p ʳ ä,ä,İ);ʔ;projection_dot(p,q,Ё);}Ć(ö ä Į)p Ƈ ƪ(
p ɔ,p ư);ʔ;fft ԏ(p,ä);ʔ Ū Ÿ;}Ĉ compose(U*p,Ɯ q,ý χ,ý İ,U&j9,U&k8,U*p6=Ƿ,ý Ȍ=0)Ù{ȟ ä=χ*İ ĺ(İ у){Ҭ(p,χ)ø(Ż χ-1;i и i)p ɔ=p ư=p Ĕ а}CompositionWords ʟ ĺ(!p6){ý ţ=__builtin_ctz((ϔ)İ);ʟ.reset(ú ϔ*>(::ŕ new[]((4*ä ү)*ţ*ƚ(ϔ),ɴ)));p6=ʟ Ӳ;}U*v=p6+(4*ä ү)*Ȍ ĺ(χ у){ʇ(v+İ,İ,U(0))ø(ö İ Į)v Ƈ sub(0,q Ĕ);Ҭ(v,2*İ)ø(ö 2*İ Į){U x=v Ĕ;v Ƈ ƪ(x,one);v[i ʳ İ]=sub(x,one);}}ŝ{pair_product(v,q,Ё);k8=ѭ(k8,k8);project(v,χ,İ)Ԗ(v,Ё,v ʳ ä);U*ʓ=v ʳ ä;double_y(ʓ,χ,İ,Ś)ӻ ʓ Ƈ sub(ʓ Ĕ,d4);ˣ(ʓ,ä,İ);}U saved=k8;compose(p,v,2*χ,İ/2,j9,k8,p6,Ȍ+1);pair_multiply(v,p,Ё);j9=ѭ(j9,saved)ĺ(χ у){fft ԏ(v,2*İ,İ);fft ԏ(v ʳ İ,2*İ,İ);U Ÿ=in(T(į(ѭ(j9,in(4*ä))))ˆ ȃ);composition_finish(p,v,İ,Ÿ);}ŝ{U*ʓ=v ʳ ä;double_y(ʓ,χ,İ,Ɔ);ˣ(ʓ,ä,İ);j9=ƪ(j9,j9);composition_difference(p,v,ʓ,Ё);project(p,χ,İ);j9=ѭ(j9,in(2*İ));}}};ò NTT&ntt_plan(ý ä){ċ NTT t(1,Ś)ĺ(ó(t.ķ Û)<ä)t=NTT(ä,Ś)Ū t;}}í ntt(Ʋ&a,ƍ ć=Ś){ơ(!a ű&&!(Ư&(Ư-1))&&Ư<=(1U<<23));ǧ=ó(Ư)Ѵ&t=ç ntt_plan(Ŋ(1,n/2))Ѵ reorder=Ѯ{Ć(Ż 1,j Ē i<n Į){ý bit=n>>1 ø(;j&bit;bit>>=1)j^=bit;j^=bit ĺ(i<j)ь(ɨ,a Ɓ);}};Ǝ
(ƚ(Ě)==ƚ(Ű U)&&ƈ Ě>)Ѵ*č=õ Ű U*>(ʰ)ĺ(ć){reorder();t.fft ԏ(č,n);t.ŧ(č,n,Ě(n)ˆ ȃ);}ŝ{t.Ҭ(č,n);reorder();}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ ƛ Ď;ӂ{Ő T,Ũ Work=Ě>ƛ Operations{Ѽ ͼ W=Ď<Work>ͼ Ƽ=ѝ ͼ Word=ę U;ċ W ź(Ҥ a){W r(Ư)ø(ö ó(Ư)Į)r Ƈ Ě ι(ɨ ȃ)Έ}ċ F ƿ(Ù W&a){F r(Ư)ø(ö ó(Ư)Į)r Ƈ T ι(ɨ ȃ)Έ}ċ ç NTT&ƀ(ǧ){Ú ç ntt_plan(n);}ċ ɓ č(Ƽ&a){Ǝ(ƚ(T)==ƚ(Word)&&ƈ T>)Ū õ ɓ>(ʰ);}ċ Ù ɓ č(Ҥ a){Ú õ Ù ɓ>(ʰ);}ή ć>Ŵ Җ(Ƽ&a){ǧ=Ư;ơ(n&&!(n&(ԕ))&&n ɰ)Ѵ&t=ƀ(Ŋ(1,n/2));t.Ą fft<ć>(č(a),n);ƴ t.normalize(č(a),n);}Ŵ ntt(Ƽ&a){Җ ə(a);}Ŵ ʒ(ɓ a,ǧ){ĵ(a,n/2,a+n/2);ƀ(Ŋ(1,n/2)).Ą Ҭ(a,n,n/2);}Ŵ ʒ(Ƽ&a){ǧ=Ư Ԗ(a ü,n/2,a ʷ n/2);ƀ(Ŋ(1,n/2)).Ą Ҭ(č(a),n,n/2);}Ŵ Ʊ(Ƽ&a){Җ ԏ(a);}Ŵ Ζ(Ƽ&a,T Ÿ){if(a ű)Ú;ġ{ł&t=ƀ(1);t.ŧ(č(a),Ư,Ÿ ȃ);}ŝ Ć(T&x:a)x*=Ÿ;}Ŵ ɘ(Ƽ&a,T ù,T ɯ){ġ Ű geometric(č(a),Ư,ù.v,ɯ.v)і Ć(T&x:a){x*=ù;ù*=ɯ;}}Ŵ batch_inverse(Ƽ&ː,Ҥ src){ǧ=src Û;
#if defined(__x86_64__) || defined(__i386__)
ġ{if(n>=128 ȏ){Ű batch_inverse8(č(ː),č(src),n)а}}
#endif
Ƽ ʜ(n);T p=1 ø(Е)ʜ Ƈ p,p*=src Ĕ;ơ(p!=Ȓ);p=p ˆ ø(Ż ԕ;i и i){ː Ƈ ʜ Ĕ*p;p*=src Ĕ;}}Ŵ ȇ(Ƽ&r,Ҥ a,Ҥ b,ǧ){ġ Ű Ŏ(č(r),č(a),č(b),n)і Ć(Е)r Ƈ ɨ*Ϲ;}
#if defined(__x86_64__) || defined(__i386__)
Ӂ þ,ƍ ù>ˌ accumulate_products8(ɓ ː,Ù ɓ Ù*a,Ù ɓ Ù*b,ǧ){Ę I8=Ű I8 В p ʖ(Ҳ),pp ʖ(2*Ҳ),ˠ ø(ö n Ɲ){I8 Ŧ=đ,Ȏ=Ŧ;
#pragma GCC unroll 7
Ć(ˋ þ ȝ){ͳ Ű œ(a ƫ+i),y=Ű œ(b ƫ+i)ɳ}ӹ Ű ɾ(Ű œ(ҷ),ĳ);Ű ŗ(ҷ,ĳ);}}
#endif
Ӂ þ,ƍ ù>Ŵ accumulate_products(ɓ ː,Ù ɓ Ù*a,Ù ɓ Ù*b,ǧ){Ǝ(þ>=1&&þ<=7);
#if defined(__x86_64__) || defined(__i386__)
if(n>=8 ȏ){accumulate_products8<þ,ù>(ː,a,b,n)а}
#endif
Ć(Е){Word ĳ ƒ(ˋ þ ȝ)ĳ=Ű ƪ(ĳ,Ű ѭ(a ƫ Ĕ,b ƫ Ĕ))Ѓ(!ù)ĳ=Ű ƪ(ĳ,ː Ĕ);ː Ƈ ĳ;}}Ŵ join_windows(ɓ ǽ,ί,ý þ){Ц(ǽ,î,þ,î);}Ŵ Ц(ɓ ǽ,ί,ý þ,ý k4){Ć(Ż þ-1;i>0;--i){ɓ a=ǽ+i*k4 ĩ ɓ b=a-k4 ø(ȓ î/2 ų)a ˉ Ű ƪ(a Ɓ,b Ɓ)ø(ý j=î/2;j<î ų)a ˉ Ű sub(a Ɓ,b Ɓ);}}Ŵ inverse_extend(F&h,Ҥ a,Ҥ Ǥ){ȟ m=ó(Ǥ Û)ĩ T im=T ι(ǘ m);Ƽ b(m);ȇ(b,a,Ǥ,m);Ʊ(b);ż(b ü,b ʷ m/2,Ȓ);ntt(b);ȇ(b,b,Ǥ,m);Ʊ(b);h Ř(m)ø(Ż m/2 Ͼ m Į)h Ƈ-Ϲ*im*im;}ċ T dot(Ù T*a,Ù T*b,ǧ){
#if defined(__x86_64__) || defined(__i386__)
ġ{if(n>=64 ȏ)Ú T ɪ(Ű reverse_dot8(õ Ù ɓ>(a),õ Ù ɓ>(b),n));}
#endif
ϔ ă ƒ(ö n ƽ 16){о ʂ ƒ(ý j=i;j<Ħ(n,i ү)ų)ʂ+=(о)a Ɓ ȃ*b[-j]ȃ;ă+=ʂ%Ҳ ĺ(ă>=Ҳ)ă-=Ҳ;}Ú T ι(ă);}ċ ý Ѫ(ħ Ø pair<ý,T>>&Ơ){Ӄ ƒ(ł&[j,x]:Ơ){ď=Ø gcd(ď,j);ġ if(x.v>=Ҳ)x.v-=Ҳ;}Ú ď;}ċ T sparse_dot(Ù ħ Ø pair<ý,T>>&Ơ,ʧ g,ý i){ġ{Ǝ(7ULL*(Ҳ-1)*(2ULL*Ҳ-1)+0xffffffffULL*Ҳ<~0ULL);о ʂ Ē Word ă Ē ý þ ƒ(ł[j,x]:Ơ){if(j>i)Ϸ ʂ+=(о)x.v*g[i-j].v ĺ(++þ==7){ă=Ű ƪ(ă,Ű ń(ʂ)%Ҳ);ʂ Ē þ Ē}}Ú T ɪ(Ű ƪ(ă,Ű ń(ʂ)%Ҳ));}ŝ{T ă ƒ(ł[j,x]:Ơ){if(j>i)Ϸ ă+=x*g[i-j];}Ѡ}}Ŵ intt(Ƽ&a){Ʊ(a)ĩ T ё=T ι(ǘ ó(Ư));Ζ(a,ё);}ċ T mod_sqrt(T a){if(a Ү||a.pow((Ҳ-1)/2)!=Ӹ)Ú 0;T r=a.pow(60),t=a.pow(119),c=T(3).pow(119);ý m=23 ǟ(t!=Ӹ){ȥ(T u=t;u!=Ӹ;u*=u)++i;T b=c.pow(1<<(m-i-1));r*=b;c=b*b;t*=c;m=i;}Ú r;}ċ Ҥ inverse_ntt_roots(ǧ){ċ Ƽ ķ{1}ǟ(ó(ķ Û)<n){ҝ ˆ Ԙ}Ú ķ;}ċ Ҥ ntt_roots(ǧ){ċ Ƽ ķ{1}ǟ(ó(ķ Û)<n){ҝ Ԙ}Ú ķ;}ċ Ù ѝ&inverses(ǧ){ơ(ī(n)<ī(Ҳ));ċ ѝ ё{0,1};
#if defined(__x86_64__) || defined(__i386__)
ġ{ý ù=ё Û,þ=(n+1-ù)/8*8 ĺ(þ>=256 ȏ){ё.reserve(n+1);ё Ř(Ў þ);Ű inverse_sequence8(õ ɓ>(ё à)+ù,ù,þ);}}
#endif
while(ó(ё Û)<=n){Ù Ż ó(ё Û);ё ӱ(-ё[Ҳ%i]*(Ҳ/i));}Ú ё;}Ŵ trim(F&a){while(!a ű&&a Ӱ Ү)a.pop_back();}};}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ӂ{ā horner_detail{Ӂ K,Ũ T>ò T scalar(Ù T*a,ǀ,T x){Ǝ(K>0);T ɯ=1 ø(ö K Į)ɯ*=x;ǖ T,K>ʂ{};Õ f5=n/K,Η=n%K ø(Ů Η Į)ʂ Ƈ a[f5*K+i]ǟ(f5){--f5 ø(ö K Į)ʂ Ƈ ʂ Ĕ*ɯ+a[f5*K+i];}T ă ƒ(Ż K;i-->0;)ă=ă*x+ʂ Ĕ Ȧ}
#if defined(__x86_64__) || defined(__i386__)
Ӂ K>Ü ò Ě vector(ǣ*a,ǀ,Ě x){Ǝ(K у||K==2||K==4)ͼ V=ę I8;Ǝ(ƈ Ě>&&ƚ(Ě)==ƚ(ī));Ě ɯ=x ø(ö 3 Į)ɯ*=ɯ ĩ ī x8=ɯ ȃ ø(λ K;i*=2)ɯ*=ɯ ĩ ī ĳ=ɯ ȃ ĩ V Ÿ ʖ(ĳ),ť ʖ(ę ť(ĳ));V ʂ[K]ø(ł&v:ʂ)v=đ;ɿ ҕ=8*K;Õ f5=n/ҕ,Η=n%ҕ ĺ(Η){Ť Ě ŀ[ҕ]{};Ǵ(ш(ŀ),a+f5*ҕ,Η*ƚ(Ě))ø(ö K Į)Ǵ(&ʂ Ĕ,ŀ Ʀ i,ƚ(V));}м(f5){--f5 ø(ö K Į)Ǵ(&ʂ Ĕ,a+f5*ҕ Ʀ i,ƚ(V));}while(f5){--f5 ø(ö K Į){V ʑ;Ǵ(&ʑ,a+f5*ҕ Ʀ i,ƚ(V));ʂ Ƈ ę ɾ(ĳ у?ʂ Ĕ:ę ƥ(ʂ Ĕ,Ÿ,ť),ʑ);}}Ù V n5 ʖ(x8),l0 ʖ(ę ť(x8));V h1=ʂ[K-1]ø(Ż K-1;i-->0;)h1=ę ɾ(ę ƥ(h1,n5,l0),ʂ Ĕ);Ť Ě ɽ ɚ;Ǵ(ш(ɽ),&h1,ƚ(h1));Ě ă ƒ(Ż 8;i-->0;)ă=ă*x+ɽ Ĕ Ȧ}
#endif
}ò Ě horner(ǣ*a,ǀ,Ě x){if(!n)Ú Ě(0)ĺ(x==Ě(0))Ú a Ŭ;
#if defined(__x86_64__) || defined(__i386__)
if(n>=32&&ę Ĝ){if(n<64)Ú horner_detail::vector<1>(a,n,x)Ū horner_detail::vector<4>(a,n,x);}
#endif
if(n>=32)Ú horner_detail::scalar<8>(a,n,x);Ě ă Ē while(n)ă=ă*x+a[--n]Ȧ}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
ӂ{
#if defined(__x86_64__) || defined(__i386__)
Ü í factorial_transpose(ę I8(&x)ɚ){ę I8 a ɚ,b ҹ(ö 8 ƽ 2){ӫ _mm256_unpacklo_epi32(x Ĕ,x Υ);a Υ ǿ(x Ĕ,x Υ);}Ć(ö 8 ƽ 4){b Ƈ ɻ(ɨ,a[i+2]);b Υ Ǿ(ɨ,a[i+2]);b[i+2]=ɻ(a Υ,a[i+3]);b[i+3]Ǿ(a Υ,a[i+3]);}Т{x Ƈ Ł(Ϲ,b[i+4],0x20);x[i+4]=Ł(Ϲ,b[i+4],0x31);}}ή ˁ>Ü í factorial_chain(ҿ į,ý ѩ,ę I8 ĳ,ę I8 Ÿ){Ę I=ę I8;I ɯ ʖ(ę one);I с=ʞ ø(ȓ ѩ Ӗ){I ŀ ҹ(ˋ 8 ȝ){ŀ ђ ĳ;ĳ=ę ɏ(ĳ,Ÿ)Ѓ(ˁ)Ÿ=ę sub8(Ÿ,ɯ)і Ÿ=ę ɾ(Ÿ,ɯ);}factorial_transpose(ŀ)ø(ˋ 8 ȝ){Ĺ(ˁ)ę ŗ(ѓ(k+1)*ѩ-j-8,Ľ(ŀ ƫ,с))і ę ŗ(ѓ k*ѩ+j,ŀ ƫ);}}}Ü í factorials8(ҿ ԓ,ҿ g9,ǧ){Ę I=ę I8 ͼ U=ę U ȉ ѩ=n/256*8,d5=ѩ*32;Ť U Ÿ[32],ƕ[32],n4[33],ć[33];I p ɛ,x ɛ,ɯ ʖ(ę one)ø(ö 32 Į)Ÿ Ƈ ę in(i*ѩ+1)ø(ˋ 4 ȝ)p ƫ ʖ(ę one),x ђ ę œ(Ÿ Ʀ k)ø(ö ѩ Į){ʢ p ђ ę ɏ(p ƫ,x ƫ),x ђ ę ɾ(x ƫ,ɯ);}ʢ ę ŗ(ƕ Ʀ k,p ƫ);n4 Г ę one ø(ö 32 Į)n4 Υ=ę ѭ(n4 Ĕ,ƕ Ĕ);U Ȣ=n4[32]ø(Ż d5 ț)ԓ Ƈ Ȣ,Ȣ=ę ѭ(Ȣ,ę in(i+1));Ȣ=Ҿ(Ȣ)ˆ.v ø(Ż ԕ;i>=d5;--i)Ȣ=ę ѭ(Ȣ,ę in(i+1)),g9 Ƈ Ȣ;ć[32]=Ȣ ø(Ż 31;i и i)ć Ƈ ę ѭ(ć Υ,ƕ Ĕ)ø(ˋ 4 ȝ){factorial_chain ə(ԓ Ʀ k*ѩ,ѩ,ę œ(n4 Ʀ k),ę œ(Ÿ Ʀ k));Ť U Ƴ ɚ,e2 ҹ(ȓ 8 ų){ȟ end=(8*k+j+1)*ѩ;Ƴ ˉ ę ѭ(ć[8*k+j+1],ę in(end)
);e2 ˉ ę in(end-1);}factorial_chain ԏ(g9 Ʀ k*ѩ,ѩ,ę œ(Ƴ),ę œ(e2));}}
#endif
í Ө(ҿ ԓ,ҿ g9,ǧ){if(!n)Ú;
#if defined(__x86_64__) || defined(__i386__)
if(n>=256&&ę Ĝ){factorials8(ԓ,g9,n)а}
#endif
ę U ĳ=ę one ø(Е)ԓ Ƈ ĳ,ĳ=ę ѭ(ĳ,ę in(i+1));ĳ=Ҿ(ԓ[ԕ])ˆ.v ø(Ż ԕ;i и i)g9 Ƈ ĳ,ĳ=ę ѭ(ĳ,ę in(i));}ˬ requires Ĥ T,Ě>í Ө(T*ԓ,T*g9,ǧ){ơ(0<=n&&ī(n)<=Ƀ);Ǝ(ƚ(Ě)==ƚ(ę U)&&ƈ Ě>)ĺ(ī(n)==Ƀ){Ө(ԓ,g9,ԕ);ԓ[ԕ]=-ԓ[n-2];g9[ԕ]=-g9[n-2]а}Ө(õ ҿ>(ԓ),õ ҿ>(g9),n);}}
#if defined(__linux__)
#include <sys/mman.h>
#endif
ӂ{ƛ FpsWordsDelete{Ĉ*region=Ƿ;Õ bytes Ē Ĉ ŕ()(ϔ*p)Ù{
#if defined(__linux__)
if(region){munmap(region,bytes)а}
#endif
::ŕ delete[](p,ɴ);}}ͼ FpsWords=Ø unique_ptr<ϔ[],FpsWordsDelete>;ò FpsWords ɀ(Ƞ){
#if defined(__linux__) && defined(MADV_HUGEPAGE)
ɿ page=1<<21 ĺ(þ>=page/ƚ(ϔ)&&þ<=(Ø numeric_limits<Õ>::max()-2*page)/ƚ(ϔ)){ģ bytes=(þ*ƚ(ϔ)+page-1)/page*page;Ĉ*region=mmap(Ƿ,bytes+page,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)ĺ(region!=MAP_FAILED){ʦ a6=(Λ(region)+page-1)&~(page-1)Ѵ*p=::new(õ Ĉ*>(a6))ϔ[þ];madvise(p,bytes,MADV_HUGEPAGE)Ū FpsWords(p,FpsWordsDelete{region,bytes+page});}}
#endif
Ú FpsWords(new(ɴ)ϔ[þ]);}}
#define À(a,b) "add " #a ", " #b "\n\t"
#define Á(a,b) "and " #a ", " #b "\n\t"
#define Ä(a,b) "cmp " #a ", " #b "\n\t"
#define Å(a) "inc " #a "\n\t"
#define Æ(a) "jne " #a "\n\t"
#define Ç(a,b) "lea " #a ", " #b "\n\t"
#define È(a,b) "mov " #a ", " #b "\n\t"
#define É(a,b) "vbroadcastss " #a ", " #b "\n\t"
#define Ê(a,b) "vmovdqa " #a ", " #b "\n\t"
#define Ë(a,b) "vmovdqu " #a ", " #b "\n\t"
#define Ì(a,b,c) "vpaddd " #a ", " #b ", " #c "\n\t"
#define Í(a,b,c) "vpaddq " #a ", " #b ", " #c "\n\t"
#define Î(a,b,c,d) "vpblendd " #a ", " #b ", " #c ", " #d "\n\t"
#define Ï(a,b) "vpbroadcastd " #a ", " #b "\n\t"
#define Ð(a,b,c) "vpminud " #a ", " #b ", " #c "\n\t"
#define Ñ(a,b,c) "vpmulld " #a ", " #b ", " #c "\n\t"
#define Ò(a,b,c) "vpmuludq " #a ", " #b ", " #c "\n\t"
#define Ó(a,b,c) "vpsrlq " #a ", " #b ", " #c "\n\t"
#define Ô(a,b,c) "vpsubd " #a ", " #b ", " #c "\n\t"
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#include <sys/mman.h>
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ӂ{ā fps_output_detail{ò ϸ Ŕ(ϸ a){ʦ b ʖ(932051910u),ni ʖ(998244351u),p ʖ(Ƀ)Ѵ Ŧ=Þ(a,b),Ȏ=Þ(ñ(a,32),b);ĥ Ú ǝ;}}í ƙ(л,ǀ){if(n<=į.ε())Ú;į.reserve(n)ĺ(n-ǲ<(1u<<17))Ú Ǎ ù=Λ(ԃ ǲ)Ǎ end=Λ(ԃ n);[[maybe_unused]]ʦ advise=[ù,end](Ø uintptr_t Б,ý a8){ʦ lo=(Ў Б-1)&~(Б-1),hi=end&~(Б-1)ĺ(lo<hi)ú Ĉ>(madvise(õ Ĉ*>(lo),hi-lo,a8));};
#if defined(MADV_HUGEPAGE)
advise(2u*1024*1024,MADV_HUGEPAGE);
#endif
#if defined(MADV_POPULATE_WRITE)
advise(4096,MADV_POPULATE_WRITE);
#endif
}ή Ŕ>í append_fps_words(л,Ù ī*in,ǀ){Ǝ(ƚ(Ě)==ƚ(ī)&&ƈ Ě>)ĺ(n ζ)Ú ĺ(n>į.max_size()-ǲ)throw Ø length_error("fps output");ƙ(į,ǲ+n);ǖ Ě,64>batch;ɵ ƒ(;i+64<=n ƽ 64){Ĺ(Ŕ)Ǵ(ш(batch à),in+i,ƚ(batch))і{
#pragma GCC unroll 8
Ć(ȓ 64 Ӗ){ʦ x=fps_output_detail::Ŕ(ɂ(õ Ù ϸ*>(in+i+j)));Ǵ(ш(batch Ʒ j),&x,ƚ(x));}}į ӯ(į Ƃ,batch ü,batch Ƃ);}Ć(ț){Ĺ(Ŕ)į θ(Ҿ(in Ĕ))і į θ(Ě ι(in Ĕ));}}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ӂ{ā ì kernels{Ę ĭ=ϸ;ò ӄ kP=998244353,k2P=2*kP,kNI=998244351;í ı(ҳ a,Ϫ,Ɠ x,Ɠ y){Ť ĭ w Ѐ;ќ
	#define Â(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) R(%[P],z) R(%[P2],r) D((%[x]),d) m(d,G)D(32(%[x]),w)D((%[y]),d)m(d,L)D(32(%[y]),E)D(4(%[y]),d)m(d,M)D(36(%[y]),F)".p2align 5\n\t" "1:\n\t" m(H,d)m(J,c)m(N,k)m(Q,g)q(H,w,u)q(J,w,s)q(4(%[a],%[h3]),w,e)q(36(%[a],%[h3]),w,i)m(I,l)v(t,u,u)m(K,n)v(t,s,s)B(A,e,u,e)B(A,i,s,i)m((%[a]),s)p(G,d,d)p(z,e,e)m(O,u)p(G,c,c)p(z,i,i)j(r,k,f)C(f,k,f)q(I,w,k)j(e,d,e)j(r,g,d)C(d,g,d)j(i,c,i)q(K,w,c)q(4(%[a],%[h],2),w,g)v(t,k,k)p(G,l,l)p(G,n,n)v(t,c,c)B(A,g,k,g)q(36(%[a],%[h],2),w,k)p(z,g,g)B(A,k,c,k)o(r,f,c)o(e,f,f)j(e,c,e)p(z,k,k)j(g,l,g)o(r,d,l)o(i,d,d)j(i,l,i)v(t,f,l)q(E,f,c)q(E,l,l)p(L,f,f)j(k,n,k)v(t,e,n)q(F,n,n)v(t,c,c)B(A,l,c,l)q(F,e,c)p(M,e,e)p(z,l,l)v(t,c,c)B(A,n,c,n)j(l,f,l)v(t,d,f)q(E,d,c)q(E,f,f)p(z,n,n)p(L,d,d)v(t,c,c)B(A,f,c,f)j(n,e,n)v(t,i,e)q(F,i,c)q(F,e,e)p(z,f,f)p(M,i,i)v(t,c,c)B(A,e,c,e)j(f,d,f)j(r,s,d)j(r,u,c)C(d,s,d)C(c,u,c)p(z,e,e)o(g,d,u)j(g,d,g)o(k,c,d)j(k,c,k)j(r,u,c)o(r,g,s)C(c,u,c)C(s,g,s)j(e,i,e)j(r,d,i)o(r,k,g)C(i,d,i)o(r,c,d)o(r,s,u)C(g,k,g)o(l,c,c)j(l,d,l)o(n,s,s)o(r,i,d)o(r,g,k)j(n,u,n)o(f,i,i)m(c,(%[a]))j(f,d,f)o(e,g,g)j(e,k,e)m(l,N)m(s,I)m(n,H)m(i,O)m(f,Q)m(g,K)m(e,J)S($64,%[a])T(%[end],%[a])U(1b)
ӧ(Â(%%ymm1,%%ymm0,%%ymm6,%%ymm10,%%ymm3,%%ymm7,Ô,%%ymm2,%%ymm8,Ê,%%ymm9,Ì,Ñ,Ò,%%ymm14,%%ymm5,$32,%%ymm4,Ó,%%ymm11,%%ymm15,$170,Î,Ð,É,%%ymm12,%%ymm13,(%[w]),(Ϻ,%[h3]),(Ϻ,%[h],2),32(Ϻ,%[h3]),32(Ϻ,%[h],2),32(%[w]),64(%[w]),(Ϻ,%[h]),32(Ϻ),32(Ϻ,%[h]),Ï,À,Ä,Æ)
#undef Â
ʆ,ӭ,ұ,[y]"r"(y),[x]"r"(x),[w]"r"(w),[P]"m"(kP),Ӭ
		: Ã);
}í ć(ҳ a,Ϫ,Ɠ x,Ɠ y){Ť ĭ w Ѐ;ќ
	#define Â(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T) Q(%[P],w) Q(%[P2],k) C((%[x]),c) r(c,F)C(32(%[x]),v)C((%[y]),c)r(c,K)C(32(%[y]),D)C(4(%[y]),c)r(c,L)C(36(%[y]),E)".p2align 5\n\t" "1:\n\t" r((%[a]),c)r(N,f)r(M,s)r(O,t)n(k,c,i)n(H,c,c)n(k,f,j)h(H,i,i)n(J,f,f)h(k,c,u)n(k,s,e)h(J,j,j)q(l,i,g)p(D,i,d)n(k,t,m)h(G,e,e)p(D,g,g)o(K,i,i)h(I,m,m)n(G,s,s)B(u,c,u)q(l,j,c)p(D,c,c)q(l,d,d)n(I,t,t)A(z,g,d,g)p(D,j,d)o(K,j,j)o(w,g,g)q(l,d,d)A(z,c,d,c)h(g,i,g)q(l,e,i)p(E,e,d)p(E,i,i)o(w,c,c)o(L,e,e)q(l,d,d)A(z,i,d,i)h(c,j,c)q(l,m,j)p(E,m,d)p(E,j,j)o(w,i,i)o(L,m,m)q(l,d,d)A(z,j,d,j)h(i,e,i)h(k,f,e)h(k,s,d)o(w,j,j)B(e,f,e)B(d,s,d)h(k,t,s)n(k,u,f)B(s,t,s)n(k,e,t)n(d,u,u)h(d,f,d)h(s,t,t)n(s,e,s)n(k,g,e)h(j,m,j)n(k,c,m)h(i,e,e)q(l,d,f)h(j,m,m)p(v,f,f)n(i,g,i)n(j,c,j)p(v,d,c)q(l,e,g)o(F,d,d)p(v,g,g)q(l,c,c)A(z,f,c,f)p(v,e,c)o(F,e,e)o(w,f,f)q(l,c,c)A(z,g,c,g)h(f,d,f)q(l,t,d)p(v,t,c)p(v,d,d)o(w,g,g)r(f,M)q(l,m,f)p(v,f,f)o(F,t,t)q(l,c,c)A(z,d,c,d)h(g,e,g)p(v,m,e)o(F,m,m)o(w,d,d)h(k,u,c)r(g,G)h(k,i,g)B(c,u,c)q(l,e,e)h(k,s,u)B(g,i,g)r(c,(%[a]))A(z,f,e,f)h(k,j,e)B(u,s,u)h(d,t,d)r(g,H)o(w,f,f)B(e,j,e)r(u,N)r(e,J)r(d,O)h(f,m,f)r(f,I)R($64,%[a])S(%[end],%[a])T(1b)
ӧ(Â(%%ymm0,%%ymm9,%%ymm7,%%ymm1,%%ymm8,Ô,%%ymm4,%%ymm5,%%ymm14,$32,%%ymm10,Ì,Ñ,Ò,Ó,Ê,%%ymm2,%%ymm3,%%ymm6,%%ymm11,%%ymm15,$170,Î,Ð,É,%%ymm12,%%ymm13,(%[w]),(Ϻ,%[h3]),(Ϻ,%[h]),32(Ϻ,%[h3]),32(Ϻ,%[h]),32(%[w]),64(%[w]),(Ϻ,%[h],2),32(Ϻ),32(Ϻ,%[h],2),Ï,À,Ä,Æ)
#undef Â
ʆ,ӭ,ұ,[y]"r"(y),[x]"r"(x),[w]"r"(w),[P]"m"(kP),Ӭ
		: Ã);
}í forward_identity(ҳ a,Ϫ,Ɠ y){ќ
	#define Â(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N) K(%[P],A) K(%[P2],d) I(4(%[y]),z) I(36(%[y]),t)".p2align 5\n\t" "1:\n\t" j(C,o)j(G,g)j(B,p)j(F,l)j((%[a]),i)j(D,m)j(E,q)j(H,r)c(d,o,h)c(d,g,f)n(h,o,h)c(d,p,o)n(f,g,f)k(d,h,g)c(d,l,s)n(o,p,o)k(d,f,p)n(s,l,s)c(o,g,g)c(d,i,l)k(o,h,o)c(s,p,p)x(u,g,h)w(t,g,e)n(l,i,l)x(u,p,i)w(t,h,h)v(z,g,g)k(s,f,s)w(t,p,f)w(t,i,i)x(u,e,e)v(z,p,p)J($170,h,e,h)c(d,m,e)x(u,f,f)n(e,m,e)v(A,h,h)c(d,q,m)n(m,q,m)J($170,i,f,i)c(d,r,f)k(e,l,q)n(f,r,f)c(e,l,e)v(A,i,i)c(d,q,l)k(f,m,r)c(f,m,f)k(d,e,m)n(l,q,l)c(d,r,q)n(m,e,m)c(d,o,e)c(h,g,h)k(d,f,g)n(q,r,q)c(d,s,r)n(e,o,e)n(g,f,g)k(d,l,f)k(d,m,o)n(r,s,r)c(i,p,i)k(d,q,p)k(d,g,s)k(e,l,l)c(e,f,e)k(h,m,m)c(h,o,h)k(r,q,q)j(l,(%[a]))c(r,p,r)k(i,g,g)c(i,s,i)j(e,C)j(m,D)j(h,B)j(q,E)j(r,G)j(g,H)j(i,F)L($64,%[a])M(%[end],%[a])N(1b)
ӧ(Â(Ô,%%ymm14,%%ymm11,%%ymm9,%%ymm1,%%ymm8,%%ymm4,Ê,Ì,%%ymm3,%%ymm5,Ð,%%ymm0,%%ymm2,%%ymm6,%%ymm7,%%ymm10,%%ymm13,$32,Ñ,Ò,Ó,%%ymm12,%%ymm15,(Ϻ,%[h3]),(Ϻ,%[h]),(Ϻ,%[h],2),32(Ϻ),32(Ϻ,%[h3]),32(Ϻ,%[h]),32(Ϻ,%[h],2),É,Î,Ï,À,Ä,Æ)
#undef Â
ʆ,ӭ,ұ,[y]"r"(y),[P]"m"(kP),Ӭ
		: Ã);
}í inverse_identity(ҳ a,Ϫ,Ɠ y){ќ
	#define Â(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N) K(%[P],A) K(%[P2],f) I(4(%[y]),z) I(36(%[y]),t)".p2align 5\n\t" "1:\n\t" p(F,c)p(H,r)p((%[a]),n)p(G,j)h(f,c,o)h(B,c,c)h(f,r,i)e(B,o,o)h(D,r,r)e(f,c,s)e(D,i,i)x(u,o,d)w(t,o,g)v(z,o,o)x(u,i,k)w(t,i,m)w(t,d,d)h(C,n,q)w(t,k,k)v(z,i,i)e(C,n,n)l(s,c,s)x(u,g,g)h(E,j,c)e(E,j,j)x(u,m,m)J($170,d,g,d)e(f,q,g)J($170,k,m,k)v(A,d,d)l(g,q,g)h(f,n,q)v(A,k,k)e(f,c,m)l(q,n,q)h(s,g,n)l(m,c,m)e(s,g,s)e(f,r,g)h(f,j,c)e(d,o,d)l(g,r,g)l(c,j,c)e(f,n,j)e(k,i,k)h(d,q,i)e(d,q,d)h(g,m,q)h(k,c,r)e(g,m,g)e(k,c,k)e(f,i,c)h(f,s,m)h(f,d,o)l(j,n,j)l(c,i,c)e(f,q,i)e(f,r,n)l(m,s,m)l(o,d,o)p(j,(%[a]))h(f,g,j)h(f,k,d)l(i,q,i)l(n,r,n)p(c,C)l(j,g,j)l(d,k,d)p(m,F)p(o,B)p(i,G)p(n,E)p(j,H)p(d,D)L($64,%[a])M(%[end],%[a])N(1b)
ӧ(Â(%%ymm0,%%ymm7,Ô,%%ymm14,%%ymm8,Ì,%%ymm5,%%ymm3,%%ymm9,Ð,%%ymm10,%%ymm2,%%ymm4,Ê,%%ymm11,%%ymm1,%%ymm6,%%ymm13,$32,Ñ,Ò,Ó,%%ymm12,%%ymm15,(Ϻ,%[h3]),(Ϻ,%[h]),32(Ϻ,%[h3]),32(Ϻ,%[h]),(Ϻ,%[h],2),32(Ϻ),32(Ϻ,%[h],2),É,Î,Ï,À,Ä,Æ)
#undef Â
ʆ,ӭ,ұ,[y]"r"(y),[P]"m"(kP),Ӭ
		: Ã);
}í scale_radix2(ҳ a,Ϫ,Ɠ y){ќ
	#define Â(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K) H(%[P],m) H(%[P2],C) G(4(%[y]),s) G(36(%[y]),l)".p2align 5\n\t" "1:\n\t" r((%[a]),c)r(F,d)z(C,c,e)z(D,c,c)z(C,d,f)k(D,e,e)z(E,d,d)q(n,c,h)k(E,f,f)p(l,c,v)q(n,e,i)p(l,e,w)q(n,d,j)p(l,d,x)q(n,f,g)p(l,f,u)p(l,h,h)p(l,i,i)p(l,j,j)p(l,g,g)q(n,v,v)q(n,w,w)q(n,x,x)q(n,u,u)o(s,c,c)o(s,e,e)A(t,h,v,h)A(t,i,w,i)o(s,d,d)o(s,f,f)A(t,j,x,j)A(t,g,u,g)o(m,h,h)o(m,i,i)o(m,j,j)o(m,g,g)k(h,c,h)k(i,e,i)k(j,d,j)k(g,f,g)k(m,h,f)k(m,i,d)k(m,j,e)k(m,g,c)B(f,h,f)B(d,i,d)B(e,j,e)B(c,g,c)r(f,(%[a]))r(d,D)r(e,F)r(c,E)I($64,%[a])J(%[end],%[a])K(1b)
ӧ(Â(%%ymm0,%%ymm1,%%ymm2,%%ymm3,%%ymm10,%%ymm4,%%ymm6,%%ymm8,Ô,%%ymm13,%%ymm15,$32,Ñ,Ò,Ó,Ê,%%ymm12,$170,%%ymm11,%%ymm5,%%ymm7,%%ymm9,Ì,Î,Ð,%%ymm14,(Ϻ,%[h]),32(Ϻ,%[h]),32(Ϻ),É,Ï,À,Ä,Æ)
#undef Â
ʆ,ӭ,ұ,[y]"r"(y),[P]"m"(kP),Ӭ
		: Ã);
}}ā blocked_ntt{ò ӄ ϵ=Ƥ kP;ò æ ý Ҕ=25;Ϧ{Ę Ƥ ĭ ͼ Ƥ kP;ӄ Đ(Ö x,Ö y){Ú Ö(ŉ(x)*y Ұ);}ӄ Ǹ(Ö x,Ö e){Ö r=1 ø(;e;e>>=1,x=Đ(x,x))if(e&1)r=Đ(r,x)Έ}ӄ ť(Ö w){Ú Ö((ŉ(w)<<32)/kP);}ò ӄ kR=Ö((ŉ(1)<<32)Ұ);ò æ ý kRootCount=Ҕ-4;ò æ ł kRoots=[]{ǖ ǖ Ö,kRootCount>,2>t{}ø(ȓ kRootCount ų){t Ŭ ˉ Ǹ(3,(Ͻ 1)>>(j+2));t ƹ ˉ Ǹ(t Ŭ Ɓ,ӕ);}Ú t;}();Ȅ ĕ(Ö x){Ú å(ý(x));}Ȅ ƪ(ɍ,ĭ y){Ú ň(x,y);}Ȅ Ǚ(ɍ,ĭ y){Ú é(ň(x,П),y);}Ȅ ń(ɍ,Ö c0){Ú ą(x,é(x,ĕ(c0)));}ƛ ƭ{ĭ w,q;explicit ƭ(Ö ĳ):w(ĕ(ĳ)),q(ĕ(ť(ĳ))){}ƭ(Ö ĳ,Ö o7):w(ĕ(ĳ)),q(ĕ(o7)){}};Ȅ Ź(ɍ,Œ&f){Ģ Ŧ=ñ(Þ(x,f.q),32)κ Ȏ=Þ(ʤ,f.q)κ q=Ĵ(Ŧ,Ȏ,0xAA)Ū é(ό(x,f.w),ό(q,ɟ));}Ί φ(Ȕ){Ú(k>>3<<4)|(k&7);}ò ɿ ϛ(ý lg){Ú(ǻ lg)/8 ү;}í build_table(Ņ t,Ƞ,Ù ǖ Ö,kRootCount>&ķ){t[φ(0)]=1;t[φ(0)+8]=ť(1)ø(ý Ȍ Ē Ȍ<kRootCount ė Ȍ){ģ h=ǻ Ȍ ĺ(h>=þ)Ϸ Ġ ˀ=ķ[Ȍ]ĺ(h<8){Р{Ġ v=Đ(t[φ(j)],ˀ);t[φ(h+j)]=v;t[φ(h+j)+8]=ť(v);}ӊ;}Œ by(ˀ),to_r(kR)ø(ǉ h Ӗ){Ģ v=ń(Ź(_mm256_load_si256(õ Ģ*>(t+φ(j))),by),kP)κ vr=ń(Ź(v,to_r),kP);Ѩ(Ț(t+φ(h+j)),v);Ѩ(Ț(t+φ(h+j)+8),ό(vr,ĕ(Ƥ kNI)));}}}í inverse_radix4(ҳ f,Ϫ,Ɠ ô,Œ&s){Œ z(ô ƹ,ô[9])Ǎ ŧ=[&s](ɍ)Ü{Ú
 ń(Ź(x,s),kP);}ʋ{Ģ p0=f Ɓ,p1=f ǹ,p2=f ɋ,p3=f Ɋ κ ab=ń(ƪ(p0,p1),2*kP),cd=ń(ƪ(p2,p3),2*kP)κ amb=ń(Ǚ(p0,p1),2*kP),cmd=Ź(Ǚ(p2,p3),z);f ˉ ŧ(ϓ),f ǹ=ŧ(ϒ);f ɋ=ŧ(Ќ),f Ɋ=ŧ(ϑ);}}}}ā ʃ kernels{Ę ĭ=ϸ;ò ӄ kP=998244353,k2P=2*kP;Ɩ í forward_h4(ҳ a,Ƞ,Ɠ n8,Ȕ){Ć(ǉ þ ų)ì Ƥ ı(a ү*j,4,n8+á φ(k+j),n8+á φ(2*(k+j)));}Ɩ í inverse_h4(ҳ a,Ƞ,Ɠ n8,Ȕ){Ć(ǉ þ ų)ì Ƥ ć(a ү*j,4,n8+á φ(k+j),n8+á φ(2*(k+j)));}}ā ē{ò ӄ ϵ=ì ϵ;enum Ũ Half{kBoth,kLower,kUpper};Ũ ě{Ϳ ě(Õ č):ϴ(ɀ(č)),next_(ϴ Ӳ),end_(next_+č){}ě(Ù ě&)=delete;ě&ŕ=(Ù ě&)=delete;ċ ɿ footprint(ǀ){Ú(n+7)/8*8 ү;}ĝ ś(ǀ){if(Õ(end_-next_)<footprint(n))throw Ø bad_alloc()Ѵ*start=next_;next_+=footprint(n)Ū{start,n};}ƍ zero_initialized()Ù{Ú ϴ.get_deleter().region!=Ƿ;}Ⱦ FpsWords ϴ;Ņ next_,*end_;};Ϧ{Ę á ƪ ͼ á ĕ ͼ á Ǚ ͼ á ƭ ͼ á kP ͼ á kR ͼ á ń ͼ á φ ͼ á ĭ;ò Ǔ ĭ Ō(Ɠ p){Ú _mm256_load_si256(õ Ģ*>(p));}í Š(Ņ p,ɍ){Ѩ(Ț(p),x);}Ȅ ʗ(ɍ){Ú ń(x,2*kP);}Ȅ ů(ɍ){Ú ń(ń(x,2*kP),kP);}Ȅ low_difference(ɍ,ĭ y){Ģ t=é(x,y)Ū ą(t,ň(t,П));}Ȅ ɸ(ɍ,ĭ y){Ú ń(é(ƪ(x,ɟ),y),kP);}Ȅ ŭ(ɍ,Œ&w){Ú á Ź(x,w);}ò ƭ ʮ(Ɠ n8,
Ȕ){Ɠ e=n8+φ(k)Ū ƭ(e Ŭ,e ɚ);}ò ƭ Ϛ(М,Õ p){Ɠ e=ķ+φ(p>>1)Ū p&1?ƭ(Ͻ e Ŭ,~e ɚ):ƭ(e Ŭ,e ɚ);}ƛ Τ{ƭ x,y,z;ĉ Τ(Ɠ n8,Õ g):x(ʮ(n8,g)),y(ʮ(n8,2*g)),z(ʮ(n8,2*g+1)){}};ˮ forward_h1(ĭ(&f)ɛ,Ù Τ&w){Ģ a ӑ(f Ŭ),b ӑ(f ƹ),c Ҝ(f ǥ,w.x),d Ҝ(f Ѐ,w.x)κ ac ӑ(ƪ(a,c)),amc=low_difference(a,c)κ bd Ҝ(ƪ(b,d),w.y),bmd Ҝ(Ǚ(b,d),w.z);f Г ƪ(ac,bd),f ƹ=Ǚ(ac,bd),f ǥ=ƪ(amc,bmd),f Ѐ=Ǚ(amc,bmd);}ˮ υ(ĭ(&f)ɛ,Ù Τ&w){Ģ ab ӑ(ƪ(f Ŭ,f ƹ)),cd ӑ(ƪ(f ǥ,f Ѐ))κ amb Ҝ(Ǚ(f Ŭ,f ƹ),w.y),cmd Ҝ(Ǚ(f ǥ,f Ѐ),w.z);f Г ʗ(ϓ),f ƹ ӑ(ϒ);f ǥ Ҝ(Ќ,w.x),f Ѐ Ҝ(ϑ,w.x);}ƛ alignas(64)ʅ{Ö ɽ[17];};í Ε(ʅ&Ļ,ĭ a,Œ&w){Ļ.ɽ[16]Ē Š(Ļ.ɽ+8,a);Š(Ļ.ɽ,ń(ŭ(a,w),kP));}ˮ ʿ(ʅ(&Ļ)ɛ,Ģ(&a)ɛ,М,Õ g){Œ y=ʮ(ķ,2*g),z=ʮ(ķ,2*g+1);
#pragma GCC unroll 4
Ӏ{Ļ ѯ.ɽ[16]Ē Š(Ļ ѯ.ɽ+8,a ѯ)κ wa Ҝ(a ѯ,t<2?y:z);Š(Ļ ѯ.ɽ,ń(t%2?é(П,wa):wa,kP));}}ĉ Ȅ Ǽ(Ù ʅ&Ļ,Ҡ){ȱ,p=ɟ,p2=П;н,Ȏ,bi,t0,t1,t2,t3;
	#define Â(a,c,d,e,f,g,h,i,j,k,l,m,n,o,q,r,s,t,u,v) i((%[b]),a) d(36(%[w]),a,e) d(r,a,c) i(4(%[b]),a)d(r,a,g)d(q,a,h)i(8(%[b]),a)d(q,a,j)d(o,a,k)f(g,e,e)f(h,c,c)i(12(%[b]),a)d(o,a,g)d(n,a,h)f(j,e,e)f(k,c,c)i(16(%[b]),a)d(n,a,j)d(m,a,k)f(g,e,e)f(h,c,c)i(20(%[b]),a)d(m,a,g)d(l,a,h)f(j,e,e)f(k,c,c)i(24(%[b]),a)d(l,a,j)d(s,a,k)f(g,e,e)f(h,c,c)i(28(%[b]),a)d(s,a,g)d(4(%[w]),a,h)f(j,e,e)f(k,c,c)f(g,e,e)f(h,c,c)d(%[ni],c,g)d(%[ni],e,h)d(%[p],g,g)d(%[p],h,h)f(g,c,c)f(h,e,e)u($32,c,c)t($0xaa,e,c,c)v(%[p2],c,g)"vpminud %[t0], %[even], %[even]"
asm(Â(%[bi],%[Ŧ],Ò,%[Ȏ],Í,%[t0],%[t1],Ï,%[t2],%[t3],12(%[w]),16(%[w]),20(%[w]),24(%[w]),28(%[w]),32(%[w]),8(%[w]),Î,Ó,Ô)
#undef Â
:[Ŧ]"=&x"(Ŧ),[Ȏ]"=&x"(Ȏ),[bi]"=&x"(bi),[t0]"=&x"(t0),[t1]"=&x"(t1),[t2]"=&x"(t2),[t3]"=&x"(t3):[w]"r"(Ļ.ɽ),[b]"r"(b),[ni]"x"(ni),[p]"x"(p),[p2]"x"(p2),"m"(*õ Ġ(*)[17]>(Ļ.ɽ)),"m"(*õ Ġ(*)ɚ>(b)))Ū Ŧ;}ƛ Ǆ{Ǒ Ɔ,Ƚ=Ś;М;ș{Ć(ǉ þ ų,a+=32){Ù Τ w(ķ,Ў j);Ѻ(f,w);
#pragma GCC unroll 4
Ӏ Š(ύ t,ů(f ѯ));}}};ƛ Ɉ{Ǒ Ś,Ƚ=Ɔ;Ɠ ô;Ɠ in;ș{Ɠ ʝ=in+32*ù ø(ǉ þ ų,a+=32,ʝ+=32){Ù Τ w(ô,Ў j);ϥ={Ō(ʝ),Ō(ʝ+8),Ō(ʝ ү),Ō(ʝ+24)};υ(f,w)ø(ý Ń)Š(ύ t,f ѯ);}}};ƛ Ȥ{Ǒ Ɔ,Ƚ=Ɔ;М;Ɠ ô;Ҡ;Õ origin Ē ĉ Ĉ ѐ(һ,Õ g,ʅ(&Ļ)ɛ)Ù{Ѻ(f,Τ(ķ,g));
#pragma GCC unroll 4
Ӏ f ѯ=ů(f ѯ);ʿ(Ļ,f,ķ,g);}ĉ Ĉ finish(Ң,Õ g,Ù ʅ(&Ļ)ɛ)Ù{ϥ;
#pragma GCC unroll 4
Ӏ f ѯ=Ǽ(Ļ ѯ,b Ʀ(4*g+t-origin));υ(f,Τ(ô,g))ø(ý Ń)Š(ύ t,f ѯ);}ș{ɥ ɛ;ѐ(a,ù,Ļ Ŭ)ø(ǉ þ ų,a+=32){if(j+1<þ)ѐ(a+32,Ў j+1,Ļ[(j+1)&1]);finish(a,Ў j,Ļ[j&1]);}}};ƛ ȗ{Ǒ Ś,Ƚ=Ɔ;Ȥ Ŏ;һ;ĉ Ĉ ѐ(Õ g,ʅ(&Ļ)ɛ)Ù{Ы(Ļ,f,Ŏ.ķ,g);}Ǣ{ɥ ɛ;ȁ{ɕ;Ŏ.finish(į,Ў j,Ļ[j&1]);}}};Ą<Õ K>ƛ ϙ{Ǒ Ś,Ƚ=Ɔ;ǖ ȗ,K>Ơ;Ĉ ѐ(Õ g,ʅ(&Ļ)[K]ɛ)Ù{Ć(ѿ K ȝ)Ơ ƫ.ѐ(g,Ļ ƫ);}Ǣ{ɥ[K]ɛ;ȁ{ɕ ȡ g=Ў j;ϥ;
#pragma GCC unroll 4
Κ{f ѯ=Ǽ(Ļ[j&1]Ŭ ѯ,Ơ Ŭ.Ŏ.b Ʀ(4*g+t-Ơ Ŭ.Ŏ.origin))ø(ʶ K ȝ)f ѯ ӑ(ƪ(f ѯ,Ǽ(Ļ[j&1]ƫ ѯ,Ơ ƫ.Ŏ.b Ʀ(4*g+t-Ơ ƫ.Ŏ.origin))));}υ(f,Τ(Ơ Ŭ.Ŏ.ô,g));
#pragma GCC unroll 4
Κ ԁ}}};ƛ ForwardProductBottom{Ǒ Ɔ,Ƚ=Ś;Ȥ Ŏ;ș{Œ ν(kR);ɥ ɛ;Ŏ.ѐ(a,ù,Ļ Ŭ)ø(ǉ þ ų,a+=32){if(j+1<þ)Ŏ.ѐ(a+32,Ў j+1,Ļ[(j+1)&1]);Ҡ=Ŏ.b Ʀ(4*(Ў j)-Ŏ.origin);
#pragma GCC unroll 4
Ӏ Š(ύ t,ń(ŭ(Ǽ(Ļ[j&1]ѯ,b ϝ),ν),kP));}}};Ő Ƭ>Ũ Ǉ{Ⱥ Ǉ(М,Ɠ ô,Ϩ):r_(ķ),ir_(ô),bottom_(b9){}Ĉ Ϙ(Ң,Ԇ,Ȕ)Ù{if(nv<=kTile)Ú tile(a,nv,k);ƾ 4;ʠ ı(a,h,k)Ȳ δ Ĺ(Ƭ::Ƚ)ć(a,h,k);}ċ æ ƍ radix8(Ԇ){Ú nv/2>kTile;}Ĉ Ԍ(Ң,Ԇ,Ȕ)Ù{ƾ 4 Ȳ δ Ĺ(Ƭ::Ƚ)ć(a,h,k);}Ⱦ ċ ɿ kTile=256;Ĉ tile(Ң,Ԇ,Ȕ)Ù{ʠ{Ć(Ϫ=nv/4;h>=16;h/=4)ԗ ı(ύ j,h,g)ĺ(nv>=16)Ƥ forward_h4(И(a),nv/16,r_,k*(nv/16));}bottom_(a,nv/4,k*(nv/4))Ѓ(Ƭ::Ƚ){if(nv>=16)Ƥ inverse_h4(И(a),nv/16,ir_,k*(nv/16))ø(Ϫ=16;h<nv;h*=4)ԗ ć(ύ j,h,g);}}ċ ҳ И(Ң){Ú Ț(a);}Ĉ ı(Ң,Ϫ,Ȕ)Ù{if(k ζ)Ú ì Ƥ forward_identity(И(a),h,r_);ì Ƥ ı(И(a),h,r_+φ(k),r_+φ(Ӵ));}Ĉ ć(Ң,Ϫ,Ȕ)Ù{if(k ζ)Ú ì Ƥ inverse_identity(И(a),h,ir_);ì Ƥ ć(И(a),h,ir_+φ(k),ir_+φ(Ӵ));}Ɠ r_,*ir_;Ƭ bottom_;};Ũ Ɏ{Ⱥ Ɏ(Ɠ in,Õ ä,Õ ƌ):in_(in),shift_(ƌ),end_(ƌ+ä),first_((ƌ+7)/8),ϳ(ƌ/8){Ȑ=end_/8>first_?end_/8-first_:0;Х=(end_+7)/8-ϳ;}ĭ ŕ()(Õ v)Ù{if(v-first_<Ȑ)Ú ɂ(õ Ģ*>(in_+(8*v-shift_)))ĺ(v-ϳ>=Х)Ú đ Ū edge(in_,shift_,end_,v);}ƍ in_place(Ɠ į)Ù{Ú in_==ѓ shift_;}Ĉ zero_outside(ȧ,Ї,п)Ù{ģ begin=Ø clamp(shift_,lo,hi),end=Ø clamp(end_,begin,hi);ż(ѓ lo,ѓ
 begin,0);ż(ѓ end,ѓ hi,0);}ƍ within(Ї,п)Ù{Ú Х ζ||(ϳ>=lo&&ϳ+Х<=hi);}Ⱦ Ɩ ċ ĭ edge(Ɠ in,Õ ƌ,Õ end,Õ v){љ x ɚ={}ø(ɵ=8*v Ͼ 8*v+8 Į)if(i>=ƌ&&i<end)x[i-8*v]=in[i-ƌ]Ū Ō(x);}Ɠ in_;Õ shift_,end_;Õ first_,Ȑ;Õ ϳ,Х;};Ő Input>ˮ forward_top4_body(Input in,ҳ į,Ϫ,М){Œ z(ķ ƹ,ķ[9])κ p=ɟ ĺ(in.within(0,2*h)){Р{Ģ a=in(j),b=in(j+h),zb Ҝ(b,z);į ˉ ƪ(a,b),į ǹ=Ǚ(a,b),į ɋ=ƪ(a,zb),į Ɋ=Ǚ(a,zb);}Ú;}if(in.within(2*h,4*h)){Р{Ģ c=in(j ʳ h),d=in(j+3*h)κ mc=é(p,c),zmd Ҝ(é(p,d),z);į ˉ ƪ(c,d),į ǹ=Ǚ(c,d),į ɋ=ƪ(mc,zmd),į Ɋ=Ǚ(mc,zmd);}Ú;}Р{Ģ a=in(j),b=in(j+h),c=in(j ʳ h),d=in(j+3*h)κ ac=ƪ(a,c),amc=é(ƪ(a,p),c)κ bd=ƪ(b,d),zbmd Ҝ(é(ƪ(b,p),d),z);į ˉ ƪ(ac,bd),į ǹ=Ǚ(ac,bd);į ɋ=ƪ(amc,zbmd),į Ɋ=Ǚ(amc,zbmd);}}Ő Input>ˮ forward_top2_body(Input in,ҳ į,Ϫ){Ģ p=ɟ ĺ(in.within(0,h)){Р į ˉ į ǹ=in(j)а}if(in.within(h,2*h)){Р{Ģ v=in(j+h);į ˉ v,į ǹ=é(p,v);}Ú;}Р{Ģ u=in(j),v=in(j+h);į ˉ ƪ(u,v),į ǹ=é(ƪ(u,p),v);}}Ɩ í forward_top4(Ɏ in,ҳ į,Ϫ,М){forward_top4_body(in,į,h,ķ);}Ɩ í forward_top2(Ɏ in,ҳ į,Ϫ){forward_top2_body(in,į,h);}ò ƍ forward_top8(Ù Ɏ&,Ņ
,Õ,Ɠ){Ú Ś;}Ϥ radix2_halves(Ù Ǉ<Ƭ>&Ŝ,Ù Ɏ&in,Ң,Ԇ,М){ƾ 2;ʠ{if(Ŝ.radix8(nv)&&forward_top8(in,a,nv,ķ)){Ŝ.Ԍ(a,h,0);Ŝ.Ԍ(ύ h,h,1)а}forward_top2(in,Ț(a),h);}ő(a,h,0);ő(ύ h,h,1);}í inverse_top4(ҳ f,Ϫ,Π,Ɠ ô,Œ&s){if(ǂ==Ȉ)Ú á inverse_radix4(f,h,ô,s)ƣ z(ô ƹ,ô[9])Ǎ ŧ Є ʋ{Ģ p0=f Ɓ,p1=f ǹ,p2=f ɋ,p3=f Ɋ κ ab ӑ(ƪ(p0,p1)),cd ӑ(ƪ(p2,p3))κ amb ӑ(Ǚ(p0,p1)),cmd Ҝ(Ǚ(p2,p3),z)ĺ(ǂ==ƨ){f ˉ ŧ(ϓ),f ǹ=ŧ(ϒ);}ŝ{f ɋ=ŧ(Ќ),f Ɋ=ŧ(ϑ);}}}í Ӧ(ҳ f,Ϫ,Π,Ö ŧ){if(ǂ==Ȉ){љ s[16]={};s ƹ=ŧ;s[9]=á ť(ŧ)Ū ì Ƥ scale_radix2(f,h,s);}Œ s(ŧ)ʋ{if(ǂ==ƨ)f ˉ ń(ŭ(ƪ(f Ɓ,f ǹ),s),kP)і f ǹ=ń(ŭ(Ǚ(f Ɓ,f ǹ),s),kP);}}}Ũ Č{Ⱥ ċ æ ý kMinLog=6,Ҕ=ì Ҕ;ċ ɿ č(ý Δ){Ú ß(2*á ϛ(Δ));}Č(ě&ŋ,ý Δ):lg_max_(Δ){if(Δ<kMinLog||Δ>Ҕ)ǩ ȡ n8=á ϛ(Δ),entries=(ǻ Δ)Ș;}ý Δ()Ù{Ú lg_max_;}М()Ù{Ú Ə;}Ɠ ô()Ù{Ú ō;}Ĉ ı(Ğ in,Õ ƌ,ɐ)Ù{run(į,Ο,â Ǆ{Ə},0,Ȉ);}Ĉ ı(ҽ)Ù{ı(a,0,a);}Ĉ ɇ(Ğ in,Õ ƌ,ɐ)Ù{ƞ;ʐ(2*ǲ)ĩ Ɏ src=Ο ĩ â Ҟ}Ĉ ć(Ğ in,ɐ,Π=Ȉ)Ù{ˊ(ǲ,Ś);run(į,â Ɏ(Ƿ,0,0),â Ɉ{ō,in à},ŧ,ǂ);}Ĉ ć(ҽ,Π=Ȉ)Ù{ć(a,a,ǂ);}Ĉ Ŷ(Ğ in,Õ ƌ,ɐ,Ğ b,Π=Ȉ,Ö c=1)Ù{Ę ā ã;Ġ ŧ=Đ(inverse_scale(ǲ,Ɔ),c);run(į,Ο,â Ȥ{Ə,ō,b à},ŧ,ǂ);}Ĉ Ŷ(ҽ,Ğ b)
Ù{Ŷ(a,0,a,b);}Ĉ inverse_product(Ğ a,Ğ b,ɐ,Π=Ȉ)Ù{ˊ(ǲ,Ɔ)ĩ â Ȥ Ŏ{Ə,ō,b à};run(į,â Ɏ(Ƿ,0,0),â ȗ{Ŏ,ʰ},ŧ,ǂ);}ƛ Pair{Ğ a,b;};Ĉ Ŗ(Ԅ Pair>ǜ,ɐ,Π=Ȉ)Ù{switch(ǜ Û){case 1:Ú inverse_product(ǜ Ŭ.a,ǜ Ŭ.b,į,ǂ)ϼ 2:Ú inverse_products<2>(ǜ,į,ǂ)ϼ 3:Ú inverse_products<3>(ǜ,į,ǂ);default:ǩ;}}Ϥ Ɇ(Ϩ,ɐ,Π=Ȉ)Ù{ˊ(ǲ,Ɔ);run(į,â Ɏ(Ƿ,0,0),b9,ŧ,ǂ);}Ĉ forward_product(Ğ in,Õ ƌ,ɐ,Ğ b)Ù{Ù â Ȥ Ŏ{Ə,ō,b à};run(į,Ο,â ForwardProductBottom{Ŏ},0,Ȉ);}Ĉ Ź(ҽ,Ğ b)Ù{ƕ(ʰ,ʰ,b à,Ư,Ѯ(â ĭ,â ĭ p)Ü{Ú p;});}Ĉ multiply_add(ĝ c,Ğ a,Ğ b)Ù{ƕ(c à,ʰ,b à,Ư,Ѯ(â ĭ c,â ĭ p)Ü{Ú â ƪ(c,p);});}Ⱦ Ö inverse_scale(ǀ,ƍ Ŕ)Ù{ʐ(n)Ū â ȋ[Ŕ][Ī(n)];}Ĉ ʐ(ǀ)Ù{ȟ lg=Ī(n)ĺ(!ϩ(n)||lg<kMinLog||lg>lg_max_)ǩ;}Ą<Õ K>Ĉ inverse_products(Ԅ Pair>ǜ,ɐ,Π)Ù{ˊ(ǲ,Ɔ);â ϙ<K>b9{}ø(ѿ K ȝ)b9.Ơ ђ{{Ə,ō,ǜ ƫ.b à},ǜ ƫ.ʰ};run(į,â Ɏ(Ƿ,0,0),b9,ŧ,ǂ);}ċ â Ɏ source(Ğ in,Õ ƌ,ǀ){if(ƌ>n||in Û>n-ƌ)ǩ Ū â Ɏ(in à,in Û,ƌ);}Ϥ run(ҽ,Ù â Ɏ&in,Ϩ,Ö ŧ,Π)Ù{ƞ;ʐ(Ư)ĩ â Ǉ Ŝ(Ə,ō,b9)Ϊ{Ι(v,h,ǂ,ō,ƭ(ŧ));}ŝ{ƾ 2;radix2_halves(Ŝ,in,ʰ,nv,Ə)Ѓ(Ƭ::Ƚ)Ӧ(v,h,ǂ,ŧ);}}Ő Combine>Ĉ ƕ(ȧ,һ,Ҡ,ǀ,Combine җ)Ù{ƞ;ʐ(n)ƣ ν(kR);ɥ;Ε(Ļ Ŭ,Ō(a),Ϛ(Ə,0))ø(ӛ n
/8 ė p){if(p+1<n/8)Ε(Ļ[(p+1)&1],Ō(ύ p+8),Ϛ(Ə,p+1))κ Ŏ=ń(ŭ(Ǽ(Ļ[p&1],b Ʀ p),ν),kP);Š(į Ʀ p,ů(җ(Ō(į Ʀ p),Ŏ)));}}ý lg_max_;Ņ Ə,*ō;};}ā ē{Ϧ{Ȅ Ž(Ɠ p){Ú ɂ(õ Ģ*>(p));}í ū(Ņ p,ɍ){Ψ(Ț(p),x);}Ȅ Ŕ(ĭ a,ĭ b){ȱ,p=ɟ;н=Þ(a,b);ĭ Ȏ=І;ĥ Ú ǝ;}Ũ Indices{Ϳ Indices(Ȕ){љ x ҹ(Ö i=0;i<8 Į)x Ƈ Ö((ŉ(k+i)<<32)Ұ);value_ ӷ(x);}ĭ ĳ()Ù{Ú value_;}Ĉ next(){value_=ń(ƪ(value_,ĕ(kStep)),kP);}Ⱦ ċ ӄ kStep=Ö((ŉ(8)<<32)Ұ);ĭ value_;};ԅ scalar_inverse(Ö x){Ú á Ǹ(x,ӕ);}Ő A>Ĉ ɗ(Õ ù,ɐ,Ù A&a,Ö Ÿ=1){ɿ ɤ=32 ȡ n=ǲ ĺ(n ζ)Ú ȡ ǽ=(n+ɤ-1)/ɤ,Ȣ=ɤ*(ǽ-1)κ one=ĕ(1),ɯ=ĕ(ɤ);ɍ ɛ ø(Ҹ)x ӓ=ň(ĕ(Ö(ù Ʀ v)),ɜ)Ǎ valid=[n](Õ at)Ü{Ģ т=ň(ĕ(Ö(at)),ɜ)Ū τ(ĕ(Ö(n)),т);}Ǎ masked=Ѯ(ĭ ĳ,Õ at)Ü{Ú at+8<=n?ĳ:_mm256_blendv_epi8(one,ĳ,valid(at));};ĭ ʜ ɛ ø(Ҹ)ʜ ӓ=masked(x ӓ,8*v)ø(Õ b=ɤ;b<n;b+=ɤ){Ć(Ҹ){ū(ԃ b-ɤ Ʀ v,ʜ ӓ);x ӓ=ň(x ӓ,ɯ);ʜ ӓ=Ŕ(ʜ ӓ,masked(x ӓ,b Ʀ v));}}љ o4[ɤ],ć[ɤ]ø(Ҹ)Š(o4 Ʀ v,ů(ʜ ӓ));Ö m3=1 ø(Õ l Ē l<ɤ ė l){ć[l]=m3;m3=ʀ(m3,o4[l]);}m3=ʀ(scalar_inverse(m3),ʀ(kR,Ÿ))ø(Õ l=ɤ;l-->0;){ć[l]=ʀ(ć[l],m3);m3=ʀ(m3,o4[l]);}ĭ ё ɛ ø(Ҹ)ё ӓ ӷ(ć Ʀ v)Ǎ put=Ѯ(Õ at,ĭ l4)Ü{Ģ q=ů(Ŕ(l4,a(at
)))ĺ(at+8<=n)Ú ū(ԃ at,q)Ӑ(õ ý*>(ԃ at),valid(at),q);}ø(Õ b=Ȣ;b>0;b-=ɤ){Ć(Ҹ){ģ at=b Ʀ v ĺ(at<n)put(at,Ŕ(Ž(ԃ at-ɤ),ё ӓ));ё ӓ=Ŕ(ё ӓ,masked(x ӓ,at));x ӓ=é(x ӓ,ɯ);}}Ć(Ҹ)if(8*Õ(v)<n)put(8*v,ё ӓ);}}í Ǟ(О,ĝ d){ѻ n=d Û,σ(d Ȝ,ů(Ŕ(Ž(f Ȝ+1),ȯ.ĳ())))ø(ɵ=full ț)d Ƈ i+1<Ǳ?ʀ(Ö(i+1),f Υ):0;}í ɗ(Ğ a,Õ ù,ĝ q){if(Ư!=Ѹ||ù ζ||Ў Ư>ϵ)ǩ ȡ n=Ư;â ɗ(ù,q,[a,n](Õ at)Ü{if(at+8<=n)Ú â Ž(a Ʒ at);љ ɼ ɚ={};ȩ(a ʷ at,a Ƃ,ɼ)Ū â Ō(ɼ);});}}ā ē{Ϧ{ò ɿ kInverseBase=32;í ѧ(О,ĝ g){Ę ʀ;Ġ inv0=á Ǹ(f Ŭ,ӕ);g Г inv0 ø(ɵ=1 Ͼ ȸ Į){ŉ ʂ ƒ(ʍ=1;j<=Ħ(i,Ǳ-1)ų){ʂ+=ŉ(f Ɓ)*g[i-j]Ұ;}g Ƈ Đ(Ö(ʂ Ұ?Ͻ ʂ Ұ:0),inv0);}}}ò ý inverse_log(ǀ){Ú Ŋ(ʵ,ý(ʌ(ʘ(n,2)-1)));}Ί ȵ(ǀ){Ú 2 ӎ(ǻ inverse_log(n));}í inverse_step(ƶ,О,Ğ g,ĝ to,ĝ gt,ĝ Ǡ){ģ k=ȸ Ȇ(g,0,gt);t.Ŷ(f ȑ(Ħ(Ӵ,Ǳ)),0,Ǡ,gt,Ƨ);t.Ŷ(Ǡ Ė(k),k,Ǡ,gt,Ƨ,â Ͻ 1)ĺ(to à!=Ǡ Ʒ k)ĵ(Ǡ ʊ(k),to Û,to ü);}í ć(ƶ,О,ĝ g,Ѕ){ģ n=ȸ;Ȕ Ш(n,â kInverseBase);â ѧ(f,g ȑ(k))ø(;k<n;k*=2){ģ ǯ=Ӵ;inverse_step(t,f,g ȑ(k),g Ė(k,Ħ(k,n-k)),ĸ ȑ(ǯ),Ԃ(ß(ǯ),ǯ));}}}ā ē{Ϧ{ò ɿ kLogBase=64;ò ɿ kLogBlocks=4;í log_direct(О,ĝ g){Ę ʀ;Ö a[kLogBase]={},ё[kLogBase]=
{0,1},kg[kLogBase]={}Ԗ(f ü,Ħ(Ǳ,ȸ),a)Ӻ Ŭ ƒ(ɵ=1 Ͼ ȸ Į){ŉ ʂ=Đ(Ö(i),ɨ)ø(ʶ i ȝ)ʂ+=Ͻ ŉ(kg ƫ)*a[i-k]Ұ;kg Ƈ Ö(ʂ Ұ);g Ƈ Đ(kg Ĕ,ё Ĕ);}}Ί log_block(ǀ){Ú ʘ(32,ɬ((ԕ+kLogBlocks-1)/kLogBlocks));}Ί log_blocks(ǀ){Ú(ԕ+ɣ-1)/ɣ;}í subtract_from_derivative(О,Õ ù,Ƞ,ȧ,Õ ä){ģ g6=Ǳ>ҵ?Ħ(þ,Ǳ-ù-1):0,ҫ{Ģ d=ů(Ŕ(Ž(f Ʒ ҵ+c),ȯ.ĳ()))Ԁ}Ć(Õ c=full;c<þ ė c){Ġ d=c<g6?ʀ(Ö(ҵ+c),f[ҵ+c]):0;į[c]=(d+Ͻ į[ä+c])Ұ;}}ƛ ToeplitzBottom{Ǒ Ś,Ƚ=Ɔ;М,*ô,*w1,*w2,*q0,*q1;Ņ w3;ĉ Ĉ ѐ(Õ g,ʅ(&Ļ)Ѐ ɛ)Ù{ĭ ʂ ɛ,d1 ɛ,d3 ɛ;
#pragma GCC unroll 4
Κ{ģ leaf=8*(4*g+t)κ v2 ӷ(w2+leaf);ʂ ѯ=ń(ƪ(Ō(q0+leaf),Ō(q1+leaf)),kP);d1 ѯ=ɸ(Ō(w1+leaf),v2);d3 ѯ=ɸ(Ō(w3+leaf),v2);}ʿ(Ļ Ŭ,ʂ,ķ,g);ʿ(Ļ ƹ,d1,ķ,g);ʿ(Ļ ǥ,d3,ķ,g);}Ǣ{ɥ Ѐ ɛ;ȁ{ɕ ȡ g=Ў j Ǎ&w=Ļ[j&1];ϥ;
#pragma GCC unroll 4
Κ{ģ leaf=8*(4*g+t)κ m1=Ǽ(w Ŭ ѯ,w2+leaf);f ѯ ӑ(ƪ(m1,Ǽ(w ƹ ѯ,q1+leaf)));Š(w3+leaf,ʗ(ƪ(m1,Ǽ(w ǥ ѯ,q0+leaf))));}υ(f,Τ(ô,g));
#pragma GCC unroll 4
Κ ԁ}}};ƛ SideProductBottom{Ǒ Ś,Ƚ=Ɔ;ȗ term;Ɠ side;Ǣ{ɥ ɛ;term.ȁ{if(j+1<þ)term.ѐ(Ў j+1,Ļ[(j+1)&1])ȡ g=Ў j;ϥ;
#pragma GCC unroll 4
Κ{ģ leaf=8*(4*g+t);f ѯ ӑ(ƪ(Ǽ(Ļ[j&1]ѯ,term.Ŏ.b+(leaf-8*term.Ŏ.origin)),Ō(side+leaf)));}υ(f,Τ(term.Ŏ.ô,g));
#pragma GCC unroll 4
Κ ԁ}}};Ί log_buffers(ǀ){Ú ʘ(log_blocks(n)+2,3);}ò ī mint_scalar(Ě x)Щ Ȅ mint_load(ˢ f,Õ at){if(at+8<=Ǳ){ĭ ĳ;Ǵ(&ĳ,f Ʒ at,ƚ(ĳ))Ū ń(ĳ,kP);}Ť ī Ƴ ɚ={}ø(Ů 8&&at+i<Ǳ Į)Ƴ Ƈ mint_scalar(f[at+i])Ū Ō(Ƴ);}Ũ MintSource{Ϳ MintSource(ˢ f):f_(f),full_(Ǳ/8),Х((Ǳ+7)/8){}ĭ ŕ()(Õ v)Ù{if(v<full_){ĭ ĳ;Ǵ(&ĳ,f_ ʥ v,ƚ(ĳ))Ū ń(ĳ,kP);}if(v>=Х)Ú đ Ū edge(f_,8*v);}ƍ within(Ї,п)Ù{Ú Х ζ||(lo ζ&&Х<=hi);}Ⱦ Ɩ ċ ĭ edge(ˢ f,Õ at){Ú mint_load(f,at);}ˢ f_;Õ full_,Х;};Ɩ í mint_forward_top4(MintSource in,ҳ į,Ϫ,М){forward_top4_body(in,į,h,ķ);}Ɩ í mint_forward_top2(MintSource in,ҳ į,Ϫ){forward_top2_body(in,į,h);}í mint_forward(ƶ,ˢ f,ɐ){Ù MintSource in(f)ĩ â Ǉ Ŝ(Ȱ,ɧ,Ǆ{Ȱ})ȡ Ӿ ǡ{ƾ 4;mint_forward_top4(in,v,h,Ȱ)ø(Õ q Ē q<4 ė q)ő(į ʥ q*h,h,q);}ŝ{ƾ 2;mint_forward_top2(in,v,h);ő(į à,h,0);ő(į ʥ h,h,1);}}í mint_derivative(ˢ f,ĝ d){ѻ n=d Û,σ(d Ȝ,ů(Ŕ(mint_load(f,i+1),ȯ.ĳ())))ø(ɵ=full ț)d Ƈ i+1<Ǳ?ʀ(Ö(i+1),mint_scalar(f Υ)):0;}í mint_subtract_from_derivative(ˢ f,Õ ù,Ƞ,ȧ,Õ ä){ģ g6=Ǳ>ҵ?Ħ(þ,Ǳ-ù-1):0,ҫ{Ģ d=ů(Ŕ(mint_load(f,ҵ+c),ȯ.ĳ()))Ԁ}Ć(Õ c=full
;c<þ ė c){Ġ d=c<g6?ʀ(Ö(ҵ+c),mint_scalar(f[ҵ+c])):0;į[c]=(d+Ͻ į[ä+c])Ұ;}}ƛ NoLogTransform{Ĉ ŕ()(Õ,Ğ)Ù{}};Ő Input,Ũ Sink,Ũ ғ>ˮ log_derivative_impl(ƶ,Input f,ǀ,Ѕ,Ğ ʜ,Ù Sink&sink,Ù ғ&Ǩ={}){ģ k=ɣ,ǯ=Ӵ,ǽ=log_blocks(n)Ǎ buffer=[&ĸ,ǯ](ɵ)Ü{Ú Ԃ(i ӎ(ǯ),ǯ);}ǫ ht=buffer(0),q0=buffer(ǽ)Ǎ Ļ=[&buffer](Õ t)Ü{Ú buffer(t);};ć(t,ʜ,ht ȑ(k),Ԃ(ß(ǯ),ȵ(k)))Ȇ(ht ȑ(k),0,ht)ø(Õ s=1;s<ǽ ė s){ģ ʝ Ш(Ǳ,(s-1)*k)Ѓ(Ĥ Input,ğ ǣ>>)mint_forward(t,f Ė(ʝ,Ħ(Ǳ-ʝ,ǯ)),Ļ(s))і t.ı(f Ė(ʝ,Ħ(Ǳ-ʝ,ǯ)),0,Ļ(s));}Ć(ǉ ǽ ų){ģ ù=j*k,þ Ш(k,ԕ-ù)ǫ Ǡ=j ζ?q0:buffer(ǽ+1)ĺ(j ζ){Ĺ(Ĥ Input,ğ ǣ>>)mint_derivative(f ȑ(Ħ(Ǳ,þ+1)),Ǡ ȑ(þ))і Ǟ(f ȑ(Ħ(Ǳ,þ+1)),Ǡ ȑ(þ));}ŝ{if(j у){Ɖ(Ļ(1),q0,Ǡ,Ƨ);}м(ǽ==3){Ù ɝ ǜ ǥ={{Ļ(2),q0},{Ļ(1),Ǡ}};t.Ŗ(ǜ,Ǡ,Ƨ);}м(j==2){t.Ɇ(ToeplitzBottom{Ȱ,ɧ,Ļ(1)à,Ļ(2)à,q0 à,Ǡ à,Ļ(3)à},Ǡ,Ƨ);}ŝ{Ù ȗ Ŏ{{Ȱ,ɧ,Ǡ à},Ļ(1)à};t.Ɇ(SideProductBottom{Ŏ,Ļ(3)à},Ǡ,Ƨ);}Ĺ(Ĥ Input,ğ ǣ>>)mint_subtract_from_derivative(f,ù,þ,Ǡ à,k)і subtract_from_derivative(f,ù,þ,Ǡ à,k);}t.Ŷ(Ǡ ȑ(þ),0,Ǡ,ht,ƨ);sink(ù,Ğ(Ǡ ȑ(þ)))ĺ(j+1<ǽ){t.ı(Ǡ ȑ(k),0,Ǡ);Ǩ(j,Ğ(Ǡ));}}}Ő Sink,Ũ ғ=NoLogTransform
>ˮ Γ(ƶ,О,ǀ,Ѕ,Ù Sink&sink,Ù ғ&Ǩ={}){log_derivative_impl(t,f,n,ĸ,f,sink,Ǩ);}Ő Sink,Ũ ғ=NoLogTransform>ˮ mint_log_derivative(ƶ,ˢ f,ǀ,Ѕ,Ğ ʜ,Ù Sink&sink,Ù ғ&Ǩ={}){log_derivative_impl(t,f,n,ĸ,ʜ,sink,Ǩ);}ò ý log_derivative_log(ǀ){Ú Ī(2*ɣ);}Ί log_derivative_scratch(ǀ){Ú log_buffers(n)ӎ(2*ɣ);}}ò ý log_log(ǀ){Ú n<=â kLogBase?ʵ:Ī(2*â ɣ);}Ί log_scratch(ǀ){Ú n<=â kLogBase?0:â log_buffers(n)ӎ(2*â ɣ);}í log(ƶ,О,ĝ g,Ѕ){ѻ n=ȸ ĺ(n<=kLogBase)Ú log_direct(f,g);Γ(t,f,n,ĸ,[g](Õ ù,Ğ q)Ü{â ɗ(ҵ,g Ė(ҵ,Ѹ),[q](ɵ)Ü{Ú Ō(q Ȝ);});});g Ŭ Ē}}ā ē{í native_log(ǣ*in,Õ ʭ,л,ǀ,ƍ g7){ƙ(į,n);į θ(0)ȡ c Ш(ʭ,n)ȉ lg=â log_derivative_log(n)ĩ ƍ ӥ=n>=262144 Ǎ k7=ӥ?Ħ(c,â ɣ):c й(ɠ(k7)Я(log_scratch(n)))Ǭ f ф(k7),ĸ ф(log_scratch(n))ø(Ů k7 Į){Ù ī v=Ӫ;f Ƈ Ħ(v,v-Ƀ);}ʦ sink=[&į,g7,n](Õ ù,Ğ q)Ü{if(g7){ł*Ǡ=const_cast<Ņ>(q à)+â ɣ;â ɗ(ҵ,{Ǡ,Ѹ},[q](ɵ)Ü{Ú â Ō(q Ȝ);},á kR);}ʦ*Ƴ=g7?q Ʒ â ɣ:q à ĺ(g7)ž(į,Ƴ,Ѹ)і append_fps_words ə(į,Ƴ,Ѹ);}ĺ(ӥ)â mint_log_derivative(t,{in,c},n,ĸ,f,sink)і â Γ(t,f,n,ĸ,sink);}}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ӂ{ā ē{Ϧ{ƛ ƻ Э}}ā ē{Ϧ{ò ɿ ϲ=64;í exp_direct(Ğ q,ĝ g){Ę ʀ;Ö ё[ϲ]={0,1}Ӻ Г 1 ø(ɵ=1 Ͼ ȸ Į){ŉ ʂ ƒ(Ȕ=1;k<=i ȝ)ʂ+=ŉ(q ƫ)*g[i-k]Ұ;g Ƈ Đ(Ö(ʂ Ұ),ё Ĕ);}}í multiply_by_index(Ğ a,Õ ù,ɐ){ģ n=ǲ,full=n/8*8 Ч(ù)ø(Ů full Ɲ,ȯ.next())ū(į Ȝ,ů(Ŕ(Ž(a Ȝ),ȯ.ĳ())))ø(ɵ=full ț)į Ƈ ʀ(Ö((Ў i)Ұ),ɨ);}Ί exp_length(ǀ){Ú ǻ Ŋ(ʵ+1,ý(ʌ(ʘ(n,2)-1)));}}ò ý exp_log(ǀ){Ú Ī(â γ)-1;}Ϧ{ƛ QTransforms{Ğ ʗ,block2;};ƛ NoQTransforms{};Ϥ product_to(ƶ,ɐ,Ù Ɏ&in,Ϩ,Ö ŧ,Π,ĝ to){ģ Ӿ ĩ Ǉ Ŝ(Ȱ,ɧ,b9)ƣ s(ŧ)Ǎ put=[to,&s](ʍ,ɍ)Ü{Ģ y=ń(ŭ(x,s),kP)ĺ(8*j+8<=to Û)Ú ū(to ʥ j,y)ĺ(8*j>=to Û)Ú κ т=ɜ κ mask=τ(ĕ(Ö(to Û-8*j)),т)Ӑ(õ ý*>(to ʥ j),mask,y);}ĩ ƍ lower=ǂ==ƨ ǡ{ƾ 4;ʠ forward_top4(in,v,h,Ȱ)ø(Õ q Ē q<4 ė q)ő(į ʥ q*h,h,q)ƣ z(ɧ ƹ,ɧ[9])ʋ{Ģ ab ӑ(ƪ(v Ɓ,Ӌ)),cd ӑ(ƪ(v ɋ,v Ɋ))κ amb ӑ(Ǚ(v Ɓ,Ӌ)),cmd Ҝ(Ǚ(v ɋ,v Ɋ),z);put(j,lower?ϓ:Ќ);put(j+h,lower?ϒ:ϑ);}}ŝ{ƾ 2;radix2_halves(Ŝ,in,į à,nv,Ȱ)ʋ put(j,lower?ƪ(v Ɓ,Ӌ):Ǚ(v Ɓ,Ӌ));}}Ő Given>ˮ exp_last_step(ƶ,ĝ lo,ĝ g,Ό,ƍ ù,ĝ gt,Ğ ht,ĝ w,ĝ Ǡ,Ù Given&e7){ģ n=ȸ,Ɵ=m/2,ɼ=n-m,þ Ш(ɼ,Ɵ)ĩ ƍ ʓ=ɼ>Ɵ ǫ g0t=gt ʴ,glt=gt ч,own=w ʴ,r=w ч;Ğ qt=own
,q2t=lo;ƍ e9=Ś,e8=Ś Ѓ(Ĥ Given,QTransforms>){e9=!e7.ʗ ű,e8=!e7.block2 ű ĺ(e9)qt=e7.ʗ ĺ(e8)q2t=e7.block2;}ʦ q=[g,n](ɵ)Ü{if(i+8<=n)Ú Ž(g Ȝ);љ x ɚ={};ȩ(g ʊ(i),g Ƃ,x)Ū Ō(x);}ĺ(ù)t.ı(g ȑ(Ɵ),0,g0t)Ȇ(g ʴ,0,glt)ĺ(e9){Ɖ(qt,glt,r,ʓ?Ȉ:ƨ);}м(ʓ){t.ı(lo,0,own);Ɖ(own,glt,r);}ŝ{t.Ŷ(lo,0,r,glt,ƨ);}t.ı(r ȑ(Ɵ),0,Ǡ);Ɖ(ht,Ǡ,Ǡ,ƨ);â ɗ(m,r ȑ(þ),Ѯ(ɵ)Ü{Ҽ ń(ƪ(Ō(Ǡ Ȝ),q(m+i)),kP)Ū ƪ(x,é(ɟ,Ō(lo Ȝ)));})ĺ(ʓ&&!e8)t.ı(g Ė(m,Ɵ),0,lo)Ȇ(r ȑ(þ),0,Ǡ);Ġ ŧ=ȋ ƹ[Ī(m)]ĩ Ɏ none(Ƿ,0,0);product_to(t,Ǡ,none,ȗ{{Ȱ,ɧ,Ǡ à},g0t à},ŧ,ƨ,g Ė(m,þ))ĺ(!ʓ)Ú Ȇ(g Ė(m,Ɵ),0,Ǡ)ĩ ɝ ǜ[]={{Ǡ,qt},{q2t,glt}};t.Ŗ(ǜ,Ǡ,Ƨ)Ч(Ɵ)ø(ɵ=Ɵ Ͼ m Ɲ,ȯ.next()){Ģ xg=Ŕ(Ž(g Ȝ),ȯ.ĳ())κ ʂ=ƪ(Ō(r Ȝ),Ō(Ǡ Ȝ));Š(r Ȝ,ů(é(ƪ(ʂ,П),xg)));}t.ı(r Ė(Ɵ,Ɵ),0,own);Ɖ(ht,own,own,ƨ);â ɗ(m+Ɵ,r ȑ(ɼ-Ɵ),Ѯ(ɵ)Ü{Ú ƪ(Ō(own Ȝ),q(m+Ɵ+i));})Ȇ(r ȑ(ɼ-Ɵ),0,own);product_to(t,own,none,ȗ{{Ȱ,ɧ,own à},g0t à},ŧ,ƨ,g Ė(m+Ɵ,ɼ-Ɵ));}Ő Given=NoQTransforms>ˮ exp_newton(ƶ,ĝ lo,ĝ g,Ѕ,Ù Given&e7={}){ģ n=ȸ,ǯ=2*lo Û Ǎ ś=[&ĸ](Õ č)Ü{Ù ĝ s=ĸ ȑ(č);ĸ=Ԃ(ß(č))Ū s;}ǫ h=ś(ǯ/2),gt Ӷ,ht=ś(ǯ/2),w Ӷ;Ό=ϲ;ѧ(g,h ȑ(m/2))Ȇ(h ȑ(m/2),0,ht ʴ)ø(;2*
m<n;m*=2){ģ Ɵ=m/2 ǫ e5=gt ʴ,f6=ht ʴ,o8=w ʴ Ȇ(g ʴ,0,e5);Ɖ(e5,f6,o8,Ƨ);Ġ j0=ʀ(ȋ ƹ[Ī(m)],Ͻ 1);product_to(t,o8,Ɏ(o8 Ʒ Ɵ,Ɵ,Ɵ),Ȥ{Ȱ,ɧ,f6 à},j0,Ƨ,h Ė(Ɵ,Ɵ));t.forward_product(lo ʴ,0,o8,e5);t.ć(o8,gt ч)Ȇ(h ʴ,0,ht ϡ);t.ɇ(gt ч,0,w ч);Ɖ(w ϡ,ht ϡ,w ϡ,ƨ);â ɗ(m,w ч,Ѯ(ɵ)Ü{Ҽ ń(ƪ(Ō(w Ȝ),Ō(lo Ʒ m+i)),kP)Ū ƪ(x,é(ɟ,Ō(lo Ȝ)));});t.ɇ(g ʴ,0,gt ч);product_to(t,w ϡ,Ɏ(w Ʒ m,m,m),Ȥ{Ȱ,ɧ,gt à},ȋ ƹ[Ī(2*m)],Ƨ,g ч);}exp_last_step(t,lo,g,m,m==ϲ,gt,ht,w,h,e7);}Ί exp_newton_scratch(ǀ){ģ ǯ=γ Ū 2 ӎ(ǯ)ʳ ß(ǯ/2);}}Ί exp_scratch(ǀ){Ú ß(â γ/2)+â exp_newton_scratch(n);}í exp(ƶ,О,ĝ g,Ѕ){ѻ n=ȸ,m=γ/2,ä Ш(Ǳ,n),Ԍ Ш(ä,m)ǫ lo=ĸ ʴ;multiply_by_index(f ȑ(Ԍ),0,lo ȑ(Ԍ));ż(lo ʊ(Ԍ),lo Ƃ,0)ĺ(ä>m)multiply_by_index(f Ė(m,ä-m),m,g Ė(m,ä-m))ĺ(n>Ŋ(ä,m))ż(g ʊ(Ŋ(ä,m)),g Ƃ,0);exp_direct(lo,g ȑ(Ħ(n,ϲ)))ĺ(n>ϲ)exp_newton(t,lo,g,Ԃ(ß(m)));}}ā ē{Ϧ{ή kSum>Ĉ times_x(М,һ,Ҡ,Ö e,Ö a9,Ö beta,ɐ){ģ Ȟ=ǲ/8 ƣ ŧ(e)κ m0=ŏ(7,0,1,2,3,4,5,6),Ȏ=ŏ(0,-1,0,-1,0,-1,0,-1);Ġ c9 ǥ={(a9+beta)Ұ,(a9+Ͻ beta)Ұ}ø(ӛ Ȟ;p+=8){Ù ƍ ʓ=p>=Ȟ/2 ĩ я r=entries(ķ,p/16*8)κ pick=p%16?ŏ(4,4,5,5,6,6,7,7
):ŏ(0,0,1,1,2,2,3,3)κ rw=Ľ(r.w,pick),rq=Ľ(r.q,pick)ĩ я w{Ĵ(rw,é(ɟ,rw),0xAA),_mm256_xor_si256(rq,Ȏ)};ĭ y ɚ;
#pragma GCC unroll 8
Ć(Ů 8 Į){ĭ c ӷ(ύ(p+i))Ѓ(kSum)c=ʓ?ɸ(c,Ō(b Ʀ(p+i))):ń(ƪ(c,Ō(b Ʀ(p+i))),kP);y Ƈ ń(ŭ(c,ŧ),kP);}Ģ t01 ǿ(y Ŭ,y ƹ),t23 ǿ(y ǥ,y Ѐ)κ t45 ǿ(y ɛ,y[5]),t67 ǿ(y[6],y[7])κ u03 Ǿ(t01,t23),u47 Ǿ(t45,t67)κ z=ů(ƪ(ŭ(Ł(u03,u47,0x31),w),ĕ(c9[ʓ])))κ zh=Ł(z,z,0x11)κ h2 ɚ={z,Ͳ(z,4),Ͳ(z,8),Ͳ(z,12),zh,Ͳ(zh,4),Ͳ(zh,8),Ͳ(zh,12)};
#pragma GCC unroll 8
Ć(Ů 8 Į)Š(į ʥ(p+i),Ĵ(Ľ(y Ĕ,m0),h2 Ĕ,1));}}}ò ý power_log(ǀ){Ú Ŋ(â log_derivative_log(n),exp_log(n));}Ί Ҫ(ǀ){Ú 3 ӎ(â γ/2)+Ŋ(â log_derivative_scratch(n),â exp_newton_scratch(n));}í Ǹ(ƶ,О,Ö e,Ö c,ĝ g,Ѕ){ѻ n=ȸ,m=γ/2,k=m/2,ď=ß(m)ĺ(f ű||f Ŭ ζ)ǩ ǫ lo=ĸ ʴ,ɼ=Ԃ(3 Ԏ)Ǎ q=Ѯ(ɵ)Ü{Ú i<m?lo Ȝ:g Ȝ;}ƣ Ÿ(e)ĩ ƍ l7=m>=128&&ɣ==k&&log_blocks(n)>=3 ĩ ƍ block2=l7&&log_blocks(n)==4&&n-m>k ǫ qt=Ԃ(ď,l7?m:0)ǫ q2t=Ԃ(2 Ԏ,block2?m:0);Ɠ d0t=Ƿ;lo Ŭ Ē if(n>=2)Γ(t,f,n,ɼ,Ѯ(Õ ù,Ğ d)Ü{ɵ ƒ(ʲ d Û Ɲ){ģ at=ҵ+i κ x=ń(ŭ(Ō(d Ȝ),Ÿ),kP)ĺ(at+8<=m||at>=m){ū(q(at),x);}ŝ{љ č ɚ;Š(č,x)ø(ǉ 8 ų)*q(at+j)=č Ɓ;}}Ć(Ͼ d Û Į)*q(ҵ+i)=ʀ(d Ĕ,e);},Ѯ(ʍ,Ğ dt)Ü{if(j ζ)d0t=dt à ĺ(j у&&l7)times_x ԏ(Ȱ,d0t,dt à,e,Ͻ*q(m),0,qt)ĺ(j==2&&block2)times_x ə(Ȱ,dt à,Ƿ,e,*q(m),Ͻ*q(m+k),q2t);})ǫ start=g ȑ(Ħ(n,ϲ));exp_direct(lo,start)ø(Ö&x:start)x=ʀ(x,c)ĺ(n>ϲ)exp_newton(t,lo,g,ɼ,QTransforms{qt,q2t});}}ā ē{Ϧ{ή ŵ>Ɩ Ě exp_tail_coefficient(ǣ*in,Õ ʭ,ǣ*g,ǀ){ģ þ Ш(n,ʭ?ʭ-1:0),full=þ/8*8 κ reversed=ʞ,ni=ĕ(ì Ƥ kNI),p=ɟ;ĭ o4=đ Ч(1);ʍ ƒ(;j<full;){н=đ,Ȏ=Ŧ ȡ end Ш(full,j+64)ø(;j<end Ӗ
,ȯ.next()){ĭ a,b;Ǵ(&a,in+j+1,ƚ(a));Ǵ(&b,g+n-j-8,ƚ(b));a=ń(a,kP)Ѓ(!ŵ)a=ů(Ŕ(a,ȯ.ĳ()));b=Ľ(ń(b,kP),reversed);Ŧ=ð(Ŧ,Þ(a,b));Ȏ=ð(Ȏ,І);}ĥ Ģ ĳ=ů(ǝ);o4 ӑ(ƪ(o4,ĳ));}o4 ӑ(ƪ(o4,Ł(o4,o4,1)));o4 ӑ(ƪ(o4,Ɛ(o4,0x4E)));o4 ӑ(ƪ(o4,Ɛ(o4,0xB1)));Ě ă=Ҿ(Ö(Ҙ(ɦ(o4))))ø(;j<þ ų){Ě ĳ=in[j+1]*g[n-j-1]Ѓ(!ŵ)ĳ*=Ě(j+1);ă+=ĳ;}Ú ă/Ě(n);}ή ŵ>Ɩ Ĉ import_exp_input(ǣ*in,Õ ù,Ƞ,Ņ to){Ά;ɵ Ē Œ undo(á Ǹ(kR,ӕ))Ѵ ȯ=ƪ(ĕ(Ö(ù)),ɜ)ø(ʲ þ Ɲ,ȯ=ƪ(ȯ,ĕ(8))){ĭ ĳ;Ǵ(&ĳ,in+Ў i,ƚ(ĳ))Ѓ(ŵ)ū(to+i,ń(ŭ(ĳ,undo),kP))і ū(to+i,ń(Ŕ(ĳ,ȯ),kP));}Ć(Ͼ þ Į){Ĺ(ŵ)to Ƈ in[Ў i]ȃ і to Ƈ ʀ(in[Ў i]ȃ,Ў i);}}}í native_exp_regular(ǣ*in,Õ ʭ,л,ǀ,ƍ ŵ){ģ m=â γ/2,ä Ш(ʭ,n),Ԍ Ш(ä,m)ȉ lg=exp_log(n)й(ɠ(n)Я(exp_scratch(n)))Ǭ g к,ĸ ф(exp_scratch(n))Ѵ lo=ĸ ʴ Ǎ import=Ѯ(Õ ù,Ƞ,Ņ to){if(ŵ)â import_exp_input ԏ(in,ù,þ,to)і â import_exp_input ə(in,ù,þ,to);};import(0,Ԍ,lo à);ż(lo ʷ Ԍ,lo Ƃ,0u)ĺ(ä>m)import(m,ä-m,g Ʒ m)ĺ(n>Ŋ(ä,m))ż(g ʷ Ŋ(ä,m),g Ƃ,0u);â exp_direct(lo,g ȑ(Ħ(n,â ϲ)))ø(ł&x:g ȑ(Ħ(n,â ϲ)))x=ʀ(x,â kR)ĺ(n>â ϲ)â exp_newton(t,lo,g,Ԃ(ß(m)));ž(į,g à,n);}í native_exp(ǣ*in,Õ ʭ,л,ǀ,ƍ ŵ=Ś){ģ
 ʜ=Ø bit_floor(n),Η=n-ʜ ĺ(ʜ>=4096&&Η&&Η<=4){ģ Ų=ǲ;ƙ(į,Ų+n);native_exp_regular(in,ʭ,į,ʜ,ŵ)ø(Õ at=ʜ;at<n ė at){ǣ ĳ=ŵ?â exp_tail_coefficient ԏ(in,ʭ,ԃ Ų,at):â exp_tail_coefficient ə(in,ʭ,ԃ Ų,at);į ӱ(ĳ);}Ú;}native_exp_regular(in,ʭ,į,n,ŵ);}í native_power(ǣ*in,Õ ʭ,л,ǀ,Ö e,Ě c){ȟ lg=power_log(n)ȡ ä Ш(ʭ,n)й(ɠ(n)Я(Ҫ(n)))Ǭ g к,ĸ ф(Ҫ(n))ø(Ů ä Į){ʦ x=Ӫ;g Ƈ Ħ(x,x-Ƀ);}Ǹ(t,g ȑ(ä),e,Ħ(c.v,c.v-Ƀ),g,ĸ);ž(į,g à,n);}}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā ç ē{í append_scaled(ҽ,л,Ö ŧ){Ù â ƭ Ÿ(ŧ)ĺ(Ư>į.max_size()-ǲ)throw Ø length_error("fps output");ƙ(į,ǲ+Ư);љ Ƴ[64]ø(Ů Ư ƽ 64){ʦ þ Ш<Õ>(64,Ư-i);ʍ ƒ(;j+8<=þ Ӗ)â Š(Ƴ+j,â ń(â ŭ(â Ž(a Ȝ+j),Ÿ),Ƀ))ø(;j<þ ų)Ƴ ˉ ʀ(a[i+j],ŧ);ž(į,Ƴ,þ);}}í native_inverse(ǣ*in,Õ ʭ,л,ǀ){ȟ lg=inverse_log(n)ȡ ǯ=ǻ lg,k=ǯ/2 ȡ c Ш(ʭ,n),ƃ=Ŋ(ǯ,ȵ(k))й(ɠ(ǯ)Я(ƃ)Я(k))Ǭ a ф(ǯ),ĸ ф(ƃ),g ф(k)ø(Ů c Į){Ù ī v=Ӫ;ӫ Ħ(v,v-Ƀ);}Ù О=a ȑ(c)Ǎ extend=Ѯ(Ğ d6,ĝ to,ĝ Ǩ,ĝ Ǡ){ģ þ=d6 Û Ȇ(d6,0,Ǩ);t.Ŷ(f ȑ(Ħ(2*þ,Ǳ)),0,Ǡ,Ǩ,Ƨ);t.Ŷ(Ǡ Ė(þ),þ,Ǡ,Ǩ,Ƨ,53925088)ĺ(to à!=Ǡ Ʒ þ)ĵ(Ǡ ʊ(þ),to Û,to ü);}ȡ Ʃ Ш(n,k),Ƽ Ш(Ʃ,â kInverseBase);â ѧ(f,g ȑ(Ƽ))ø(Ů Ƽ Į)g Ƈ ʀ(g Ĕ,932051910)ø(Ƞ=Ƽ;þ<Ʃ;þ*=2){ģ ä=2*þ;extend(g ȑ(þ),g Ė(þ,Ħ(þ,Ʃ-þ)),ĸ ȑ(ä),Ԃ(ß(ä),ä));}if(n<=k)Ú ž(į,g à,n);extend(g,a Ė(k,n-k),ĸ ȑ(ǯ),a);ƙ(į,n);ž(į,g à,k);ž(į,a Ʒ k,n-k);}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā ç ē{Ϧ{ò ɿ Ξ=64;í sqrt_direct(О,Ö c,ĝ g){Ę ʀ;Ġ ć=á Ǹ(Đ(2,c),ӕ);g Г c ø(ɵ=1 Ͼ ȸ Į){ŉ ʂ=i<Ǳ?f Ĕ:0 ø(ʍ=1;j<i ų)ʂ+=Ͻ ŉ(g Ɓ)*g[i-j]Ұ;g Ƈ Đ(Ö(ʂ Ұ),ć);}}Ȅ load_padded(О,Õ at){if(at+8<=Ǳ)Ú Ž(f Ʒ at);љ x ɚ={}ĺ(at<Ǳ)ȩ(f ʊ(at),f Ƃ,x)Ū Ō(x);}Ȅ first_lanes(Ƞ){Ú τ(ĕ(Ö(Ħ<Õ>(þ,8))),ɜ);}í store_first(Ņ p,ɍ,Ƞ){if(þ>=8)Ú ū(p,x)Ӑ(õ ý*>(p),first_lanes(þ),x);}Ȅ load_first(Ɠ p,Ƞ){if(þ>=8)Ú Ž(p)Ū _mm256_maskload_epi32(õ ȟ*>(p),first_lanes(þ));}ĉ Ȅ leaf_square(Ù ʅ&Ļ){ȱ,p=ɟ,p2=П;н,Ȏ,bi,t0,t1,t2;
	#define Â(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,q,r) h(m,b) c(n,b,d) c(m,b,a) h(n,b)c(l,b,e)h(40(%[w]),b)c(l,b,g)c(24(%[w]),b,i)f(e,a,a)h(44(%[w]),b)c(k,b,e)f(g,d,d)f(i,a,a)h(48(%[w]),b)c(k,b,g)c(16(%[w]),b,i)f(e,a,a)h(52(%[w]),b)c(j,b,e)f(g,d,d)f(i,a,a)h(56(%[w]),b)c(j,b,g)c(8(%[w]),b,i)f(e,a,a)h(60(%[w]),b)c(4(%[w]),b,e)f(g,d,d)f(i,a,a)f(e,a,a)f(d,d,d)c(%[ni],a,e)c(%[ni],d,g)c(%[p],e,e)c(%[p],g,g)f(e,a,a)f(g,d,d)q($32,a,a)o($0xaa,d,a,a)r(%[p2],a,e)"vpminud %[t0], %[even], %[even]"
asm(Â(%[Ŧ],%[bi],Ò,%[Ȏ],%[t0],Í,%[t1],Ï,%[t2],12(%[w]),20(%[w]),28(%[w]),32(%[w]),36(%[w]),Î,Ó,Ô)
#undef Â
:[Ŧ]"=&x"(Ŧ),[Ȏ]"=&x"(Ȏ),[bi]"=&x"(bi),[t0]"=&x"(t0),[t1]"=&x"(t1),[t2]"=&x"(t2):[w]"r"(Ļ.ɽ),[ni]"x"(ni),[p]"x"(p),[p2]"x"(p2),"m"(*õ Ġ(*)[17]>(Ļ.ɽ)))Ū Ŧ;}ƛ InverseSquareBottom{Ǒ Ś,Ƚ=Ɔ;М;Ɠ ô;һ;ĉ Ĉ ѐ(Õ g,ʅ(&Ļ)ɛ)Ù{Ы(Ļ,f,ķ,g);}Ǣ{ɥ ɛ;ȁ{ɕ;ϥ;
#pragma GCC unroll 4
Ӏ f ѯ=leaf_square(Ļ[j&1]ѯ);υ(f,Τ(ô,Ў j))ø(ý Ń)ԁ}}};ò ƍ ɷ(Ԇ){Ú Ī(nv)%2 ζ;}Ϥ Ѧ(Ù Ǉ<Ƭ>&Ŝ,Ң,Ԇ){if(!ɷ(nv)){ő(a,nv/2,0)Ū ő(a+4*nv,nv/2,1);}Ć(Õ q Ē q<4 ė q)ő(a ʳ q*nv,nv/4,q);}Ϥ ϱ(ƶ,Ù Ǉ<Ƭ>&Ŝ,Ң,Ԇ,ƍ p7=Ɔ){Ù Ɏ in(a,4*nv,0)ĺ(!ɷ(nv))Ú radix2_halves(Ŝ,in,a,nv,Ȱ)ĺ(!p7)forward_top4(in,Ț(a),nv/4,Ȱ);Ѧ(Ŝ,a,nv);}í inverse_top(ƶ,ҳ v,Ԇ,Π,Ö ŧ){if(ɷ(nv))Ú inverse_top4(v,nv/4,ǂ,ɧ,ƭ(ŧ));Ӧ(v,nv/2,ǂ,ŧ);}Ą<Half kOutput,Ũ Visit>ˮ ǔ(ƶ,ҳ v,Ԇ,Ö ŧ,Visit Ϙ){æ ƍ kLower=kOutput!=Ƨ,kUpper=kOutput!=ƨ ƣ s(ŧ)ĺ(ɷ(nv)){ƾ 4 ƣ z(ɧ ƹ,ɧ[9])ʋ{Ģ p0=v Ɓ,p1=Ӌ,p2=v ɋ,p3=v Ɋ κ ab ӑ(ƪ(p0,p1)),cd ӑ(ƪ(p2,p3))κ amb ӑ(Ǚ(p0,p1)),cmd Ҝ(Ǚ(p2,p3),z);ĭ lo ǥ={},hi ǥ={}Ѓ(kLower)lo Г ŭ(ϓ,s),lo ƹ Ҝ(ϒ,s)Ѓ(kUpper)hi Г ŭ(Ќ,s),hi ƹ Ҝ(ϑ,s);Ϙ(Ø true_type{},j,h,lo,hi);}Ú;}ƾ 2 ʋ{ĭ lo ƹ={},hi ƹ={}Ѓ(kLower)lo Г ŭ(ƪ(v Ɓ,Ӌ),s)Ѓ(kUpper)hi Г ŭ(Ǚ(v Ɓ,Ӌ),s);Ϙ(Ø false_type{},j,h,lo,hi);}}ˮ forward_lower4(ҳ v,ʍ,Ϫ,ĭ a,ĭ b,Œ&z){Ģ zb Ҝ(b,z);v ˉ ƪ(a,b),Ӌ=Ǚ(a,b),v ɋ=ƪ(a,zb),v Ɋ=Ǚ(a,zb);}ˮ forward_upper4(ҳ v,ʍ,Ϫ,ĭ c,ĭ d,Œ&z){Ģ mc=é(ɟ,c),zmd Ҝ(é(П,d),z);v ˉ ƪ(c,d),Ӌ=Ǚ(c,d),v
 ɋ=ƪ(mc,zmd),v Ɋ=Ǚ(mc,zmd);}ò ƭ top_root(ƶ){Ú ƭ(Ȱ ƹ,Ȱ[9]);}Ȅ*И(ҽ){Ú Ț(ʰ);}ԅ product_scale(ǀ){Ú ȋ ƹ[Ī(n)];}Ȅ Ʌ(О,Ό,Õ at,ĭ s){Œ Ɵ((kP+1)/2)κ fs=ƪ(load_padded(f,at),load_padded(f,m+at))Ū ń(ŭ(é(ƪ(s,П),fs),Ɵ),kP);}í Ʌ(О,Ό,Õ ù,Ɠ s,ȧ,Ƞ){Ć(Ů þ Ɲ)store_first(ѓ i,Ʌ(f,m,Ў i,Ō(s+i)),þ-i);}í sqrt_step(ƶ,О,Ό,Ņ g,Ņ h,ĝ b,ĝ ht){ģ Ɵ=m/2,nv=m/8 ǫ gt=b ʴ,w=b ч,Ǡ=b ϡ ƣ z=top_root(t)ĩ Ȥ p5{Ȱ,ɧ,ht à};Ġ ŧ=product_scale(m),m4=product_scale(2*m)Ȇ(Ğ(g,m),0,gt);ҳ v=И(w);Ѧ(Ǉ(Ȱ,ɧ,ȗ{p5,gt à}),w à,nv);ǔ<Ƨ>(t,v,nv,ŧ,Ѯ(ł ɷ,ʍ,Õ q,Ģ*,Ģ*e){Ĺ(ɷ)forward_upper4(v,j,q,ń(e Ŭ,kP),e ƹ,z)і v ˉ e Ŭ,v[j+q]=é(П,e Ŭ);});Ѧ(Ǉ(Ȱ,ɧ,p5),w à,nv);ǔ<Ƨ>(t,v,nv,ŧ,Ѯ(ł ɷ,ʍ,Õ q,Ģ*,Ģ*x){ū(h+Ɵ Ʀ j,ń(x Ŭ,kP))Ѓ(ɷ)ū(h+Ɵ Ʀ(j+q),ń(x ƹ,kP));})Ȇ(Ğ(h,m),0,ht);v=И(gt);Ѧ(ӗ,gt à,nv);ǔ<Ȉ>(t,v,nv,ŧ,Ѯ(ł ɷ,ʍ,Õ q,Ģ*lo,Ģ*hi){Ĺ(ɷ){v ˉ Ʌ(f,m,8*j,lo Ŭ),v[j+q]=Ʌ(f,m,8*(j+q),lo ƹ);v[j ʳ q]=Ʌ(f,m,Ɵ Ʀ j,hi Ŭ);v[j+3*q]=Ʌ(f,m,Ɵ Ʀ(j+q),hi ƹ);}ŝ{forward_lower4(v,j,q,Ʌ(f,m,8*j,lo Ŭ),Ʌ(f,m,Ɵ Ʀ j,hi Ŭ),z);}});ϱ(t,Ǉ(Ȱ,ɧ,p5),Ǡ à,2*nv);ǔ<ƨ>(t,И(Ǡ),2*nv,m4,Ͷ{ū(g+m Ʀ j,ń(d Ŭ,kP))Ѓ(ɷ
)ū(g+m Ʀ(j+q),ń(d ƹ,kP));});}}ò ý sqrt_log(ǀ){Ú Ŋ(ʵ,ý(ʌ(ʘ(n,2)-1))-1);}Ί sqrt_scratch(ǀ){if(n<=â Ξ)Ú 0 Ū 3 ӎ(ǻ sqrt_log(n));}í sqrt_steps(ƶ,О,Ö c,ҽ,ĝ b,ĝ ht){ѻ ǯ=Ư ĺ(f ű||f Ŭ ζ||ʀ(c,c)!=f Ŭ||ǯ<Ξ)ǩ;sqrt_direct(f,c,a ȑ(Ξ));Ǜ h=a Ʒ ǯ/2;љ h0[Ξ/2];ѧ(a ȑ(Ξ/2),h0)ø(Ö&x:h0)x=x?Ͻ x:0 Ȇ(h0,0,ht ȑ(Ξ))ĺ(ǯ>Ξ)ĵ(h0,Ξ/2,h)ø(Ό=Ξ;m<ǯ;m*=2)sqrt_step(t,f,m,ʰ,h,b ϡ,ht ϡ);}í sqrt_last_step(ƶ,О,Ğ gt,Ğ ht,ĝ Ǡ,ĝ hi){ѻ m=gt Û,Ɵ=m/2,ɼ=hi Û,nv=m/8 ĺ(ɼ ζ||ɼ>m)ǩ;Ġ ŧ=product_scale(m)ƣ z=top_root(t)ĩ Ȥ p4{Ȱ,ɧ,gt à},p5{Ȱ,ɧ,ht à}ĩ Ǉ product_h(Ȱ,ɧ,p5);ҳ v=И(Ǡ)Ǎ source=Ѯ(ł ɷ,ʍ,Õ q,ł x){Ĺ(ɷ)forward_lower4(v,j,q,x(0,j),x(1,j+q),z)і v ˉ x(0,j);}Ǎ each=[](ł ɷ,ʍ,Õ q,ł put){put(0,j)Ѓ(ɷ)put(1,j+q);}Ǎ put=Ѯ(ȧ,Ƞ,Õ p,ɍ)Ü{if(8*p<þ)store_first(į Ʀ p,x,þ-8*p);};Ѧ(ӗ,Ǡ à,nv)ĩ ƍ split=ɼ>Ɵ;inverse_top(t,v,nv,split?Ȉ:ƨ,ŧ);Ʌ(f,m,0,Ǡ à,Ǡ à,Ħ(ɼ,Ɵ))ȡ þ=split?ɼ-Ɵ:0;Ǜ ʓ=hi Ʒ(ɼ-þ);Ʌ(f,m,Ɵ,Ǡ Ʒ Ɵ,ʓ,þ);ϱ(t,product_h,Ǡ à,nv,Ś)ĺ(!split){ǔ<ƨ>(t,v,nv,ŧ,Ͷ{each(ɷ,j,q,Ѯ(ң,Õ p){put(hi à,ɼ,p,ń(d ƫ,kP));});})а}ǔ<ƨ>(t,v,nv,ŧ,Ͷ{source(ɷ,j,q,Ѯ(ң,Õ p){Ģ d0=ń(d ƫ
,kP);ū(hi ʥ p,d0)Ū d0;});});ϱ(t,Ǉ(Ȱ,ɧ,p4),Ǡ à,nv);ǔ<Ƨ>(t,v,nv,ŧ,Ѯ(ł ɷ,ʍ,Õ q,Ģ*,Ģ*x){source(ɷ,j,q,Ѯ(ң,Õ p){ģ at=8*p Ū ů(ƪ(x ƫ,load_first(ʓ+Ħ(at,þ),at<þ?þ-at:0)));});});ϱ(t,product_h,Ǡ à,nv);ǔ<ƨ>(t,v,nv,ŧ,Ͷ{each(ɷ,j,q,Ѯ(ң,Õ p){put(ʓ,þ,p,ń(d ƫ,kP));});});}í sqrt(ƶ,О,Ö c,ĝ g,Ѕ){ѻ n=ȸ ĺ(f ű||f Ŭ ζ||ʀ(c,c)!=f Ŭ)ǩ ĺ(n<=Ξ)Ú sqrt_direct(f,c,g)ȡ m=ǻ sqrt_log(n),č=ß(m)ǫ b=ĸ ʴ,ht=Ԃ(č,m),Ǡ=Ԃ(2*č,m);sqrt_steps(t,f,c,g ʴ,b,ht)Ȇ(g ʴ,0,b);sqrt_last_step(t,f,b,ht,Ǡ,g Ė(m));}}ā ç ē{í native_sqrt(ǣ*in,Õ ʭ,л,ǀ,Ě ˏ){ȟ lg=sqrt_log(n)ȡ c Ш(ʭ,n)й(ɠ(c)Я(n)Я(sqrt_scratch(n)))Ǭ f ф(c),g к,ĸ ф(sqrt_scratch(n))ø(Ů c Į){Ù ī v=Ӫ;f Ƈ Ħ(v,v-Ƀ);}Ġ ct=ʀ(Ħ(ˏ.v,ˏ.v-Ƀ),á Ǹ(65536,Ƀ-2));sqrt(t,f,ct,g,ĸ);append_scaled(g,į,65536);}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
Ň ā ç ʉ::unbalanced{Ę ā ē;ò ƍ fits(ǀ,Ό){ʦ n0 Ш(n,m),i4=Ŋ(n,m)Ū n0>32&&i4>=32768&&n0<=i4/32;}ò Ʋ Ź(ˢ a,ˢ b){ģ ǯ=8*ɬ(Ħ(Ư,η))Ǎ&x=Ư>=η?a:b Ǎ&y=Ư>=η?b:a ȡ n=x Û,m=y Û,ď=ǯ-m+1 ȉ lg=Ī(ǯ)й(Č Ʈ(lg)+3 ӎ(ǯ))Ǭ bt ф(ǯ),in ф(ǯ),į ф(ǯ)ø(Ů m Į)in Ƈ y Ĕ ȃ Ȇ(in ʴ,0,bt);Ʋ ă;ç ƙ(ă,n+m-1)ø(ӛ n;p+=ď){ģ þ Ш(ď,n-p)ø(Ů þ Į){Ù ī v=x[p+i].v;in Ƈ Ħ(v,v-Ƀ);}t.Ŷ(in ȑ(þ),0,į,bt)ȡ j8=p ζ?0:m-1 ø(Ů j8 Į){Ù ī v=ă[p+i].v+į Ĕ;ă[p+i]=Ҿ(Ħ(v,v-Ƀ));}ç ž(ă,ԃ j8,þ+m-1-j8);}Ѡ}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
Ň ā ç ʉ::ˏ{ò ī ů(Ě x)Щ ò ƍ same_eight(ǣ*p,ī ĳ){ϸ v;Ǵ(&v,p,ƚ(v));v=ą(v,é(v,å(Ƀ)))Ū _mm256_movemask_epi8(_mm256_cmpeq_epi32(v,å(ĳ)))==-1;}ò ƍ remaining(ˢ a,ī ĳ){ɵ=8 ø(ʲ Ư Ɲ)if(!same_eight(a Ȝ,ĳ))Ú Ś ø(Ͼ Ư Į)if(ů(ɨ)!=ĳ)Ú Ś Ū Ɔ;}Ɩ ò ƍ fits(ҥ a,ҥ b){if(Ħ(Ư,η)<4096)Ú Ś ĩ ī av=ů(a Ŭ),bv=ů(b Ŭ)ĺ(!same_eight(ʰ,av)||!same_eight(b à,bv))Ú Ś Ū remaining(a,av)&&(&a==&b||remaining(b,bv));}Ɩ ò Ʋ Ź(ˢ a,ˢ b){ģ n=Ŋ(Ư,η),m Ш(Ư,η),ä=n+m-1 ĩ ī c=ů(Ҿ(ů(a Ŭ))*Ҿ(ů(b Ŭ)));Ʋ ă(ä)ĺ(c ζ)Ѡ Ť ī ù ҹ(ī i=0;i<8 Į)ù Ƈ ŉ(c)*(i+1)%Ƀ;ϸ ĳ=_mm256_load_si256(õ Ù ϸ*>(ù))Ǎ ɯ ʖ(ŉ(c)*8%Ƀ),p ʖ(Ƀ)Ǎ ˁ=ʞ;ɵ ƒ(ʲ m Ɲ){Ǵ(ш(ă Ȝ),&ĳ,ƚ(ĳ))Ǎ l6=Ľ(ĳ,ˁ);Ǵ(ш(ă Ʒ ä-i-8),&l6,ƚ(l6));ĳ=ň(ĳ,ɯ);ĳ=ą(ĳ,é(ĳ,p));}ī d6=_mm256_extract_epi32(ĳ,0)ø(Ͼ m Į){ă Ƈ ă[ä-i-1]=Ҿ(d6);d6+=c;d6 Ш(d6,d6-Ƀ);}if(n>m+1)ż(ă ʷ m,ă ʷ ԕ,Ҿ(ŉ(c)*m%Ƀ))Ȧ}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
Ň ā ç ʉ{ā ntt{ā kernels=ì kernels;Ϧ=ã;Ũ Ғ{Õ n_,m_,len_;ʃ ě arena_;ʃ Č Ǫ;Ņ a_,*b_;ċ ý ɶ(ǀ,Ό){if(n ζ||m ζ||n>(1u<<23)||m>(1u<<23)||n+m-1>(1u<<23))ǩ Ū Ŋ(6,ý(ʌ(n+m-2)));}Ⱥ Ғ(ǀ,Ό):Ғ(n,m,Ƿ){}Ғ(ǀ,Ό,Ņ b8):n_(n),m_(m),len_(ǻ ɶ(n,m)),arena_(ʃ Č Ʈ(ɶ(n,m))+(b8?1:2)*ʃ ß(len_)),Ǫ(arena_,ɶ(n,m)),a_(b8?b8:arena_ ņ(len_)à),b_(arena_ ņ(len_)à){if(n ζ||m ζ||ɶ(n,m)>23)ǩ;}ċ ƍ fits(ǀ,Ό){ȟ lg=ɶ(n,m)Ū lg%2 ζ&&lg>=10&&2*Ŋ(n,m)<=ǻ lg;}Ң(){Ú a_;}Ņ b(){Ú b_;}Ɠ Ź(){Ǫ.ı(Ğ(b_,m_),0,ĝ(b_,len_));Ǫ.Ŷ(Ğ(a_,n_),0,ĝ(a_,len_),Ğ(b_,len_),ʃ Ȉ,â Ǹ(â kR,Ƥ ӕ))Ū a_;}};}Ę Product=ntt::Ғ ȅ ĭ ȅ ƭ ȅ ƪ ȅ Ǚ ȅ kP ȅ ń;Ȅ ŭ(ɍ,Œ&f){Ú ntt::â Ź(x,f);}Ȅ diff_canonical(ɍ,ĭ y){Ú é(ň(x,ntt::â ɟ),y);}Ũ Subtrees{Ɠ Ə,*ϰ;Ⱥ Subtrees(Ɠ r,Ɠ ir):Ə(r),ϰ(ir){}Ĉ Ϙ(ҳ a,ҳ b,Ԇ,Ȕ)Ù{Ę ā ʃ ê;Ǉ<Ǆ>(Ə,ϰ,Ǆ{Ə}).Ϙ(õ Ņ>(b),nv,k);Ȥ b9{Ə,ϰ,õ Ņ>(b),k*nv};Ǉ<Ȥ>(Ə,ϰ,b9).Ϙ(õ Ņ>(a),nv,k);}};í forward_radix8(ҳ f,Õ q,М){Œ i(ķ ƹ,ķ[9]),y(ķ ǥ,ķ[10]),z(ķ Ѐ,ķ[11])ø(ǉ q ų){Ģ f0=f Ɓ,f1=f[j+q],f2=f[j ʳ q],f3=f[j+3*q]κ g0=ƪ(f0,f2),g1=ƪ(f1,f3)κ h0=diff_canonical(f0,f2),ih1 Ҝ(diff_canonical
(f1,f3),i);f ˉ ƪ(g0,g1),f[j+q]=Ǚ(g0,g1);f[j ʳ q]=ƪ(h0,ih1),f[j+3*q]=Ǚ(h0,ih1)κ if2 Ҝ(f2,i),if3 Ҝ(f3,i)κ u0=ń(ƪ(f0,if2),2*kP),v0=ń(Ǚ(f0,if2),2*kP)κ yu1 Ҝ(ƪ(f1,if3),y),zv1 Ҝ(Ǚ(f1,if3),z);f[j+4*q]=ƪ(u0,yu1),f[j+5*q]=Ǚ(u0,yu1);f[j+6*q]=ƪ(v0,zv1),f[j+7*q]=Ǚ(v0,zv1);}}í copy_input(ҥ in,ȧ,Õ ǯ){ɵ Ē ʦ p ʖ(Ƀ)ø(ʲ in Û Ɲ){ʦ x=ɂ(õ Ù ϸ*>(in Ȝ));Ѩ(õ ϸ*>(ѓ i),Ǧ);}Ć(Ͼ in Û Į)į Ƈ Ħ(Ӫ,Ӫ-Ƀ);ż(ѓ i,ѓ(i<=ǯ/2?ǯ/2:ǯ),0);ż(ѓ ǯ,ѓ ǯ ү,0);}í copy_input_mint(ҥ in,Ě*į){Ά;ɵ Ē ʦ p ʖ(Ƀ)ø(ʲ in Û Ɲ){ϸ x;Ǵ(&x,in Ȝ,ƚ(x));x=Ǧ;Ǵ(ш(ѓ i),&x,ƚ(x));}Ć(Ͼ in Û Į)į Ƈ Ҿ(Ħ(Ӫ,Ӫ-Ƀ));}Ő Plan>ò Ʋ run_borrowed_as(ҥ a,ҥ b){ģ ǯ=ǻ Ŋ(6,ý(ʌ(Ư+η-2)));ɿ Б=32 ȡ č=ǯ ү+Б/ƚ(Ě)-1,ä=Ư+η-1;Ʋ ă;ƙ(ă,č);ă.assign(č,Ҿ(0))Ǎ a6=Λ(ă à)ȡ Ų=((Б-(a6&(Б-1)))&(Б-1))/ƚ(Ě);Ě*Ù to=ă Ʒ Ų;Plan plan(Ư,η,õ Ņ>(to));copy_input_mint(a,to);copy_input(b,Ƹ b(),ǯ);Ƹ Ź()ĺ(Ų)Ø memmove(ш(ă à),to,ä*ƚ(Ě));ă Ř(ä)Ȧ}ò Ʋ run_borrowed(ҥ a,ҥ b){Ú run_borrowed_as<Product>(a,b);}Ő Plan>ò Ʋ run(ҥ a,ҥ b){Plan plan(Ư,η)Ǎ ǯ=ǻ Ŋ(6,ý(ʌ(Ư+η-2)));copy_input(a,Ƹ a(),ǯ);copy_input(b,Ƹ b(),ǯ)Ǎ*p=Ƹ Ź();
Ʋ ă ȡ ä=Ư+η-1;ƙ(ă,ä);ž(ă,p,ä)Ȧ}ò Ʋ Ź(ҥ a,ҥ b){ơ(!a ű&&!b ű&&Ư+η-1 ɰ)ĺ(ˏ::fits(a,b))Ú ˏ::Ź(a,b)ĺ(unbalanced::fits(Ư,η))Ú unbalanced::Ź(a,b)Ū run_borrowed_as<ntt::Ғ>(a,b);}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā ç ē{Ί З(ǀ,Ό){Ú n>=m?n-m+1:0;}Ί remainder_size(ǀ,Ό){Ú Ħ(n,m-1);}ā ƺ{ƞ;ò ɿ ϯ=8;ò ɿ kLongDivision=ǻ 12;ò ɿ kDirectRemainder=64;ò æ ǖ ʬ,26>kTransformNs={0,0,0,0,0,0,0.45,0.48,0.51,0.55,0.6,0.65,0.7,0.76,0.81,0.88,0.95,1.02,1.09,1.17,1.23,1.3,1.37,1.44,1.51,1.58};ò æ ʬ kLeafProductNs=0.92,kDirectTermNs=0.04,kFoldNs=0.25;Ί transform_length(ǀ){Ú ʘ(64,ɬ(n));}ò ʬ ʾ(Õ ǯ){ʦ lg=Ī(ǯ)Ū Õ(lg)<kTransformNs Û?kTransformNs[lg]*ʬ(ǯ):Ǻ;}ò ʬ Ӊ(Õ ǯ){Ú 2*ʾ(ǯ)+kLeafProductNs*ʬ(ǯ);}ò ʬ inverse_ns(ǀ){ʬ ns ƒ(Ȕ=kInverseBase;k<n;k*=2)ns+=ʾ(Ӵ)ʳ Ӊ(Ӵ)Ū ns;}ƛ ɢ{Õ s=0,ǽ=0,pad=0,Ǯ=0,Η Ē ƍ ѥ=Ś;ʬ ns=Ǻ;ɢ()=default;ɢ(Ȕ,Õ d,Õ ŀ):s(ŀ),ǽ((k+ŀ-1)/ŀ),pad(ǽ*ŀ-k),Η(transform_length(2*d)),ѥ(d<ŀ){ģ ǯ=2*s;ns=inverse_ns(ɒ(k))+ʾ(ǯ)+ʬ(ǽ)*Ӊ(ǯ)ĺ(ǽ у)Ú ĺ(ѥ){ns+=ʾ(Η)+ʬ(ǽ-1)*Ӊ(Η)а}ģ l3=d/s+1;Ǯ Ш(l3,ǽ-1)ĺ(Ǯ>ϯ){ns=Ǻ а}ʬ Ơ ƒ(ʍ=1;j<ǽ ų)Ơ+=ʬ(Ħ(j,l3));ns+=ʬ(Ǯ ʳ(ǽ-1))*ʾ(ǯ)+Ơ*kLeafProductNs*ʬ(ǯ);}Õ ɒ(Ȕ)Ù{Ú ǽ у?k:s;}};enum Ũ ǵ{kDirect,kWrap,kSplit,kTail};ƛ RemainderPlan{ǵ method;Õ î Ē ʬ ns=Ǻ;RemainderPlan(ǵ how,ǀ,Ȕ,Õ d,Ù ɢ&ť):method(how){ģ wrap=transform_length
(d);switch(how){case ǵ::kDirect:if(k<=kDirectRemainder)ns=ʬ(k)*ʬ(d)*kDirectTermNs;Ϸ case ǵ::kWrap:î=wrap;ns=Ӊ(î)+ʾ(î)+ʬ(n+k)*kFoldNs;Ϸ case ǵ::kSplit:î=wrap/2 ĺ(î>=64&&d>î&&k+(d-î)<=î+1)ns=ʾ(î)ʳ Ӊ(î)+ʬ(n+d)*kFoldNs;Ϸ case ǵ::kTail:if(!ť.ѥ)Ϸ ns=(ť.ǽ у?ʾ(ť.Η):0)+Ӊ(ť.Η)+ʬ(d)*kFoldNs;Ϸ}}Õ buffers()Ù{Ú method==ǵ::kWrap?2:method==ǵ::kSplit?3:0;}};ƛ Ν{ǀ,m,k,d;ƍ ˑ;ɢ ť;RemainderPlan ɑ;ý lg=ʵ;Ν(Õ n_,Õ m_):n(n_),m(m_),k(З(n_,m_)),d(m_-1),ˑ(k*d<=kLongDivision),ɑ(ǵ::kWrap,n,k,d,ť){if(ˑ)Ú;ʬ best=Ǻ ø(Õ s=32;;s*=2){Ù ɢ p(k,d,s)ø(ǵ how:{ǵ::kDirect,ǵ::kWrap,ǵ::kSplit,ǵ::kTail}){Ù RemainderPlan r(how,n,k,d,p)ĺ(p.ns+r.ns<best)best=p.ns+r.ns,ť=p,ɑ=r;}if(s>=k||4*s>(ǻ Č::Ҕ))Ϸ}set_log();}Ν(Õ n_,Õ m_,Õ s,ǵ how):n(n_),m(m_),k(З(n_,m_)),d(m_-1),ˑ(Ś),ť(k,d,s),ɑ(how,n,k,d,ť){set_log();}ƍ ѥ()Ù{Ú ť.ѥ&&(ť.ǽ>1||ɑ.method==ǵ::kTail);}Ĉ set_log(){lg=Ī(Ŋ({2*ť.s,ѥ()?ť.Η:0,ɑ.î}));}Õ μ()Ù{Ù ɢ&p=ť Ū ѥ()?m:Ħ(m,Ŋ(p.ɒ(k),(p.Ǯ+1)*p.s));}Õ quotient_words()Ù{Ù ɢ&p=ť ȡ ǯ=2*p.s;Õ h9=ß(p.ǽ*p.s+p.s)ĺ(ѥ())h9+=2 ӎ(p.Η)ĺ(p.Ǯ)h9+=(2*p.Ǯ+1)
ӎ(ǯ)Ū ß(μ())Я(ǯ)+Ŋ(ȵ(p.ɒ(k)),h9);}Õ ĸ()Ù{Ú Ŋ(quotient_words(),ɑ.buffers()ӎ(ɑ.î));}Õ č()Ù{Ú ˑ?ß(n):ɠ(ĸ());}};Ũ Bump{Ϳ Bump(ĝ i6):ю(i6){}ĝ ś(ǀ){if(used_>ю Û||ß(n)>ю Û-used_)throw Ø bad_alloc()ǫ s=ю Ė(used_,n);used_+=ß(n)Ū s;}ĝ ɼ()Ù{Ú ю Ė(used_);}Ⱦ ĝ ю;Õ used_ Ē};Ȅ ґ(н,ĭ Ȏ){ȱ,p=ɟ;ĥ Ú ǝ;}ԅ А(Ö x){Ú x;}ԅ А(Ě x)Щ Ȅ Β(Ɠ p){Ú ɂ(õ Ģ*>(p));}Ȅ Β(ǣ*p){Ά;ĭ ĳ;Ǵ(&ĳ,p,ƚ(ĳ))Ū ń(ĳ,kP);}í store_any(Ņ p,ɍ){Ψ(Ț(p),x);}Ȅ reversed(ɍ){Ú Ľ(x,ʞ);}í append_words(л,Ɠ Ƴ,ǀ,Ö Ÿ=1){Œ ŧ(Ÿ);ɵ ƒ(ʲ n Ɲ){ĭ ɽ=Β(Ƴ+i)ĺ(Ÿ!=1)ɽ Ҝ(ɽ,ŧ);Ě ŀ ɚ;Ǝ(ƚ(ŀ)==ƚ(ɽ)&&ƈ Ě>);Ǵ(ш(ŀ),&ɽ,ƚ(ŀ));į ӯ(į Ƃ,ŀ,ŀ+8);}Ć(ț)į θ(Ҿ(Ÿ у?Ƴ Ĕ:ʀ(Ƴ Ĕ,Ÿ)));}í Ј(л,Ɠ Ƴ,ǀ,Ö Ÿ=1){Ά ƣ ŧ(Ÿ);ǖ Ě,64>batch;ɵ ƒ(;i+64<=n ƽ 64){if(Ÿ у)Ǵ(ш(batch à),Ƴ+i,ƚ(batch))і{
#pragma GCC unroll 8
Ć(ǉ 64 Ӗ){Ģ ɽ Ҝ(Β(Ƴ+i+j),ŧ);Ǵ(ш(batch Ʒ j),&ɽ,ƚ(ɽ));}}į ӯ(į Ƃ,batch ü,batch Ƃ);}if(i<n)append_words(į,Ƴ+i,n-i,Ÿ);}ˬ í ʫ(Ù T*a,Ҡ,ȧ,Ƞ){ɵ ƒ(ʲ þ Ɲ)store_any(ѓ i,ɸ(Β(a+i),Β(b+i)))ø(Ͼ þ Į)į Ƈ(А(ɨ)+Ͻ Ϲ)Ұ;}ˬ í ʽ(ğ T>a,ř top,ȧ,Ƞ,Ɠ sub=Ƿ){Ù ř ä=ř(Ư)Ǎ ĳ=Ѯ(ɵ){Ù ř p=top-ř(i);Ġ x=p>=0&&p<ä?А(a[Õ(p)]):0 Ū sub?(x+Ͻ sub Ĕ)Ұ:x;};ɵ=top>=ä?Ħ(þ,Õ(top-ä+1)):0 ĺ(sub){Ć(ǉ i ų)į ˉ ĳ(j);}ŝ{ʇ(į,i,0);}Ć(ʲ þ&&top-ř(i)>=7 Ɲ){Ҽ reversed(Β(a Ʒ(top-ř(i)-7)));store_any(ѓ i,sub?ɸ(x,Β(sub+i)):x);}Ć(Ͼ þ Į)į Ƈ ĳ(i);}ˬ í ѷ(ğ T>a,ɐ){ģ ǯ=ǲ;ż(į ü,į Ƃ,0)ø(Õ ʝ Ē ʝ<Ư;ʝ+=ǯ){ģ þ Ш(ǯ,Ư-ʝ),full=þ/8*8 ĩ T*x=a Ʒ ʝ ø(ǉ full Ӗ){Š(ԃ j,ń(ƪ(Ō(ԃ j),Β(x+j)),kP));}Ć(ʍ=full;j<þ ų)į ˉ(į Ɓ+А(x Ɓ))Ұ;}}ˬ í ˑ(Ԅ T>f,Ğ g,ĝ q,ĝ r,ĝ ɼ){Ę ʀ ȡ d=ȸ-1 Ѓ(Ĥ T,Ö>)ȩ(f ü,f Ƃ,ɼ ü)і Ć(Ů Ǳ Į)ɼ Ƈ А(f Ĕ);Ġ Μ=á Ǹ(g[d],ӕ)ø(ɵ=Ѹ;i-->0;){Ġ c=Đ(ɼ[i+d],Μ);q Ƈ c ø(Ӆ<d ė t)ɼ[i+t]=(ɼ[i+t]+Ͻ Đ(c,g ѯ))Ұ;}ĵ(ɼ ü,r Û,r ü);}ˬ í remainder_direct(Ԅ T>f,Ğ g,Ğ q,ĝ r){ģ d=r Û,k=Ѹ;ĭ Ÿ[kDirectRemainder]ø(Ů k Į)Ÿ Ƈ _mm256_set1_epi64x(Ø int64_t((ŉ(q Ĕ)<<32)Ұ))Ǎ scalar=Ѯ(ʍ){ŉ ʂ ƒ(ɵ Ē i<=Ħ(j,k
-1)Į)ʂ=(ʂ+ŉ(q Ĕ)*g[j-i])Ұ;r ˉ Ö((А(f Ɓ)+Ͻ ʂ)Ұ);};ʍ ƒ(;j<Ħ(d,k)ų)scalar(j)ø(;j+8<=d Ӗ){ĭ o4=Β(f Ʒ j)ø(Ů k ƽ 4){н=đ,Ȏ=đ ø(Õ t=i;t<Ħ(k,i+4)ė t){Ҽ Β(g Ʒ(j-t));Ŧ=ð(Ŧ,Þ(x,Ÿ ѯ));Ȏ=ð(Ȏ,Þ(ʤ,Ÿ ѯ));}o4=ń(é(ƪ(o4,П),ґ(Ŧ,Ȏ)),2*kP);}store_any(r Ʒ j,ń(o4,kP));}Ć(;j<d ų)scalar(j);}ˬ í remainder_wrap(ƶ,Ԅ T>f,Ğ g,Ğ q,ĝ r,ҽ,ĝ b){ģ ǯ=Ư ĺ(Ѹ<=ǯ){t.ı(q,0,a);}ŝ{ѷ(q,a)Ȇ(a);}Ğ gl=g ĺ(ȸ>ǯ){ĵ(g ü,ǯ,b ü);b Г(b Ŭ+g[ǯ])Ұ;gl=b;}t.Ŷ(gl,0,b,a);ѷ(f,a);ʫ(ʰ,b à,r à,r Û);}ˬ í remainder_split(ƶ,Ԅ T>f,Ğ g,Ğ q,ĝ r,ҽ,ĝ b,ĝ c){ģ ǯ=Ư,k=Ѹ,e=r Û-ǯ Ȇ(q,0,a);t.Ŷ(g Ė(ǯ+1-k,k+e-1),0,c,a);ʫ(f Ʒ ǯ,c Ʒ(k-1),r Ʒ ǯ,e);ѷ(g,b);t.Ŷ(b,a);ѷ(f,c);ʫ(c à,b à,r à,ǯ);ʫ(r à,r Ʒ ǯ,r à,e);}Ą<Õ K>Ĉ ʼ(ƶ,Ԅ ɝ>ǜ,ɐ){ϙ<K>b9{}ø(Ů K Į)b9.Ơ Ƈ{{Ȱ,ɧ,ǜ Ĕ.b à},ǜ Ĕ.ʰ};t.Ɇ(b9,į,Ƨ);}í ʼ(ƶ,Ԅ ɝ>ǜ,ɐ){switch(ǜ Û){case 1:case 2:case 3:Ú t.Ŗ(ǜ,į,Ƨ)ϼ 4:Ú ʼ<4>(t,ǜ,į)ϼ 5:Ú ʼ<5>(t,ǜ,į)ϼ 6:Ú ʼ<6>(t,ǜ,į)ϼ 7:Ú ʼ<7>(t,ǜ,į)ϼ 8:Ú ʼ<8>(t,ǜ,į);default:ǩ;}}ˬ í subtract_reversed(Ù T*a,Ҡ,ȧ,Ƞ){ʍ ƒ(;j+8<=þ Ӗ)store_any(ѓ j,ɸ(Β(a+j),reversed(Β(b+þ-8-j))))ø(;j<þ ų)į ˉ(А(a Ɓ)+Ͻ b[þ-1-j])Ұ;}ˬ í divide_blocks
(ƶ,Ù Ν&plan,Ԅ T>f,Ğ g,ĝ q,ĝ r,Ѕ){Ù ɢ&p=Ƹ ť ȡ k=Ѹ,s=p.s,ǯ=2*s,d=Ƹ d ĩ ř top=ř(Ǳ-1+p.pad);Bump bump(ĸ)ǫ G=bump ņ(Ƹ μ()),ht=ɭ ӏ(g Ƃ-ř(G Û),g Ƃ,G ü)ȡ ɒ=p.ɒ(k);ć(t,G ȑ(Ħ(G Û,ɒ)),ht ȑ(ɒ),bump.ɼ())Ȇ(ht ȑ(ɒ),0,ht)ǫ qs=bump ņ(p.ǽ*s+s);ĝ gt,mp,Ǡ;ǖ ĝ,ϯ>Ǯ{},ring{}ĺ(Ƹ ѥ()){gt=bump ņ(p.Η),mp=bump ņ(p.Η)Ȇ(G,0,gt);}if(p.Ǯ){Ǡ=ɭ ø(Ů p.Ǯ Į){Ǯ Ƈ ɭ,ring Ƈ ɭ Ȇ(G Ė(i*s,Ħ(ǯ,G Û-i*s)),0,Ǯ Ĕ);}}ʦ tail_product=Ѯ(ʍ){t.Ŷ(qs Ė(j*s-d,d),p.Η-2*d,mp,gt,Ƨ)Ū mp Ʒ(p.Η-d);}ø(ǉ p.ǽ ų){Ǜ ŀ=qs Ʒ j*s ĩ ř ʝ=top-ř(j*s)ĺ(j ζ){ʽ(f,ʝ,ŀ,s);}м(p.ѥ){ʽ(f,ʝ,ŀ,d,tail_product(j));ʽ(f,ʝ-ř(d),ŀ+d,s-d);}ŝ{ģ Ơ Ш(j,d/s+1);ǖ ɝ,ϯ>ǜ ø(Ů Ơ Į)ǜ Ƈ{Ǯ Ĕ,ring[(j-1-i)%p.Ǯ]};ʼ(t,Ԅ ɝ>(ǜ à,Ơ),Ǡ);ʽ(f,ʝ,ŀ,s,Ǡ Ʒ s);}t.Ŷ(qs Ė(j*s,s),0,qs Ė(j*s,ǯ),ht,ƨ)ĺ(p.Ǯ&&j+1<p.ǽ)t.ı(qs Ė(j*s,s),0,ring[j%p.Ǯ]);ʽ(qs ȑ((j+1)*s),ř((j+1)*s-1),q Ʒ(p.ǽ-1-j)*s,j ζ?s-p.pad:s);}if(Ƹ ɑ.method==ǵ::kTail)subtract_reversed(f à,tail_product(p.ǽ),r à,d);}ˬ í divide(ě&ŋ,Ù Ν&plan,Ԅ T>f,Ğ g,ĝ q,ĝ r){if(Ƹ ˑ)Ú ˑ(f,g,q,r,Ĭ(Ǳ))ĩ Č t(ŋ,Ƹ lg)ǫ ĸ ф(Ƹ ĸ());divide_blocks(t,plan,f,g,q,r,ĸ);Bump bump(
ĸ)ȡ ǯ=Ƹ ɑ.î;switch(Ƹ ɑ.method){case ǵ::kDirect:Ú remainder_direct(f,g,q,r)ϼ ǵ::kWrap:Ú remainder_wrap(t,f,g,q,r,ɭ,ɭ)ϼ ǵ::kSplit:Ú remainder_split(t,f,g,q,r,ɭ,ɭ,ɭ)ϼ ǵ::kTail:Ú;}}}Ί divide_words(ǀ,Ό){Ú ƺ::Ν(n,m).č();}í divide(ě&ŋ,О,Ğ g,ĝ q,ĝ r){Ù ƺ::Ν plan(Ǳ,ȸ)ĺ(Ƹ k ζ){ȩ(f ü,f Ƃ,r ü)а}ƺ::divide(ŋ,plan,f,g,q,r);}}ā ç ē{í native_divide(ǣ*a,ǀ,ǣ*b,Ό,Ʋ&ť,Ʋ&ɑ){Ù ƺ::Ν plan(n,m)ȡ k=З(n,m),d=remainder_size(n,m)й(Ƹ č()Я(m)Я(k)Я(d))ĩ ˢ f(a,n)Ѵ g ф(m),q ф(k),r ф(d)ø(Ů m Į)g Ƈ Ħ(Ϲ.v,Ϲ.v-Ƀ);ƺ::divide(ŋ,plan,f,g,q,r);ƙ(ť,k),ƙ(ɑ,d);ƺ::Ј(ť,q à,Ѹ,á kR);ƺ::Ј(ɑ,r à,r Û);}í native_div_series(ǣ*a,Õ ʭ,ǣ*b,Õ μ,л,ǀ){Ę ā ƺ ȡ þ Ш(μ,n),ũ=þ-1;ɢ plan ø(Õ s=32;;s*=2){ɢ candidate(n,ũ,s)ĺ(candidate.ns<Ƹ ns)plan=candidate ĺ(s>=n||4*s>(ǻ Č::Ҕ))Ϸ}ģ s=Ƹ s,ǯ=2*s ĩ ƍ ѥ=Ƹ ѥ&&Ƹ ǽ>1 ȡ ɒ=Ƹ ɒ(n),j4 Ш(ʭ,n)ȉ lg=Ī(Ŋ(ǯ,ѥ?Ƹ Η:0))ȡ h9=ß(Ƹ ǽ*s+s)+(ѥ?2 ӎ(Ƹ Η):0)+(Ƹ Ǯ?(2*Ƹ Ǯ+1)ӎ(ǯ):0)й(ɠ(j4)Я(þ)Я(ǯ)Я(Ŋ(ȵ(ɒ),h9)))Ǭ f ф(j4),g ф(þ),ht ф(ǯ)ø(Ů j4 Į)f Ƈ Ħ(ɨ.v,ɨ.v-Ƀ)ø(Ů þ Į)g Ƈ Ħ(Ϲ.v,Ϲ.v-Ƀ)Ѵ ĸ ф(Ŋ(ȵ(ɒ),h9));ć(t,g ȑ(Ħ(þ,ɒ)),ht ȑ(ɒ),ĸ)Ȇ(ht ȑ(ɒ),0,
ht);Bump bump(ĸ)Ѵ qs=bump ņ(Ƹ ǽ*s+s);ĝ gt,mp,Ǡ;ǖ ĝ,ϯ>Ǯ{},ring{}ĺ(ѥ){gt=bump ņ(Ƹ Η),mp=bump ņ(Ƹ Η)Ȇ(g,0,gt);}if(Ƹ Ǯ){Ǡ=ɭ ø(Ů Ƹ Ǯ Į){Ǯ Ƈ ɭ,ring Ƈ ɭ Ȇ(g Ė(i*s,Ħ(ǯ,þ-i*s)),0,Ǯ Ĕ);}}ƙ(į,n)ø(ǉ Ƹ ǽ ų){ł ŀ=qs Ė(j*s,ǯ)ȡ ʝ=j*s,ɱ=ʝ<j4?Ħ(s,j4-ʝ):0 ĺ(ɱ)ĵ(f Ʒ ʝ,ɱ,ŀ à);ż(ŀ ʷ ɱ,ŀ ʷ s,0u)ĺ(j&&ѥ){t.Ŷ(qs Ė(j*s-ũ,ũ),Ƹ Η-2*ũ,mp,gt,Ƨ);ʫ(ŀ à,mp Ʒ Ƹ Η-ũ,ŀ à,ũ);}м(j){ģ Ơ Ш(j,ũ/s+1);ǖ ɝ,ϯ>ǜ ø(Ů Ơ Į)ǜ Ƈ{Ǯ Ĕ,ring[(j-1-i)%Ƹ Ǯ]};ʼ(t,Ԅ ɝ>(ǜ à,Ơ),Ǡ);ʫ(ŀ à,Ǡ Ʒ s,ŀ à,s);}t.Ŷ(ŀ ȑ(s),0,ŀ,ht,ƨ)ĺ(Ƹ Ǯ&&j+1<Ƹ ǽ)t.ı(ŀ ȑ(s),0,ring[j%Ƹ Ǯ]);Ј(į,ŀ à,Ħ(s,n-ʝ),á kR);}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
Ő T=Ě>ƛ Ď:ѝ,private ç ǅ{Ǝ(Ҳ==998244353,"FormalPowerSeries supports only mod 998244353")ͼ W=Ď<Ě>ͼ Ƽ=ѝ ͼ F=Ď ͼ Ƽ::Ƽ;Ď()=default;Ď(Ƽ v):Ƽ(ƅ(v)){}F&ŕ=(Ƽ v){Ƽ::ŕ=(ƅ(v))Ū ˈ;}F&ŕ=(Ø initializer_list<T>v){Ƽ::ŕ=(v)Ū ˈ;}ċ F Ź(ʧ a,ʧ b){if(a ű||b ű)Ú{};ƴ{W x=ź(a)Ū ƿ(&a==&b?W::Ź(x,x):W::Ź(x,ź(b)));}if(Ħ(Ư,η)<=32){F r(Ư+η-1)ĩ F&small=Ư<η?a:b ĩ F&h8=Ư<η?b:a;ġ{Ć(ȓ ó(small Û)ų)Ű add_scaled(č(r)+j,č(h8),h8 Û,small Ɓ ȃ);}ŝ Ć(ȓ ó(small Û)ų)Ć(ö ó(h8 Û)Į)r[i+j]+=h8 Ĕ*small Ɓ Έ}ǧ=1,ä=Ư+η-1 ǟ(n<ä)n<<=1;ơ(n ɰ);
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(Ű Ĝ)Ú F(ç ʉ::Ź(a,b));}
#endif
F x=a,y;x Ř(n)ĺ(&a!=&b){y=b;y Ř(n);}if(n>=64&&&a!=&b){ł&t=ƀ(n/16);t.convolution(č(x),č(y),n);x Ř(ä);t.ŧ(č(x),ä,ǘ(n/8))Ū x;}ntt(x)ĺ(&a==&b)ȇ(x,x,x,n)і{ntt(y);ȇ(x,x,y,n);}intt(x);x Ř(ä)Ū x;}F ŕ-()Ù{F r=ˈ ø(T&x:r)x=-x Έ}F&ŕ+=(ʧ g){Ԕ+=g Ĕ Ū ˈ;}F&ŕ-=(ʧ g){Ԕ-=g Ĕ Ū ˈ;}F&ŕ+=(T x){if(Ƕ empty())Ƕ resize(1);(ˈ)Ŭ+=x Ū ˈ;}F&ŕ-=(T x){Ú ˈ+=-x;}F&ŕ*=(ʧ g){Ú ˈ=Ź(ˈ,g);}F&ŕ*=(T x){ġ{Ζ(ˈ,x)Ū ˈ;}Ć(T&a:ˈ)a*=x Ū ˈ;}F&ŕ/=(ʧ g){Ú ˈ=Α(g);}F&ŕ%=(ʧ g){Ú ˈ=divmod(g).ψ;}F&ŕ/=(T x){Ú ˈ*=x ˆ;}F&ŕ<<=(ý d){ơ(d>=0)ĩ ǧ=ϧ ĺ(d>=n)Ú ˈ=F(n);Ƕ insert(Ƕ Ă,d,Ȓ);Ƕ resize(n)Ū ˈ;}F&ŕ>>=(ý d){ơ(d>=0)ĩ ǧ=ϧ ĺ(d>=n)Ú ˈ=F(n);Ƕ erase(Ƕ Ă,Ƕ Ă+d);Ƕ resize(n)Ū ˈ;}F ŕ+(ʧ g)Ù{ѕ+=g Έ}F ŕ-(ʧ g)Ù{ѕ-=g Έ}F ŕ*(ʧ g)Ù{Ú Ź(ˈ,g);}F ŕ/(ʧ g)Ù{Ú Α(g);}F ŕ%(ʧ g)Ù{Ú divmod(g).ψ;}F ŕ+(T x)Ù{ѕ+=x Έ}F ŕ-(T x)Ù{ѕ-=x Έ}F ŕ*(T x)Ù{ѕ*=x Έ}F ŕ/(T x)Ù{ѕ/=x Έ}F ŕ<<(ý d)Ù{ѕ<<=d Έ}F ŕ>>(ý d)Ù{ѕ>>=d Έ}F pre(ǧ)Ù{ơ(n>=0)Ū F(Ƕ Ă,Ƕ Ă+ʡ(n,ǰ));}F Α(ʧ g,ў)Ù{ƴ Ú ƿ(ź(ˈ).Α(ź(g),ǁ))Н ǰ;ơ(ǁ>=0)ĺ(!ǁ)Ú{};ơ(!g ű&&g Ŭ!=Ȓ)ĺ(Ƕ empty())Ú F(ǁ);
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(ǁ>=129&&ǁ ɰ&&ȸ>=64 ȏ){F r;ç ʃ native_div_series(Ƕ è,ǰ,g à,ȸ,r,ǁ)Έ}}
#endif
if(ǁ<=128||ȸ<64){F r=Ź(pre(ǁ),g.ё(ǁ)).pre(ǁ);r ә Έ}ϕ ĺ(ǁ<ä-ä/4)ä>>=1 ȉ ŀ=ä/32,î=2*ŀ,k4=î ү,þ=(ǁ+ŀ-1)/ŀ;F ć=g.ё(ŀ),r=Ź(pre(ŀ),ć).pre(ŀ);r ә ȹ d8=ʟ Ӳ,*ť=d8+þ*k4 ø(ö þ Į){ɓ h=d8+i*k4 ȉ ɱ=Ŋ(0,ʡ(ŀ,Џ-i*ŀ))ĺ(ɱ)ĵ(č(g)+i*ŀ,ɱ,h);ż(h+ɱ,h+ŀ,0u);ʒ(h,î);}ǆ Ц(d8,î,þ,k4);Ƽ Ƌ(î)ø(λ þ Į){ȟ Ų=i*ŀ,n Ш(ŀ,ǁ-Ų);ɓ q=ť+(i-1)*k4 Ԗ(č(r)+Ų-ŀ,ŀ,q);ʒ(q,î);Ű scale_normalize(q,î,norm)ø(ȓ i;j+=7){Ù ɓ a[7],*b[7];ý Ơ Ш(7,i-j)ø(ˋ Ơ ȝ){a ђ ť+(j+k)*k4;b ђ d8+(i-j-k)*k4;}ǐ}Ʊ(Ƌ)ø(ȓ ŀ ų)Ƌ ˉ(Ų+j<ϧ?(ˈ)[Ų+j]:Ȓ)-Ƌ Ɓ Ђ ĵ(Ƌ ü,n,r ʷ Ų);}Ú r;}F ё(ў)Ù{ƴ Ú ƿ(ź(ˈ).ё(ǁ))ĺ(ǁ<0)Ϋ(!Ƕ empty()&&(ˈ)Ŭ!=Ȓ);F g{(ˈ)Ŭ ˆ}Ѵ Ơ=sparse_terms(ǁ,32);Ӄ=Ơ Û<=32?Ѫ(Ơ):1 ĺ(Ơ Û<=32&&(ǁ<=256||Ơ Û<=12ULL Ԏ)){g ә ĺ(Ơ Û у&&ď у){ż(g ü,g Ƃ,g Ŭ);ɘ(g,Ӹ,-Ơ Ŭ.ψ*g Ŭ)Ū g;}Ć(ǈ i=ď;ď&&i<ǁ ƽ ď)g Ƈ-sparse_dot(Ơ,g,ó(i))*g Ŭ Ū g;}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(ǁ>=512&&ǁ ɰ ȏ){F ă ȉ ŀ=1<<(ʌ(ú ī>(ǁ))-1),Η=ǁ-ŀ ĺ(Η>0&&Η<=96){ç ʃ native_inverse(Ƕ è,ǰ,ă,ŀ);ă ә ø(Ż ŀ Ͼ ǁ Į)ă Ƈ-ă Ŭ*dot(Ƕ è+1,ă Ȝ-1,ʡ(i,ǰ-1))Ȧ}ç ʃ native_inverse(Ƕ è,ǰ,ă,ǁ)Ȧ}}
#endif
if(ǁ>256&&ǰ>=64){
#if defined(__x86_64__) || defined(__i386__)
if(ǁ<1024||!Ű Ĝ)Ú F{1}.Α(ˈ,ǁ);
#else
Ú F{1}.Α(ˈ,ǁ);
#endif
}g Ř(Ħ(ǁ,32))ø(λ Џ Į)g Ƈ-g Ŭ*dot(Ƕ è+1,g Ȝ-1,ʡ(i,ǰ-1));
#if defined(__x86_64__) || defined(__i386__)
ç FpsWords ʟ;ɓ a=Ƿ,*b=Ƿ ĺ(ǁ>96 ȏ){ý ε=64 ǟ(ε<ǁ&&ε<(1<<23))ε<<=1;ʟ=ç ɀ(2*ε);a=ʟ Ӳ;b=a+ε;g.reserve(ε);ƀ(Ŋ(32,ε/16));}
#endif
while(Џ<ǁ){ȟ m=Џ,n=2*m ĺ(m>=32&&(ǁ-m<=32||(m>=512&&ǁ-m<=96))){g ә ø(Ż m Ͼ ǁ Į){ȟ þ=ʡ(i,ǰ-1);g Ƈ-g Ŭ*dot(Ƕ è+1,g Ȝ-1,þ);}Ϸ}
#if defined(__x86_64__) || defined(__i386__)
if(m>=64&&n ɰ ȏ){ý lo=ʡ(n,ǰ)Ԗ(č(ˈ),lo,a);ż(a+lo,a+n,Word(0))Ԗ(č(g),m,b);ƀ(Ŋ(1,n/16)).inverse_newton8(a,b,n);g Ř(n)Ԗ(a+m,m,č(g)+m);}ŝ
#endif
{Ƽ f=Ƕ pre(n),r=g;f Ř(n);r Ř(n);ntt(f);ʒ(r);ȇ(f,f,r,n);Ʊ(f);ż(f ü,f ʷ m,Ȓ);ntt(f);ȇ(f,f,r,n);Ʊ(f)ĩ T ё=T ι(ǘ n)ĩ T ŧ=-ё*ё;g Ř(n);ƀ(1).ŧ(č(f)+m,m,ŧ ȃ)Ԗ(f ʷ m,m,g ʷ m);}}g ә Ū g;}F sqrt(ў)Ù{ƴ Ú ƿ(ź(ˈ).sqrt(ǁ))Н ϧ;ơ(ǁ>=0);ý Ȣ=ó(Ħ<Õ>(ǁ,ǰ)),ù Ē while(ù<Ȣ&&(ˈ)[ù]Ү)++ù ĺ(ù==Ȣ)Ú F(ǁ)ĺ(ù&1)Ú{}ĩ T ˏ=mod_sqrt((ˈ)[ù])ĺ(ˏ Ү)Ú{}ǟ((ˈ)[Ȣ-1]Ү)--Ȣ ĺ(ҵ==Ȣ){ѫ;ă[ù/2]=ˏ Ȧ}ȟ ƌ=ù/2,Ʃ=ǁ-ƌ;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(Ʃ>=256&&Ʃ ɰ ȏ){F ă;ç ʃ native_sqrt(Ƕ è+ù,Ȣ-ù,ă,Ʃ,ˏ);ă ӯ(ă ü,ƌ,Ȓ)Ȧ}}
#endif
F f(Ƕ Ă+ù,Ƕ Ă+Ȣ);f Ř(Ʃ)ĩ T Ɵ=T(2)ˆ,ŧ=Ɵ/ˏ;F g(Ħ(Ʃ,32));g Г ˏ ø(λ Џ Į){T ʂ ƒ(ý j=1;j<i ų)ʂ+=g Ɓ*g[i-j];g Ƈ(f Ĕ-ʂ)*ŧ;}if(Ʃ>256){ý ä=1 ǟ(ä<Ʃ)ä<<=1 ĺ(Ʃ<ä-ä/4)ä>>=1 ȉ ŀ=ä/32,î=2*ŀ,k4=î ү,þ=(Ʃ+ŀ-1)/ŀ;g=f.pre(ŀ).sqrt(ŀ);Ƽ ć=g.ё(ŀ);ć Ř(î);ʒ(ć)ĩ Word f7=ǘ(2*î)ȉ ţ=__builtin_ctz((ϔ)î);T l9=T ι(ǘ(1<<(ţ/2)))ĺ(ţ&1)l9*=T ι(441024591);ƀ(1).ŧ(č(ć),î,f7)Ѵ ʟ=ç ɀ((þ-1)*k4);ɓ Ǥ=ʟ Ӳ;Ƽ Ƌ(î);Ƽ zero(î);g Ř(Ʃ)ø(λ þ Į){ý Ų=i*ŀ,n Ш(ŀ,Ʃ-Ų)Ϗ(q,î,l9 ȃ)ø(ȓ(i+1)/2;j+=6){Ù ɓ a[6],*b[6],*c[6];ý Ơ Ш(6,(i+1)/2-j)ø(ˋ Ơ ȝ){a ђ Ǥ+(j+k)*k4;b ђ(j+k?Ǥ+(i-j-k)*k4:č(zero));c ђ Ǥ+(i-j-k-1)*k4;}ł update=Ѯ(ł number){æ ý þ=decltype(number)::ĳ ĺ(j ζ){ά,Ɔ,Ƅ(č(Ƌ),a,b,c,ŀ,Ś);ά,Ɔ,Ƅ(č(Ƌ)+ŀ,a,b,c,ŀ,Ɔ);}ŝ{ά,Ś,Ƅ(č(Ƌ),a,b,c,ŀ,Ś);ά,Ś,Ƅ(č(Ƌ)+ŀ,a,b,c,ŀ,Ɔ);}};Ĩ}Ű square_fold(č(Ƌ),Ǥ+(i/2)*k4,ŀ,i&1);Ʊ(Ƌ)ø(ȓ ŀ ų)Ƌ ˉ(Ų+j<Ʃ?f[Ų+j]:Ȓ)-Ƌ Ɓ Ђ ĵ(Ƌ ü,n,g ʷ Ų);}g ӯ(g ü,ƌ,Ȓ)Ū g;}F h;Ƽ Ǥ ĺ(Џ<Ʃ){h=g.pre(16).ё(16);Ǥ=h;Ǥ Ř(32);ntt(Ǥ);}while(Џ<Ʃ){ȟ m=Џ,n=2*m ĺ(Ʃ-m<=32){g Ř(Ʃ)ø(Ż m Ͼ Ʃ Į)g Ƈ(f Ĕ-dot(g Ʒ 1,g Ȝ-1,i-1))*ŧ;Ϸ}ơ(n ɰ)ĩ T in=T ι(ǘ n);Ƽ a=g,π ȇ(Ƌ,a,a,n);Ʊ(Ƌ)
;ż(Ƌ ü,Ƌ ʷ m,Ȓ)ø(Ż m ț)Ƌ Ƈ(i<Ʃ?f Ĕ:Ȓ)-Ƌ Ĕ*in;ntt(Ƌ);ȇ(Ƌ,Ƌ,Ǥ,n);F::Ζ(Ƌ,Ɵ);Ʊ(Ƌ);g Ř(Ħ(n,Ʃ))ø(Ż m Ͼ Џ Į)g Ƈ Ƌ Ĕ*in;}g Ř(Ʃ);g ӯ(g ü,ƌ,Ȓ)Ū g;}T eval(T x)Ù{ġ Ú ç horner(Ƕ è,ǰ,x);T ans ƒ(ł it=Ƕ rbegin();it!=Ƕ rend()ė it)ans=ans*x+*it Ū ans;}Ĉ onemul(ý d,T c){ơ(d>=0)ĺ(d ζ){ˈ*=Ӹ+c а}Ć(Ż(ϧ)-1;i>=d;--i)(ˈ)Ĕ+=(ˈ)[i-d]*c;}Ĉ onediv(ý d,T c){ơ(d>=0)ĺ(d ζ){ˈ/=Ӹ+c а}Ć(Ż d Ͼ ϧ Į)(ˈ)Ĕ-=(ˈ)[i-d]*c;}F Ǚ()Ù{Ù ǧ=ϧ;F r(n);ġ{if(n>1)Ű weighted(č(r),č(ˈ)+1,ԕ,1);}ŝ Ć(Ż 1 ț)r[i-1]=(ˈ)Ĕ*i Έ}F integral()Ù{Ù ǧ=ϧ;ơ(ī(n)<ī(Ҳ));F r(n)Ǎ&ё=inverses(n);ġ{if(n>1)Ű Ŏ(č(r)+1,č(ˈ),č(ё)+1,ԕ);}ŝ Ć(Ż 1 ț)r Ƈ(ˈ)[i-1]*ё Ĕ Έ}F log(ў)Ù{ƴ Ú ƿ(ź(ˈ).log(ǁ))ĺ(ǁ<0)Ϋ(!Ƕ empty()&&(ˈ)Ŭ==Ӹ)Ū Γ(ǁ,Ɔ);}F exp(ў)Ù{ġ Ú exp_impl ə(ǁ)і Ú ƿ(ź(ˈ).exp(ǁ));}F pow(ǈ k)Ù{ƴ Ú ƿ(ź(ˈ).pow(k));ơ(k>=0)ĩ ǧ=ϧ ĺ(n ζ)Ú{}ĺ(k ζ){F r(n);r Г 1 Έ}if(k у)Ú ˈ ĺ(k<=4){F n3=Ź(ˈ,ˈ).pre(n)Ū k==2?n3:Ź(n3,k==3?ˈ:n3).pre(n);}ý ù Ē while(ù<n&&(ˈ)[ù]Ү)++ù ĺ(ù==n||ù>(ԕ)/k)Ú F(n)ȉ ƌ=ù*k ȉ ǯ=n-ƌ ĩ T Μ=(ˈ)[ù];
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(ǯ>=256&&ǯ<=(1<<24)ȏ){F r;ç ʃ native_power(Ƕ è+ù,ǰ-ù,r,ǯ,T(k)ȃ,Μ.pow(k));r ӯ(r ü,ƌ,Ȓ)Έ}}
#endif
F f(Ƕ Ă+ù,Ƕ end());f Ř(ǯ);f=f.Γ(ǯ);f*=T(k);f=f.Ą exp_impl ԏ(ǯ);f*=Μ.pow(k);F r(n)ø(ö ǯ Į)r[ƌ+i]=f Ĕ Έ}Ø pair<F,F>divmod(ʧ μ)Ù{ƴ{ł[q,r]=ź(ˈ).divmod(ź(μ))Ū{ƿ(q),ƿ(r)};}ǧ=ϧ,m=ó(μ Û)ǟ(n&&(ˈ)[ԕ]Ү)--n ǟ(m&&μ[m-1]Ү)--m;ơ(m)ĺ(n<m)Ú{{},pre(n)}ȉ qn=n-m+1;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(m>2&&(Õ)m*qn>4096&&n ɰ ȏ){F q,r;ç ʃ native_divide(Ƕ è,n,μ à,m,q,r);q.trim(),r.trim()Ū{ƅ(q),ƅ(r)};}}
#endif
F a=pre(n)ĺ(m<=2||(Õ)m*qn<=4096){F q(qn)ĩ T ё=μ[m-1]ˆ ø(Ż qn-1;i и i){q Ƈ a[i+m-1]*ё ø(ȓ m ų)a[i+j]-=q Ĕ*μ Ɓ;}a Ř(m-1)ϐ}F ra=a,rb(μ ü,μ ʷ m);Ø ˁ(ra ü,ra Ƃ);Ø ˁ(rb ü,rb Ƃ);F q=ra.pre(qn).Α(rb,qn);Ø ˁ(q ü,q Ƃ);Ø ˁ(rb ü,rb Ƃ);ý ҩ=1 ǟ(ҩ<Ŋ(m,qn))ҩ<<=1 ĺ(Ħ(m-1,qn)>32&&ҩ<m+Ħ(m-1,qn)-2){ơ(ҩ ɰ);F Ŏ=rb,ť=q;Ŏ Ř(ҩ);ť Ř(ҩ)Ѵ&t=ƀ(ҩ/16);t.convolution(č(Ŏ),č(ť),ҩ);t.ŧ(č(Ŏ),m-1,ǘ(ҩ/8))ø(ö m-1 Į){ɨ-=Ŏ Ĕ ĺ(i+ҩ<ó(Ư))ɨ+=a[i+ҩ];}a Ř(m-1)ϐ}F Ŏ=Ź(rb.pre(m-1),q.pre(m-1));a Ř(m-1)ø(ö ʡ(Ư,Ŏ Û)Į)ɨ-=Ŏ Ĕ ϐ}Ⱦ Ƙ ˤ ź ˤ ƿ ˤ ƀ ͼ Ȯ ˤ Җ ˤ ntt ˤ ʒ ˤ Ʊ ˤ Ζ ˤ ɘ ˤ batch_inverse ˤ ȇ ˤ inverse_extend ˤ dot ˤ Ѫ ˤ sparse_dot ˤ intt ˤ mod_sqrt ˤ inverse_ntt_roots ˤ ntt_roots ˤ inverses ͼ Word=ӈ ǆ Word;F Γ(ý ǁ,ƍ g7=Ś)Ù{
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(ǁ>=256&&ǁ<=(1<<24)&&ǰ>=64 ȏ){F r;ç ʃ native_log(Ƕ è,ǰ,r,ǁ,g7)Έ}}
#endif
F Ǟ(ʡ(ǁ,ǰ))ĺ(Ǟ Û>1)Ű weighted(č(Ǟ)+1,č(ˈ)+1,Ǟ Û-1,1);F r=Ǟ.Α(ˈ,ǁ)ĺ(g7){ʦ&iv=inverses(ǁ-1);Ű Ŏ(č(r),č(r),č(iv),ǁ);}Ú r;}ή ŵ>F exp_impl(ý ǁ)Ù{if(ǁ<0)Ϋ(Ƕ empty()||(ˈ)Ŭ Ү)Ѵ Ơ=sparse_terms(ǁ,64);Ӄ=Ơ Û<=64?Ѫ(Ơ):1 ĺ(Ơ Û<=64&&(ǁ<=256||Ơ Û<=24ULL Ԏ)){F g(ǁ);g Г 1 ĺ(Ơ Û у&&ď у&&ǁ>=256){Ƽ ԓ(ǁ);ç Ө(č(ԓ),č(g),ǁ);ɘ(g,Ӹ,Ơ Ŭ.ψ)Ū g;}if(!ď)Ú g Ǎ&iv=inverses((ǁ-1)/ď);T Ÿ=ŵ?T(ď)ˆ:Ӹ ø(ł&[j,x]:Ơ)x*=ŵ?Ÿ:T(j/ď);Ѫ(Ơ)ø(ǈ i=ď Ͼ ǁ ƽ ď)g Ƈ sparse_dot(Ơ,g,ó(i))*iv[i/ď]Ū g;}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(ǁ>=256&&ǁ<=(1<<24)ȏ){F r;ç ʃ native_exp(Ƕ è,ǰ,r,ǁ,ŵ)Έ}}
#endif
ʦ&iv=inverses(ǁ)ȉ g4 Ш(ǁ,32);F g(g4);g Г 1 ø(λ g4 Į){T ʂ ƒ(ý j=1;j<=i&&j<ϧ ų)ʂ+=(ŵ?(ˈ)Ɓ:T(j)*(ˈ)Ɓ)*g[i-j];g Ƈ ʂ*iv Ĕ;}if(g4==ǁ)Ú g ĺ(ǁ>256){ϕ ĺ(ǁ<ä-ä/4)ä>>=1 ȉ ŀ=ä/32,î=2*ŀ,k4=î ү,þ=(ǁ+ŀ-1)/ŀ;g=pre(ŀ).Ą exp_impl<ŵ>(ŀ);Ƽ ć=g.ё(ŀ)ȹ Ǟ=ʟ Ӳ,*Ǥ=Ǟ+þ*k4 ø(ö þ Į){ɓ d=Ǟ+i*k4 ȉ Ų=i*ŀ ȉ ɱ=Ŋ(0,ʡ(ŀ,ʡ(ǁ,ǰ)-Ų))ĺ(ɱ){Ĺ(ŵ)ĵ(č(ˈ)+Ų,ɱ,d)і Ű weighted(d,č(ˈ)+Ų,ɱ,Ų);}ż(d+ɱ,d+ŀ,0u);ʒ(d,î);}ǆ Ц(Ǟ,î,þ,k4);Ƽ Ƌ(î);g ә ø(λ þ Į){ý Ų=i*ŀ,n Ш(ŀ,ǁ-Ų)Ϗ(q,î,norm)ø(ȓ i;j+=7){Ù ɓ a[7],*b[7];ý Ơ Ш(7,i-j)ø(ˋ Ơ ȝ){a ђ Ǥ+(j+k)*k4;b ђ Ǟ+(i-j-k)*k4;}ǐ}Ʊ(Ƌ);ż(Ƌ ʷ ŀ,Ƌ Ƃ,Ȓ)Ђ Ű Ŏ(č(Ƌ),č(Ƌ),č(iv)+Ų,n);ż(Ƌ ʷ n,Ƌ Ƃ,Ȓ);ʒ(Ƌ);Ű Ŏ(č(Ƌ),č(Ƌ),Ǥ,î);Ʊ(Ƌ)Ԗ(Ƌ ü,n,g ʷ Ų);}Ú g;}F h=g.pre(16).ё(16);Ƽ Ǥ=h;Ǥ Ř(32);ntt(Ǥ)ǟ(Џ<ǁ){ȟ m=Џ,n=2*m ĺ(ǁ-m<=32){Ƽ Ǟ(ʡ(ǁ,ǰ))ø(λ ó(Ǟ Û)Į)Ǟ Ƈ ŵ?(ˈ)Ĕ:T(i)*(ˈ)Ĕ;Ǟ Ř(Ŋ<ý>(1,Ǟ Û));g ә ø(Ż m Ͼ ǁ Į){ȟ þ=ʡ(i,Ǟ Û-1);g Ƈ dot(Ǟ Ʒ 1,g Ȝ-1,þ)*iv Ĕ;}Ϸ}ơ(n ɰ)ĩ T im=T ι(ǘ m)ĩ T in=T ι(ǘ n);Ƽ a=g,b(m),π ż(b ü,b Ƃ,Ȓ)ø(λ m&&i<ϧ Į)b[i-1]=ŵ?(ˈ)Ĕ:T(i)*(ˈ)Ĕ;ntt(b);ȇ(b,b,a,m);Ʊ(b);Ƌ[m-1]=b[m-1]*im ø(ö m-1 Į)Ƌ[m+i]=Ϲ*im-T(i+1)*g Υ;ntt(Ƌ);ȇ(Ƌ
,Ƌ,Ǥ,n);Ʊ(Ƌ)ø(Ż(Ħ(n,ǁ))-1;i>=m;--i)Ƌ Ƈ(i<ϧ?(ˈ)Ĕ*(ŵ?iv Ĕ:Ӹ):Ȓ)+Ƌ[i-1]*in*iv Ĕ;ż(Ƌ ü,Ƌ ʷ m,Ȓ);ż(Ƌ ʷ Ħ(n,ǁ),Ƌ Ƃ,Ȓ);ntt(Ƌ);ȇ(Ƌ,Ƌ,a,n);Ʊ(Ƌ);g Ř(Ħ(n,ǁ))ø(Ż m Ͼ Џ Į)g Ƈ Ƌ Ĕ*in;}Ú g;}ħ Ø pair<ý,T>>sparse_terms(ý ǁ,ý i2)Ù{ħ Ø pair<ý,T>>Ơ ø(λ ʡ(ǁ,ǰ)&&ó(Ơ Û)<=i2 Į)if((ˈ)Ĕ!=Ȓ)Ơ θ(i,(ˈ)Ĕ)Ū Ơ;}Ĉ trim(){ɞ(ˈ);}}ͼ fps=Ď<Ě>;
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā ç bostan_detail{í inverse_twisted(Ù NTT&plan,Ù ҿ in,ҿ į,Ù ҿ ж,ǧ){Ę N=NTT ĺ(n==8){ł z=Ƹ Σ 4,Ƅ(Ƹ Σ 2,Ƅ(Ƹ Σ 1,Ƅ(Ӈ(in),0),0),0);N::ŗ(į,N::ɏ(z,Ӈ(ж)))а}Ć(ȓ n Ӗ){ł z=Ӈ(in+j);z=Ƹ Σ 4,Ƅ(Ƹ Σ 2,Ƅ(Ƹ Σ 1,Ƅ(z,j/8),j/8),j/8);N::ŗ(ѓ j,z);}ý h=8 ø(;4*h<n;h*=4)Ƹ radix4_8 ԏ(į,h,n)ĺ(4*h==n){ʦ im ʖ(Ƹ ô ƹ)Ǎ iq ʖ(Ƹ ſ ƹ)ø(ö h Ɲ){ʦ x0=Ӈ(ѓ i),x1=Ӈ(ѓ h+i)Ǎ x2=Ӈ(į ʳ h+i),x3=Ӈ(ѓ 3*h+i)Ǎ u=N::ɾ(x0,x1),v=N::sub8(x0,x1),s=N::ɾ(x2,x3)Ǎ t=N::ƥ(N::sub8(x2,x3),im,iq);N::ŗ(ѓ i,N::ɏ(N::ɾ(u,s),Ӈ(ж+i)));N::ŗ(ѓ h+i,N::ɏ(N::ɾ(v,t),Ӈ(ж+h+i)));N::ŗ(į ʳ h+i,N::ɏ(N::sub8(u,s),Ӈ(ж ʳ h+i)));N::ŗ(ѓ 3*h+i,N::ɏ(N::sub8(v,t),Ӈ(ж+3*h+i)));}}ŝ{Ć(ö h Ɲ){ʦ x=Ӈ(ѓ i),y=Ӈ(ѓ h+i);N::ŗ(ѓ i,N::ɏ(N::ɾ(x,y),Ӈ(ж+i)));N::ŗ(ѓ h+i,N::ɏ(N::sub8(x,y),Ӈ(ж+h+i)));}}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ T bostan_mori(Ȼ p,Ȼ q,ǈ k){Ѽ ͼ Ƽ=ѝ;Ƙ ͼ Word=ӈ ç ǅ::Word;ƴ{Ú T ι(bostan_mori<Ě>(Һ(p),Һ(q),k)ȃ);}ŝ{ơ(k>=0&&!q ű&&q Ŭ!=Ȓ);ɞ(p);ɞ(q)ĺ(p ű)Ú 0 Ѵ ʑ=[](Ҥ p,Ҥ q,ң){F ć(q ü,q ʷ ʡ(k+1,Ѹ));ć=ć.ё(k+1)Ū ǆ dot(p à,ć Ʒ k,ʡ(k+1,p Û));}ĺ(k<32)Ú ʑ(p,q,k);T b3 Ē if(p Û>=Ѹ){ł[ť,ɑ]=p.divmod(q)ĺ(k<(ǈ)ť Û)b3=ť ƫ;p=ƅ(ɑ);}if(p ű)Ú b3 ĺ(Ѹ==2)Ú b3+p Ŭ/q Ŭ*(-q ƹ/q Ŭ).pow(k)ȉ d=ó(Ѹ)-1 ĺ(k<d)Ú b3+ʑ(p,q,k);ǧ=1 ǟ(n<d)n<<=1;ơ(n<=(1<<22))ĩ T Ɵ=T(2)ˆ,ё=T(n)ˆ;T d4=1 ĩ T ɯ=T(3).pow((Ҳ-1)/(2*n));Ƽ ж(n),twists(n)Ѵ ʟ=ç ɀ(4*Õ(n));ɓ a=ʟ Ӳ,*b=a ʳ n Ԗ(Ȯ(p),p Û,a);ż(a+p Û,a ʳ n,0u)Ԗ(Ȯ(q),Ѹ,b);ż(b+Ѹ,b ʳ n,0u)Ѵ&plan=ǆ ƀ(n)ø(Е)Ȯ(twists)Ƈ Ƹ ſ Ĕ*(0u-Ҳ);T Μ=q Ӱ;ż(ж ü,ж Ƃ,Ӹ);ǆ ɘ(ж,ё,ɯ);Ƹ Ą Ҭ(a,2*n);Ƹ Ą Ҭ(b,2*n)Ѵ doubling=Ѯ(ɓ v,T i0){ɓ Ȏ=v+n;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(n>=8 ȏ){ç bostan_detail::inverse_twisted(plan,v,Ȏ,Ȯ(ж),n);Ȏ Г Ű sub(Ȏ Ŭ,(T(2)*i0).v);}ŝ
#endif
{ĵ(v,n,Ȏ);Ƹ Ą fft ԏ(Ȏ,n);Ȏ Г Ű sub(Ȏ Ŭ,(T(2*n)*i0).v);Ű Ŏ(Ȏ,Ȏ,Ȯ(ж),n);}Ƹ Ą Ҭ(Ȏ,n);}ǟ(k){if(k&1)Ű reduce_pairs ԏ(a,b,Ȯ(twists),n)і Ű reduce_pairs ə(a,b,Ȯ(twists),n);d4*=Ɵ;Μ=(d&1?-Μ:Μ)*Μ;k>>=1 ĺ(k<d)Ϸ doubling(a,Ȓ);doubling(b,d==n?Μ:Ȓ);}if(k){Ƽ aa(n),bb(n)Ԗ(a,n,Ȯ(aa))Ԗ(b,n,Ȯ(bb));ǆ intt(aa);ǆ intt(bb)ĺ(d==n)bb Ŭ-=Μ Ū b3+d4*ʑ(aa,bb,k);}T j4=0,d8 ƒ(Е)j4+=T ɪ(ɨ),d8+=T ɪ(Ϲ)ĺ(d==n)d8-=T(n)*Μ Ū b3+d4*j4/d8;}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ӂ{ā Ȗ{Ę ʃ ě ͼ ʃ Č ͼ ʃ Half ͼ ʃ ϵ;ā kernels{Ę ĭ=ϸ;ā column_detail=ʃ ê;ή Inverse>Ɩ Ĉ İ(ҳ a,Ϫ,Õ ҕ,Ɠ x,Ɠ y){Ę ā column_detail ƣ wx(x Ŭ,x ɚ),wy(y Ŭ,y ɚ),wz(y ƹ,y[9])Ǖ ǳ{Ģ f0=a Ɓ,f1=a ǹ,f2=a ɋ,f3=a Ɋ Ѓ(Inverse){Ģ ab ӑ(ƪ(f0,f1)),cd ӑ(ƪ(f2,f3))κ amb Ҝ(Ǚ(f0,f1),wy),cmd Ҝ(Ǚ(f2,f3),wz);a ˉ ʗ(ϓ),a ǹ ӑ(ϒ);a ɋ Ҝ(Ќ,wx),a Ɋ Ҝ(ϑ,wx);}ŝ{Ģ lo ӑ(f0),hi ӑ(f1),xc Ҝ(f2,wx),xd Ҝ(f3,wx)κ ac ӑ(ƪ(lo,xc)),amc=low_difference(lo,xc)κ bd Ҝ(ƪ(hi,xd),wy),bmd Ҝ(Ǚ(hi,xd),wz);a ˉ ƪ(ac,bd),a ǹ=Ǚ(ac,bd);a ɋ=ƪ(amc,bmd),a Ɋ=Ǚ(amc,bmd);}}}í ԋ(ҳ a,Ϫ,Õ ҕ,Ɠ x,Ɠ y){İ ə(a,h,ҕ,x,y);}í ǔ(ҳ a,Ϫ,Õ ҕ,Ɠ x,Ɠ y){İ ԏ(a,h,ҕ,x,y);}í forward_even_columns(ҳ a,Ϫ,Ɠ x,Ɠ y){İ ə(a,h,1,x,y);}í inverse_even_columns(ҳ a,Ϫ,Ɠ x,Ɠ y){İ ԏ(a,h,1,x,y);}}Ϧ{Ę ā ʃ ê ͼ ʃ â ŭ;ƛ ƻ Ǔ Э}}ā Ȗ{Ϧ{ò ɿ Ʉ=32;Ί compose_length(ǀ){Ú ɬ(ʘ(n,256));}í compose_direct(О,Ğ g,ĝ h){ģ n=h Û;ż(h ü,h Ƃ,0)ø(ɵ Ш(Ǳ,n);i-->0;){Ć(Ȕ=n;k-->1;){ŉ ʂ ƒ(ʍ=1;j<=k&&j<ȸ ų)ʂ+=ŉ(g Ɓ)*h[k-j]Ұ;h ђ Ö(ʂ Ұ);}h Г f Ĕ;}}ӄ Ґ(Ö x){Ġ y=x?2*ӕ*x:0 Ū y>=kP?y-kP:y;}Ȅ Ӥ(ɍ){Ú ą(é(ɟ,x),é(đ,x));}Ȅ even_lanes(ɍ,ĭ y){Ģ t
=_mm256_castps_si256(_mm256_shuffle_ps(р(x),р(y),0x88))Ū ҭ(t,0xD8);}í spread(ɍ,ĭ(&į)ǥ){į Г Њ(ɦ(x));į ƹ=Њ(_mm256_extracti128_si256(x,1));}Ȅ ϗ(ɍ){Ú ń(ƪ(x,x),kP);}í ʈ(ĭ(&r)ɚ){ĭ t ɚ,u ɚ;
#pragma GCC unroll 4
Ć(ö 8 ƽ 2)ͻ,t Υ ǿ(r Ĕ,r Υ);
#pragma GCC unroll 2
Ć(ö 8 ƽ 4)
#pragma GCC unroll 2
Ć(Ϡ)u[i ʳ j]=ɻ(t[i+j],t[i+j+2]),u[i ʳ j+1]Ǿ(t[i+j],t[i+j+2]);
#pragma GCC unroll 4
Т ѹ,з}Ȅ Ŏ(ɍ,ĭ y){Ú Þ(x,y);}Ȅ Ƒ(ɍ){Ú ð(x,x);}Ȅ plus(ɍ,ĭ y){Ú ð(x,y);}Ȅ ʁ(ɍ,ĭ y){Ú _mm256_sub_epi64(x,y);}Ȅ Ų(){Ú _mm256_set1_epi64x(ú ǈ>(4*ŉ(kP)*kP));}Ą<Õ N>ˮ ȕ(Ģ(&x)[N],ĭ(&į)[N]){
#pragma GCC unroll 8
Ć(Ů N Į)į Ƈ ñ(x Ĕ,32);}Ȅ reduce_sums(н,ĭ Ȏ){ȱ,p=ɟ;ĥ Ú ǝ;}ˮ graeffe_sums(Ģ(&c)ɚ,Ģ(&tc)ɚ,ĭ(&ʂ)ɛ){ʦ x=[&c](ý i,ý j){Ú Ŏ(c Ĕ,c Ɓ);}Ǎ t=[&c,&tc](ý i,ý j){Ú Ŏ(c Ĕ,tc Ɓ);};ʂ Г ʁ(ʁ(plus(plus(x(0,0),Ƒ(t(2,6))),plus(t(4,4),Ų())),Ƒ(t(1,7))),Ƒ(t(3,5)));ʂ ƹ=ʁ(ʁ(plus(plus(Ƒ(x(0,2)),Ƒ(t(4,6))),Ų()),x(1,1)),plus(Ƒ(t(3,7)),t(5,5)));ʂ ǥ=ʁ(ʁ(plus(plus(Ƒ(x(0,4)),x(2,2)),plus(t(6,6),Ų())),Ƒ(x(1,3))),Ƒ(t(5,7)));ʂ Ѐ=ʁ(ʁ(plus(plus(Ƒ(x(0,6)),Ƒ(x(2,4))),Ų()),Ƒ(x(1,5))),plus(x(3,3),t(7,7)));}ˮ ӣ(Ģ(&c)ɚ,Ģ(&tc)ɚ,ĭ(&į)ɛ){н ɛ,Ȏ ɛ,c2 ɚ,tc2 ɚ;graeffe_sums(c,tc,Ŧ);ȕ(c,c2),ȕ(tc,tc2);graeffe_sums(c2,tc2,Ȏ);
#pragma GCC unroll 4
ʢ į ђ ʗ(β);}ˮ ΐ(Ɠ ʝ,ĭ(&c)ɚ){
#pragma GCC unroll 8
ѽ c Ƈ Ō(ʝ ү*i);ʈ(c);}ή kNegative>ˮ scaled_leaves(Ģ(&c)ɚ,Ù я&s,ý ù,ĭ(&tc)ɚ){
#pragma GCC unroll 8
Ć(ȓ 8 ų){Ҽ ń(ŭ(c Ɓ,s),kP);tc ˉ j<ù?đ:kNegative?Ӥ(x):x;}}í Ҩ(Ģ(&x)ɛ,Ģ(&y)ɛ,Ù я&ć,ȧ){ĭ r ɚ;
#pragma GCC unroll 4
Т r Ƈ ʗ(ƪ(x Ĕ,y Ĕ)),r[i+4]Ҝ(Ǚ(x Ĕ,y Ĕ),ć);ʈ(r);
#pragma GCC unroll 8
ѽ Š(į Ʀ i,r Ĕ);}ƛ Ȋ{М,*ô;ċ Õ č(Ό){Ú 2 ӎ(á ϛ(Ī(m)+2));}Ȋ(ĝ i6,Ό,Ù Č*m9=Ƿ){if(m9&&m9->Δ()>=Ī(m)+2){ķ=m9->ķ(),ô=m9->ô()а}ł*Ù ı=i6 à Ѵ*Ù ć=ı Я(á ϛ(Ī(m)+2));ķ=ı,ô=ć;ż(i6 ü,i6 Ƃ,0u);á build_table(ı,m/4,á kRoots Ŭ);á build_table(ć,m/4,á kRoots ƹ);}};ƛ PairWeights{я s,ć;PairWeights(ΰ,Ȕ):s(entries(Ķ.ķ,k)),ć(entries(ћ,k)){}};Ӂ ҏ>ˮ product_sums(Ģ(&a)ɚ,Ģ(&c)ɚ,Ģ(&tc)ɚ,ĭ(&ʂ)ɛ){
#pragma GCC unroll 4
ʢ{ĭ s=Ų();
#pragma GCC unroll 8
ѽ{ȟ j=Ӵ+ҏ-i,l=j+8 ĺ(j>=0)s=j%2?ʁ(s,Ŏ(ɨ,c Ɓ)):plus(s,Ŏ(ɨ,c Ɓ))ĺ(l<8)s=l%2?ʁ(s,Ŏ(ɨ,tc[l])):plus(s,Ŏ(ɨ,tc[l]));}ʂ ђ s;}}Ӂ ҏ>ˮ Ӣ(Ģ(&a)ɚ,Ģ(&c)ɚ,Ģ(&tc)ɚ,ĭ(&į)ɛ){н ɛ,Ȏ ɛ,a2 ɚ,c2 ɚ,tc2 ɚ;product_sums<ҏ>(a,c,tc,Ŧ);ȕ(a,a2),ȕ(c,c2),ȕ(tc,tc2);product_sums<ҏ>(a2,c2,tc2,Ȏ);
#pragma GCC unroll 4
ʢ į ђ ʗ(β);}ή kNumerator>ƛ FirstLevelProducts{ċ æ ý kOutputs=kNumerator?5:3 ĩ Ȋ*Ķ;Ɠ q1,*q2,*p1;ȧ[kOutputs];Ѱ ǜ(Ȕ)Ù{ы a[kOutputs]ɛ,b[kOutputs]ɛ;Ȟ ə(16*k,ş.s,a);Ȟ ԏ(16*k+8,ş.s,b);
#pragma GCC unroll 5
Ć(ö kOutputs Į)Ҩ(ɨ,Ϲ,ş.ć,į Ĕ Ʀ k);}ʙ Ĉ Ȟ(Õ at,Ù я&s,ĭ(&r)[kOutputs]ɛ)Ù{ĭ c1 ɚ,c2 ɚ,a ɚ,t1 ɚ,t2 ɚ;ΐ(q1+at,c1),ΐ(q2+at,c2)Ѓ(kNumerator)ΐ(p1+at,a)Φ(c1,s,kNumerator?2:4,t1)Φ(c2,s,1,t2);ӣ(c1,t1,r Ŭ);ӣ(c2,t2,r ƹ);Ӣ<0>(c1,c2,t2,r ǥ)Ѓ(kNumerator){Ӣ<1>(a,c1,t1,r Ѐ);Ӣ<1>(a,c2,t2,r ɛ);}}};ԅ Ж(ý lg){Ú ʀ(ȋ Ŭ[lg],Ö((ŉ(1)<<31)Ұ));}ƛ LevelBottom{Ǒ Ɔ,Ƚ=Ś ĩ Ȋ*Ķ;Ņ f4;ș{Ǆ{Ķ->ķ}(a,þ,ù)ĺ(f4)Ć(Ů þ ƽ 4)ǜ(a+32*i,2*(Ў i));}Ѱ ǜ(Ɠ q,Ȕ)Ù{ы va ɛ,vb ɛ;Ȟ ə(q,ş.s,va);Ȟ ԏ(q+8,ş.s,vb);Ҩ(va,vb,ş.ć,f4 Ʀ k);}ʙ Ŵ Ȟ(Ɠ q,Ù я&s,ĭ(&v)ɛ){ĭ c ɚ,tc ɚ;ΐ(q,c)Φ(c,s,4,tc);ӣ(c,tc,v);}};ƛ ͱ{Ǒ Ś,Ƚ=Ɔ ĩ Ȋ*Ķ;Ɠ f,*q;Ӂ ҏ>ĉ Ŵ Ͽ(Ģ(&p)ɛ,Ģ(&wp)ɛ,Ģ(&c)ɚ,ĭ(&ʂ)ɛ){
#pragma GCC unroll 4
ʢ{Ģ ù=Ŏ(p Ŭ,c[Ӵ+ҏ]);ĭ s=ҏ?ʁ(Ų(),ù):ù;
#pragma GCC unroll 3
Ć(λ 4 Į){Ģ term=Ŏ(i<=k?p Ĕ:wp Ĕ,c[2*((k-i+4)%4)+ҏ]);s=ҏ?ʁ(s,term):plus(s,term);}ʂ ђ s;}}ĉ Ŵ leaf(Ģ(&p)ɛ,Ģ(&wp)ɛ,Ģ(&c)ɚ,ĭ(&į)ɚ){ĭ p2 ɛ,wp2 ɛ,c2 ɚ,Ŧ ɛ,Ȏ ɛ;ȕ(p,p2),ȕ(wp,wp2),ȕ(c,c2);Ͽ<0>(p,wp,c,Ŧ);Ͽ<0>(p2,wp2,c2,Ȏ);
#pragma GCC unroll 4
ʢ į[Ӵ]=β;Ͽ<1>(p,wp,c,Ŧ);Ͽ<1>(p2,wp2,c2,Ȏ);
#pragma GCC unroll 4
ʢ į[Ӵ+1]=β;}Ѱ ƕ(Õ p,ĭ(&r)[16])Ù{ɍ ɚ,c ɚ,pa ɛ,pb ɛ,spa ɛ,spb ɛ,ra ɚ,rb ɚ;
#pragma GCC unroll 8
ѽ x Ƈ Ō(f Ʀ(p+i));ʈ(x)ĩ я s=entries(Ķ->ķ,p);
#pragma GCC unroll 4
Т{Ģ t Ҝ(x[i+4],s);pa Ƈ ů(ƪ(x Ĕ,t)),pb Ƈ ů(Ǚ(x Ĕ,t));spa Ƈ ń(ŭ(pa Ĕ,s),kP),spb Ƈ Ӥ(ń(ŭ(pb Ĕ,s),kP));}ΐ(q ү*p,c);leaf(pa,spa,c,ra);ΐ(q ү*p+8,c);leaf(pb,spb,c,rb);ʈ(ra),ʈ(rb);
#pragma GCC unroll 8
ѽ r ɔ=ra Ĕ,r ư=rb Ĕ;}ĉ Ŵ finish(ΰ,Ģ(&r)[16],Õ g,ȧ){
#pragma GCC unroll 4
Т{ϥ={r[4*i],r[4*i+1],r[4*i+2],r[4*i+3]};υ(f,Τ(ћ,g+i));
#pragma GCC unroll 4
Ӏ Š(ѓ 32*i ϝ,f ѯ);}}Ǣ{Ć(ǉ þ;j+=4,į+=128){ĭ r[16];ƕ(2*(Ў j),r);finish(*Ķ,r,Ў j,į);}}};ƛ CompositionSumBottom{Ǒ Ś,Ƚ=Ɔ;ͱ Ơ ǥ;Ǣ{Ć(ǉ þ;j+=4,į+=128){ĭ r[16],r2[16];Ơ Ŭ.ƕ(2*(Ў j),r);Ơ ƹ.ƕ(2*(Ў j),r2);
#pragma GCC unroll 16
Ć(ö 16 Į)r Ƈ ʗ(ƪ(r Ĕ,r2 Ĕ));ͱ::finish(*Ơ Ŭ.Ķ,r,Ў j,į);}}};Ϥ forward_with(ҽ,Õ ä,ΰ,Ϩ){Ù Ǉ Ŝ(Ķ.ķ,ћ,b9)ĩ Ɏ in(ʰ,ä,0)Ϊ{ƾ 4;forward_top4(in,v,h,Ķ.ķ)Ȳ ő(a ʥ t*h,h,t);}ŝ{ƾ 2;forward_top2(in,v,h)ѳ;}}Ϥ Ɇ(ҽ,ΰ,Ϩ,Ö ŧ,Π){Ù Ǉ Ŝ(Ķ.ķ,ћ,b9)Ϊ{ƾ 4 Ȳ ő(a ʥ t*h,h,t);inverse_top4(v,h,ǂ,ћ,ƭ(ŧ));}ŝ{ƾ 2 ѳ;Ӧ(v,h,ǂ,ŧ);}}í ԋ(Ң,Ϫ,Õ b,М,Ȕ){ҳ Ù v=Ț(a)ĺ(b ζ)Ú Ƥ forward_even_columns(v,h,ķ+φ(k),ķ+φ(Ӵ));Ƥ ԋ(v,h,ǻ b,ķ+φ(k),ķ+φ(Ӵ));}í ǔ(Ң,Ϫ,Õ b,Ɠ ô,Ȕ){ҳ Ù v=Ț(a)ĺ(b ζ)Ú Ƥ inverse_even_columns(v,h,ô+φ(k),ô+φ(Ӵ));Ƥ ǔ(v,h,ǻ b,ô+φ(k),ô+φ(Ӵ));}í forward_half(Ң,Ϫ,ƍ high,Ù Τ&w){Ć(Ņ x=a;x<ύ h;x+=8){Ģ u ӑ(Ō(x))ĺ(high){Ģ b ӷ(x Ʀ h),yb Ҝ(b,w.y),zb Ҝ(b,w.z);Š(x,ƪ(u,yb)),Š(x Ʀ h,Ǚ(u,yb));Š(x ү*h,ƪ(u,zb)),Š(x+24*h,Ǚ(u,zb));}ŝ{Ģ xc Ҝ(Ō(x ү*h),w.x),ʂ ӑ(ƪ(u,xc)),ɸ=low_difference(u,xc);Š(x,ʂ),Š(x Ʀ h,ʂ),Š(x ү*h,ɸ),Š(x+24*h,ɸ);}}}í inverse_half(Ң,Ϫ,ƍ high,Ù Τ&w){Ć(Ņ x=a;x<ύ h;x+=8){Ģ f0 ӷ(x),f1 ӷ(x Ʀ h),f2 ӷ(x ү*h),f3 ӷ(x+24*h)κ ab ӑ(ƪ(f0,f1)),cd ӑ(ƪ(f2,f3));Š(x,ʗ(ϓ))ĺ(high)Š(x Ʀ h,Ӓ)і Š(x ү*h,ŭ(Ќ,w.x));}}Ő Ƭ>Ũ Pruned{Ⱥ Pruned(ΰ,Ϩ,Õ b):ӡ(Ķ.ķ,
ћ,b9),ʻ(Ķ),b_(b){}Ĉ ı(Ң,Ԇ,Ȕ)Ù{ƾ 4,ʗ=Õ(Ī(h))ĺ(nv<=kForwardFull||ʗ+1<b_)Ú ӡ.Ϙ(a,nv,k)ĺ(ʗ>b_){ԋ(a,h,b_,ʻ.ķ,k)Ȳ ı(ύ t*h,h,4*k+t)а}forward_half(a,h,ʗ+1==b_,Τ(ʻ.ķ,k))Ȳ ӡ.δ}Ĉ ć(Ң,Ԇ,Ȕ)Ù{ƾ 4,ʗ=Õ(Ī(h))ĺ(nv<=kInverseFull||ʗ+1<b_)Ú ӡ.Ϙ(a,nv,k)ĺ(ʗ>b_){Κ ć(ύ t*h,h,4*k+t)Ū ǔ(a,h,b_,ʻ.ô,k);}Κ ӡ.δ inverse_half(a,h,ʗ+1==b_,Τ(ʻ.ô,k));}Ⱦ ċ ɿ kForwardFull=64,kInverseFull=16;Ǉ<Ƭ>ӡ ĩ Ȋ&ʻ;Õ b_;};Ϥ forward_pruned(ҽ,Õ ä,Õ b,ΰ,Ϩ){Ù Pruned<Ƭ>ҧ(Ķ,b9,b)ĩ Ɏ in(ʰ,ä,0)ȡ nv=Ư/8,ҕ=ǻ b Ѵ*v=Ț(ʰ)κ p=ɟ,zero=đ ǡ{ƾ 4 ƣ z(Ķ.ķ ƹ,Ķ.ķ[9])Ǖ{ǳ{Ģ x0=in(j),x1=in(j+h),x2=in(j ʳ h),x3=in(j+3*h)κ ac=ƪ(x0,x2),amc=é(ƪ(x0,p),x2)κ bd=ƪ(x1,x3),zbmd Ҝ(é(ƪ(x1,p),x3),z);v ˉ ƪ(ac,bd),Ӌ=Ǚ(ac,bd);v ɋ=ƪ(amc,zbmd),v Ɋ=Ǚ(amc,zbmd);}ȳ v ɋ=v Ɋ=zero;}Κ ҧ.ı(a ʥ t*h,h,t);}ŝ{ƾ 2 Ǖ{ǳ{Ģ x0=in(j),x1=in(j+h);v ˉ ƪ(x0,x1),Ӌ=é(ƪ(x0,p),x1);}ȳ zero;}ҧ.ı(ʰ,h,0);ҧ.ı(a ʥ h,h,1);}}Ϥ inverse_pruned(ҽ,Õ b,ΰ,Ϩ,Ö ŧ,Π=Ȉ){Ù Pruned<Ƭ>ҧ(Ķ,b9,b)ȡ nv=Ư/8,ҕ=ǻ b Ѵ*v=Ț(ʰ)ƣ s(ŧ)Ǎ е Є ĩ ƍ lower=ǂ!=Ƨ,ʓ=ǂ!=ƨ ǡ{ƾ 4 Ȳ ҧ.ć(a ʥ t*h,h,t)ƣ z(ћ ƹ,ћ[9])Ǖ ǳ{Ģ x0=v Ɓ,x1=Ӌ,x2=v ɋ,x3=v Ɋ κ ab ӑ(ƪ
(x0,x1)),cd ӑ(ƪ(x2,x3))κ amb ӑ(Ǚ(x0,x1)),cmd Ҝ(Ǚ(x2,x3),z)ĺ(lower)v ˉ е(ϓ),Ӌ=е(ϒ)ĺ(ʓ)v ɋ=е(Ќ),v Ɋ=е(ϑ);}}ŝ{ƾ 2;ҧ.ć(ʰ,h,0);ҧ.ć(a ʥ h,h,1)Ǖ ǳ{Ģ x0=v Ɓ,x1=Ӌ ĺ(lower)v ˉ е(ƪ(x0,x1))ĺ(ʓ)Ӌ=е(Ǚ(x0,x1));}}}Ũ Ԋ{Ϳ Ԋ(Ѕ):rest_(ĸ){}ċ ɿ č(ǀ){Ú ß(n)+8;}ĝ ś(ǀ){ģ skip=(0-Λ(rest_ à)/4)%16 ĺ(skip Я(n)>rest_ Û)ǩ ǫ s=rest_ Ė(skip,n);rest_=rest_ Ė(skip Я(n))Ū s;}Ⱦ ĝ rest_;};ƛ Levels{Ό;ý lg;Ȋ Ķ;ĝ o5;ĝ Ȍ(ý s)Ù{Ú o5 Ė(Õ(s)ӎ(4*m),4*m);}ĝ д(ɵ)Ù{Ú Ȍ(1)Ė(i ӎ(m),m);}ĝ column_transform(Ȕ)Ù{Ú Ȍ(lg-3)Ė((k-1)ӎ(m/4),m/4);}ċ Õ ĸ(Ό){Ú ß(Ȋ Ʈ(m))+Ԋ Ʈ(Õ(Ī(m))ӎ(4*m));}Levels(Õ ä,Ѕ,Ù Č*m9=Ƿ):m(ä),lg(Ī(ä)),Ķ(ĸ ȑ(Ȋ Ʈ(ä)),ä,m9){Ԋ carve(Ԃ(ß(Ȋ Ʈ(ä))));o5=carve ņ(Õ(lg)ӎ(4*m));}};í next_level(ҽ,Õ ď,Õ χ){ģ Ɵ=ď/2,top=χ Ԏ;ȩ(a ü,a ʊ(Ɵ),a ʊ(top));a[top]=a[top]?a[top]-1:Ͻ 1;a Г 1;ż(a ʷ 1,a ʊ(Ɵ),0);}í first_level(ƶ,Ğ g,Levels&ţ){ģ m=ţ.m,h=m/2,k9=m/4 ĩ Ȋ&Ķ=ţ.Ķ ĩ Ğ v=Ȁ(3)ȑ(h)ǫ t1=ţ.д(0),t2=ţ.д(1)ȡ Ŧ=ʘ(Ħ(h,(ȸ+1)/2),1);ʍ ƒ(;j+8<=Ŧ&&2*j ү<=ȸ Ӗ){Ҽ even_lanes(ɂ(õ Ģ*>(g Ʒ 2*j)),ɂ(õ Ģ*>(g Ʒ 2*j+8)));Š(t1 Ʒ j,Ӥ(ϗ(x)));}Ć(;j<Ŧ ų)t1 ˉ Ґ(g[2*j]);t1 Ŭ Ē ż
(t1 ʊ(Ŧ),t1 ʊ(h),0)ǫ q=Ȁ(2)Ӛ Š(q Ʒ h+i,ϗ(even_lanes(Ō(t1 Ʒ 2*i),Ō(t1 Ʒ 2*i+8))))Ȇ(t1 ȑ(h),0,t1)Ȇ(v,0,t2);ȧ Ѐ ø(Ů 3 Į)į Ƈ Ȁ(3)Ʒ ß(m)+i ӎ(h)ĩ FirstLevelProducts ə ƕ{&Ķ,t1 à,t2 à,Ƿ,{į Ŭ,į ƹ,į ǥ}}ø(ѿ m/16;k+=8)ƕ.ǜ(k)ø(Ņ x:į)Ɇ(ĝ(x,h),Ķ,Ɉ{ћ,x},Ж(ţ.lg-1),ƨ);q Г 1;ż(q ʷ 1,q ʊ(k9),0)Ӛ{Ģ v2=ϗ(even_lanes(Ō(v Ʒ 2*i),Ō(v Ʒ 2*i+8)));Š(q Ʒ 2*h+i,ń(ƪ(v2,Ō(į Ŭ+i)),kP));Š(q Ʒ 3*h+i,ϗ(Ō(į ǥ+i)));}ĵ(į ƹ,k9,q ʊ(4*h));}í first_level_transposed(ƶ,Ù Levels&ţ,Ğ in,ɐ){ģ m=ţ.m,h=m/2,k9=m/4 ĩ Ȋ&Ķ=ţ.Ķ Ǎ ξ=Ѯ(Ȕ){Ú in Ė(2*m+k*h,k9);};ĝ tp Ѐ ø(ѿ 3 ȝ){tp ђ į Ė(k ӎ(h),h)Ȇ(ξ(k),0,tp ƫ);}Ɠ q1=ţ.д(0)à,*q2=ţ.д(1)à Ǎ term=Ѯ(Ȕ,Ɠ q){Ú ͱ{&Ķ,tp ƫ à,q};}ø(Õ r=2;r<4 ė r){Ù ĝ ă=į Ė(r*m,m);Ɇ(ă,Ķ,CompositionSumBottom{{term(r-1,q1),term(r-2,q2)}},ȋ ƹ[ţ.lg],ƨ)ĩ Ğ a5=ξ(r)Ӛ{ɍ ǥ;spread(Ō(a5 Ȝ),x);Š(ă Ʒ 2*i,ń(ƪ(Ō(ă Ʒ 2*i),x Ŭ),kP));Š(ă Ʒ 2*i+8,ń(ƪ(Ō(ă Ʒ 2*i+8),x ƹ),kP));}}}Ą<Õ K>Ĉ Ŗ(ΰ,Ù ɝ*ǜ,ɐ,Π){ϙ<K>b9{}ø(ѿ K ȝ)b9.Ơ ђ{{Ķ.ķ,ћ,ǜ ƫ.b à},ǜ ƫ.ʰ};Ɇ(į,Ķ,b9,ȋ ƹ[Ī(ǲ)],ǂ);}í ʏ(Ğ a,Ö c,ĝ t){Œ f(c)ø(Ů Ư Ɲ)Š(t Ȝ,ń(ŭ(Ō(a Ȝ),f),kP));}í third_last_level(ƶ
,Ù Levels&ţ,Ù ĝ(&c)ɛ){ģ m=ţ.m,k9=m/4,e1=m/8 ĩ Ğ q=Ȁ(ţ.lg-3)ǫ n9=Ȁ(ţ.lg-1);ĝ ļ ɚ,tc ҹ(ʶ 8 ȝ){ļ ђ n9 Ė((k-1)ӎ(e1+1),e1+1);tc ђ ţ.column_transform(k);}ԑ{ɍ ɚ;
#pragma GCC unroll 8
Ԑ
#pragma GCC unroll 7
Ć(ʶ 8 ȝ)Š(ļ ƫ Ʒ r,k%2?Ӥ(x ƫ):x ƫ);}Ć(ʶ 8 ȝ){Ġ x=q[16*e1+k];ļ ƫ[e1]=k%2&&x?Ͻ x:x;}Ć(ʶ 8 ȝ)t.ı(ļ ƫ,0,tc ƫ);ĝ Ƒ Ѐ,n3 ɛ ǫ Ͽ=n9 Ė(7 ӎ(e1+1))ø(Ů 3 Į)Ƒ Ƈ Ͽ Ė(i ӎ(k9),k9)У n3 ђ Ͽ Ė((k+2)ӎ(k9),k9);ʏ(tc[5],2,Ƒ Ŭ),ʏ(tc Ѐ,ӕ,Ƒ ƹ),ʏ(tc ɛ,ӕ,Ƒ ǥ)ĩ ɝ ù ƹ={{tc ƹ,tc ƹ}},ψ ǥ={{tc ǥ,tc ǥ},{tc ƹ,Ƒ ƹ}},г(ù,n3 ƹ);t.Ŗ(ψ,n3 ǥ);t.Ŗ(o0,n3 Ѐ);Ġ o9 ɛ={0,ӕ,2,ӕ}У{Ù Ğ i3=ļ[Ӵ]ƣ w(o9 ƫ)Ӛ{Ҽ Ō(n3 ƫ Ȝ);Š(c ƫ Ȝ,i<e1?ů(ƪ(x,ń(ŭ(Ō(i3 Ȝ),w),kP))):x);}Ù ŉ Ȣ=c ƫ[e1]+ŉ(o9 ƫ)*i3[e1];c ƫ[e1]=Ö(Ȣ Ұ);c ƫ[k9]=c ƫ Ŭ;c ƫ Ŭ Ē ż(c ƫ ʊ(k9+1),c ƫ Ƃ,0);}}í third_last_level_transposed(ƶ,Ù Levels&ţ,Ù Ğ(&p)ɛ,ĝ Ǡ,ɐ){ģ m=ţ.m,k9=m/4,e1=m/8;ĝ tp ɛ,r ҹ(ǉ 4 ų){tp ˉ Ǡ Ė(j ӎ(k9),k9)Ȇ(p Ɓ,0,tp Ɓ);}Ć(Õ c=1;c<8 ė c){r[c]=Ǡ Ė((c+3)ӎ(k9),k9);ɝ ǜ ɛ;Ƞ ƒ(ǉ 4&&2*j<c ų)ǜ[þ++]={tp Ɓ,ţ.column_transform(c-2*j)}ĺ(þ<4)t.Ŗ(Ø span(ǜ,þ),r[c],Ƨ)і Ŗ<4>(ţ.Ķ,ǜ,r[c],Ƨ);}Ć(ɵ=e1 Ͼ k9 Ɲ){ɍ ɚ;x Г Ō(p Ŭ Ȝ);
#pragma GCC unroll 7
Ć(Õ c=1;c<8 ė c)x[c]=c%2?Ō(r[c]Ȝ):ů(ƪ(Ō(r[c]Ȝ),Ō(p[c/2]Ȝ)));ʈ(x);
#pragma GCC unroll 8
Ć(ǉ 8 ų)Š(ԃ 2*m ү*(i-e1+j),x Ɓ);}}í Ӡ(ƶ,Ğ g,Levels&ţ){ģ m=ţ.m ǫ v=Ȁ(3)ʴ,q0=Ȁ(0)ϡ;ż(q0 ü,q0 Ƃ,0);ȩ(g ʊ(Ħ<Õ>(ȸ,1)),g ʊ(Ħ(ȸ,m)),q0 ʷ 1)ĩ Ȋ&Ķ=ţ.Ķ;forward_with(q0,m,Ķ,LevelBottom{&Ķ,v à});Ɇ(v,Ķ,Ɉ{ћ,v à},Ж(ţ.lg),Ȉ);first_level(t,g,ţ)ø(ý s=2;s+3<ţ.lg ė s){ģ ď=2*(m>>s),pad=Õ(ţ.lg-s-3)ǫ next=Ȁ(s+1)ϡ;forward_pruned(Ȁ(s),((ǻ s)+1)Ԏ,pad,Ķ,LevelBottom{&Ķ,next à});inverse_pruned(next,pad-1,Ķ,Ɉ{ћ,next à},Ж(ţ.lg+1));next_level(Ȁ(s+1),ď/2,Õ(2)<<s);}ĝ c ɛ У c ђ Ȁ(ţ.lg-2)Ė((k-1)ӎ(m/2),m/2);third_last_level(t,ţ,c)ǫ q=Ȁ(ţ.lg-1)ʴ;ż(q ü,q Ƃ,0)ø(Ů m/4 Ɲ)Š(q Ȝ,Ӥ(ϗ(Ō(c ǥ Ȝ))));q[m/4]=Ґ(c ǥ[m/4])ǫ n3=Ȁ(ţ.lg-1)Ė(ß(m),m/2)У t.ı(c ƫ);Ɖ(c ƹ,c ƹ,n3)ø(Ů m/2 Ɲ)Š(q Ȝ,ń(ƪ(Ō(q Ȝ),Ō(n3 Ȝ)),kP));Ġ top=q[m/2]+q Ŭ;q[m/2]=top>=kP?top-kP:top;q Ŭ Ē t.ı(q);}}ò ý compose_log(ǀ){if(n<=â Ʉ)Ú ʵ Ū Ī(â compose_length(n))+1;}Ί compose_scratch(ǀ){if(n<=â Ʉ)Ú 0 Ū â Levels::ĸ(â compose_length(n));}ԅ outer_word(Ö x){Ú x;}ԅ outer_word(ǣ&x)Щ Ő Ф>Ĉ compose_imported(ƶ,Ԅ Ф>f,Ğ g,ĝ h,Ѕ,Ù Č*m9=Ƿ){Ǝ(Ĥ Ф,Ö>||Ĥ Ф,Ě>)ȡ n=h Û ĺ(f ű||ȸ<=1){ż(h ü,h Ƃ,0u)
ĺ(n&&!f ű)h Г outer_word(f Ŭ)а}if(n<=â Ʉ){Ĺ(Ĥ Ф,Ö>)Ú â compose_direct(f,g,h);Ö a[â Ʉ]ȡ ä Ш(Ǳ,n)ø(Ů ä Į)ӫ outer_word(f Ĕ)Ū â compose_direct(Ğ(a,ä),g,h);}â Levels ţ(â compose_length(n),ĸ,m9);â Ӡ(t,g,ţ)ȡ m=ţ.m ȉ lg=ţ.lg ǫ Ȣ=Ȁ(lg-1)ǫ p=Ȣ Ė(ß(m),m),pq=Ȣ Ė(2 ӎ(m),m);{ģ ä Ш(Ǳ,m);ż(p ü,p Ƃ-ř(ä),0);Ǜ to=p Ʒ(m-ä)ĩ â ĭ ˁ=ʞ;ʍ ƒ(;j+8<=ä Ӗ){Ǝ(ƚ(Ф)==ƚ(Ö));â ɍ;Ǵ(&x,f Ʒ ä-8-j,ƚ(x))Ѓ(Ĥ Ф,Ě>)x=â ń(x,â kP);Ψ(õ â ҳ>(to+j),Ľ(x,ˁ));}Ć(;j<ä ų)to ˉ outer_word(f[ä-1-j]);}t.Ŷ(p,0,pq,Ȣ ʴ,Ƨ);{ģ h=m/2,k9=m/4 ǫ p0=p Ė(h),p1=pq Ė(h),t0=Ȣ ȑ(h),t1=Ȣ Ė(ß(h),h)ǫ c=Ȁ(lg-2)Ǎ column=Ѯ(Ȕ){Ú c Ė((k-1)ӎ(h),h);}ǫ r1=p ȑ(h),r2=pq ȑ(h),r3=Ȣ Ė(3 ӎ(m),h)Ȇ(p0,0,t0)Ȇ(p1,0,t1);Ɖ(t0,column(1),r1,Ƨ);Ɖ(t0,column(2),r2,Ƨ)ĩ ɝ ǜ ǥ={{t1,column(1)},{t0,column(3)}};t.Ŗ(ǜ,r3,Ƨ)ø(ɵ=k9 Ͼ h Ɲ)â Š(r2 Ȝ,â ń(â ƪ(â Ō(p1 Ȝ),â Ō(r2 Ȝ)),ϵ))ĩ Ğ İ ɛ={p0 Ė(k9),r1 Ė(k9),r2 Ė(k9),r3 Ė(k9)};â third_last_level_transposed(t,ţ,İ,Ȁ(lg-2),Ȁ(lg-3));}ĝ in=Ȁ(lg-3);Ġ ŧ=â ȋ ƹ[lg+2]ø(ý s=lg-4;s>=2;--s){Ù ĝ x=in Ė(2*m),į=Ȁ(s==lg-4?lg-2:s+1)ȡ pad=Õ(lg-s-3);â forward_pruned
(x,2*m,pad-1,ţ.Ķ,â Ǆ{ţ.Ķ.ķ});â inverse_pruned(į,pad,ţ.Ķ,â ͱ{&ţ.Ķ,x à,Ȁ(s)à},ŧ,Ƨ);in=į;}â first_level_transposed(t,ţ,in,Ȁ(2));in=Ȁ(2)ǫ į=Ȁ(1)ǫ p0=in Ė(2*m,m),r=į ϡ Ȇ(p0 ȑ(m/2),0,p0);â Ɇ(r,ţ.Ķ,â ͱ{&ţ.Ķ,p0 à,Ȁ(0)à},â ȋ ƹ[ţ.lg+1],ƨ);Ȕ ƒ(;k ү<=n;k+=16){â ĭ a ǥ;â spread(â Ō(in Ʒ 3*m+k/2),a)ø(Ӆ<2 ė t)Ψ(õ â ҳ>(h Ʒ k ϝ),â ɸ(a ѯ,â Ō(r Ʒ k ϝ)));}Ć(;k<n ȝ){Ġ a=k%2?0:in[3*m+k/2];h ђ a>=r ƫ?a-r ƫ:a+ϵ-r ƫ;}}í compose(ƶ,О,Ğ g,ĝ h,Ѕ){compose_imported(t,f,g,h,ĸ);}}ā Ȗ{Ϧ{Ί Ҏ(ǀ){Ú ɬ(ʘ(n,512));}í projection_direct(Ğ g,ҽ){ģ n=Ư;Ö Ǹ[Ʉ]={1},next[Ʉ]ø(ѿ n ȝ){a ђ Ǹ[ԕ]ø(Ů n Į){ŉ c ƒ(ʍ=1;j<=i&&j<ȸ ų)c+=ŉ(g Ɓ)*Ǹ[i-j]Ұ;next Ƈ Ö(c Ұ);}ĵ(next,n,Ǹ);}}Ȅ ȕ(ɍ,ĭ y){Ģ t=_mm256_castps_si256(_mm256_shuffle_ps(р(x),р(y),0xDD))Ū ҭ(t,0xD8);}ή kAdd,Ũ Op>Ĉ ʺ(Ğ b,Ї,п,ř Ų,ɐ,Ù Op&op){ʦ scalar=[&op](Ö x){Ú Ö(Ҙ(ɦ(op(å(ý(x))))));}Ǎ put=[&į](ʍ,Ö x){Ĺ(kAdd)į ˉ į Ɓ+x>=kP?į Ɓ+x-kP:į Ɓ+x і į ˉ x;}Ǎ at=[Ų](ʍ){Ú ř(2*j)+Ų;}Ǎ ù=Ѯ(ɵ){Ù ř d=ř(i)-Ų Ū Ħ(ǲ,d<=0?Õ(0):Õ(d+1)/2);}ȡ begin=ù(lo),end=Ŋ(begin,ù(hi))Ѓ(!kAdd){ż(į ü,į ʊ(begin),0);ż(į ʊ(end),į Ƃ
,0);}ʍ=begin ø(;j+8<=end&&Õ(at(j))ү<=η Ӗ){Ɠ ʝ=b Ʒ at(j);ɍ=op(even_lanes(Ž(ʝ),Ž(ʝ+8)))Ѓ(kAdd)x=ń(ƪ(Ž(ԃ j),x),kP);ū(ԃ j,x);}Ć(;j<end ų)put(j,scalar(b[Õ(at(j))]));}í add_into(Ğ x,Ğ y,ɐ){Ć(Ů ǲ Ɲ)Š(į Ȝ,ń(ƪ(Ō(x Ȝ),Ō(y Ȝ)),kP));}ƛ ProjectionBottom{Ǒ Ɔ,Ƚ=Ś ĩ Ȋ*Ķ;Ɠ p;Ņ v,*w;ș{Ǆ{Ķ->ķ}(a,þ,ù)ø(Ů þ ƽ 4)ǜ(a+32*i,2*(Ў i));}Ѱ ǜ(Ɠ q,Ȕ)Ù{ы va ɛ,vb ɛ,wa ɛ,wb ɛ;Ȟ ə(q,p ү*k,ş.s,va,wa);Ȟ ԏ(q+8,p ү*k+8,ş.s,vb,wb);Ҩ(va,vb,ş.ć,v Ʀ k);Ҩ(wa,wb,ş.ć,w Ʀ k);}ʙ Ŵ Ȟ(Ɠ q,Ɠ p,Ù я&s,ĭ(&v)ɛ,ĭ(&w)ɛ){ĭ c ɚ,a ɚ,tc ɚ;ΐ(q,c),ΐ(p,a)Φ(c,s,2,tc);ӣ(c,tc,v);Ӣ<1>(a,c,tc,w);}};í inverse_lower(һ,Ϫ,Ù Τ&w,ȧ){Ć(Ů 8*h Ɲ){Ģ f0 ӷ(a+i),f1 ӷ(a+i Ʀ h),f2 ӷ(a+i ү*h),f3 ӷ(a+i+24*h);Š(ѓ i,ʗ(ƪ(ʗ(ƪ(f0,f1)),ʗ(ƪ(f2,f3)))));Š(ѓ i Ʀ h,Ӓ);}}í forward_lower(Ɠ in,Ϫ,Ù Τ&w,Ң){Ć(Ů 8*h Ɲ){Ģ u ӑ(Ō(in+i)),b ӷ(in+i Ʀ h),yb Ҝ(b,w.y),zb Ҝ(b,w.z);Š(a+i,ƪ(u,yb)),Š(a+i Ʀ h,Ǚ(u,yb));Š(a+i ү*h,ƪ(u,zb)),Š(a+i+24*h,Ǚ(u,zb));}}Ő Ƭ>Ũ Doubling{Ⱥ Doubling(ҽ,Õ ŀ,ΰ,Ϩ):a_(ʰ),half_(Ư/2),ʪ(ŀ),b_(Õ(Ī(ŀ))-1),aligned_(Ī(ŀ)%2 ζ),ʻ(Ķ),bottom_(b9),ϰ(Ķ.ķ,ћ,Ɉ{ћ,ʰ}),forward_(Ķ.ķ,ћ,b9){}
Ĉ run(Ö p8,Ö fix)Ù{Ǜ copy=a_+half_ ȡ nv=half_/8,ѩ=ʘ(ʪ,16)ø(Ȫ nv;c+=ѩ){Ć(ʍ=c/ʪ;j<(c+ѩ)/ʪ ų){inverse_x(a_ Ʀ ʪ*j,j,copy Ʀ ʪ*j);forward_x(copy Ʀ ʪ*j,j,a_ Ʀ ʪ*j);}if(ʪ<16)bottom_(a_ Ʀ c,ѩ/4,c/4);}inverse_y(copy,p8)ĺ(fix){Ć(Ů 4*ʪ Ɲ)Š(copy+i,Ӥ(Ō(copy+i)));copy Г fix>=Ͻ copy Ŭ?fix-(Ͻ copy Ŭ):fix+copy Ŭ;}ɇ();}Ⱦ Ĉ inverse_x(Ң,ʍ,ȧ)Ù{if(ʪ==4){ϰ.Ϙ(a,4,j)Ԗ(a,16,į);}м(aligned_){ģ h=ʪ/4 Ȳ ϰ.Ϙ(ύ t*h,h,4*j+t);inverse_lower(a,h,Τ(ʻ.ô,j),į);}ŝ{ģ h=ʪ/2;ϰ.Ϙ(a,h,2*j);ϰ.Ϙ(ύ h,h,2*j+1)ø(Ů 8*h Ɲ)Š(ѓ i,ʗ(ƪ(Ō(a+i),Ō(a+i Ʀ h))));}}Ĉ forward_x(Ɠ in,ʍ,Ң)Ù{if(ʪ==4){ĵ(in,16,a);ʇ(a ү,16,0);}м(aligned_){ģ h=ʪ/4;forward_lower(in,h,Τ(ʻ.ķ,j),a)ĺ(h==4)Ú bottom_(a,4,4*j)Ȳ forward_.Ϙ(ύ t*h,h,4*j+t);}ŝ{ģ h=ʪ/2 ø(Ů 8*h Ɲ){Ҽ Ō(in+i);Š(a+i,x),Š(a+i Ʀ h,x);}if(h==4)Ú;forward_.Ϙ(a,h,2*j);forward_.Ϙ(ύ h,h,2*j+1);}}Ĉ above(Ң,Ԇ,Ȕ)Ù{if(nv==ʪ)Ú ĺ(nv==2*ʪ){Œ x=ʮ(ʻ.ô,k)ø(Ů 4*ʪ Ɲ){Ģ u ӷ(a+i),v ӷ(a+i Ʀ ʪ);Š(a+i,ʗ(ƪ(u,v))),Š(a+i Ʀ ʪ,ŭ(Ǚ(u,v),x));}Ú;}ƾ 4 Ȳ above(ύ t*h,h,4*k+t);ǔ(a,h,b_,ʻ.ô,k);}Ĉ inverse_y(Ң,Ö ŧ)Ù{ģ nv=half_/8,ҕ=ǻ b_ ĩ ƍ zero=
b_<=5;ҳ Ù v=Ț(a)ƣ s(ŧ)Ǎ е Є κ z0=đ ǡ{ƾ 4 Ȳ above(ύ t*h,h,t)ƣ z(ʻ.ô ƹ,ʻ.ô[9])Ǖ{ǳ{Ģ x0=v Ɓ,x1=Ӌ,x2=v ɋ,x3=v Ɋ κ ab ӑ(ƪ(x0,x1)),cd ӑ(ƪ(x2,x3))κ amb ӑ(Ǚ(x0,x1)),cmd Ҝ(Ǚ(x2,x3),z);v ˉ е(ϓ),Ӌ=е(ϒ);v ɋ=е(Ќ),v Ɋ=е(ϑ);}if(zero)ȳ v ɋ=v Ɋ=z0;}}ŝ{ƾ 2;above(a,h,0);above(ύ h,h,1)Ǖ{ǳ{Ģ x0=v Ɓ,x1=Ӌ;v ˉ е(ƪ(x0,x1)),Ӌ=е(Ǚ(x0,x1));}if(zero)ȳ z0;}}}Ĉ ɇ()Ù{ģ nv=half_/4,ҕ=ǻ b_ ĩ Pruned<Ƭ>ҧ(ʻ,bottom_,b_)ǡ{ƾ 4;ҳ Ù v=Ț(a_)ƣ z(ʻ.ķ ƹ,ʻ.ķ[9])Ǖ ǳ{Ҽ v[2*h+j],y Ҝ(v[3*h+j],z);v[2*h+j]=ƪ(x,y),v[3*h+j]=Ǚ(x,y);}ҧ.ı(a_ ү*h,h,2);ҧ.ı(a_+24*h,h,3);}ŝ{ҧ.ı(a_+4*nv,nv/2,1);}}Ņ a_;Õ half_,ʪ,b_;ƍ aligned_ ĩ Ȋ&ʻ;Ƭ bottom_;Ǉ<Ɉ>ϰ;Ǉ<Ƭ>forward_;};í ӆ(ƶ,ΰ,Ğ g,ǀ,ĝ q,ĝ p){ģ m=Ѹ/4,h=m/2,k9=m/4,e=m-n,ä Ш(n,ȸ)Ǎ ȭ=Ѯ(ɍ)Ü{Ú x;}Ǎ j0=Ѯ(ɍ)Ü{Ú Ӥ(x);}Ǎ e0=Ѯ(ɍ)Ü{Ú ϗ(x);}Ǎ Ґ=Ѯ(ɍ)Ü{Ú Ӥ(ϗ(x));}Ǎ ξ=[h,k9](ҽ,Ȕ){Ú a Ė(k*h,k9);}ĺ(ä>0){q Ŭ Ē ȩ(g ʷ 1,g ʊ(ä),q ʷ 1);}forward_with(q ϡ,ä,Ķ,LevelBottom{&Ķ,q à});Ɇ(q ʴ,Ķ,Ɉ{ћ,q à},Ж(Ī(m)),Ȉ)ǫ q2=p Ė(7*h,h),q1=q Ė(7*h,h),p1=q Ė(4*h,h)Ԗ(q ü,h,q2 ü);ʺ ə(g,1,ä,0,q1,Ґ)ĺ(e%2)ʺ ə(g,1,ä,1-ř(e),p1,j0)і ʺ ə(g,1,ä,1-ř(e),p1,ȭ
)ǫ tq2=q ʴ,tq1=q ч,tp1=p ʴ Ȇ(tq2 ȑ(h),0,tq2)Ȇ(q1,0,tq1)Ȇ(p1,0,tp1);Ǜ g1=p Ʒ 4*h;Ǜ g2=p Ʒ 5*h;Ǜ e12=p Ʒ 6*h;Ǜ o1=q Ʒ 5*h;Ǜ o2=q Ʒ 6*h ĩ FirstLevelProducts ԏ ƕ{&Ķ,tq1 à,tq2 à,tp1 à,{g1,g2,e12,o1,o2}}ø(ѿ m/16;k+=8)ƕ.ǜ(k);Ġ ŧ=Ж(Ī(h))ø(ȧ:ƕ.į)Ɇ(ĝ(į,h),Ķ,Ɉ{ћ,į},ŧ,ƨ);ż(p ü,p ʊ(k9),0)ĺ(e%2&&(e-1)/2%2)p[(e-3)/4]=1;ʺ ə(p1,0,h,1,ξ(p,1),ȭ)Ԗ(o1,k9,p ʊ(2*h))Ԗ(o2,k9,p ʊ(3*h))ĺ(e%2){ģ d=(e-1)/2 ĩ ř Ų=1-ř(d)ĩ Ğ b1=q1,b2=q2 ĺ(d%2){ʺ ԏ(b1,0,h,Ų,ξ(p,1),ȭ);ʺ ԏ(b2,0,h,Ų,ξ(p,2),ȭ);}ŝ{ʺ ԏ(b1,0,h,Ų,ξ(p,1),j0);ʺ ԏ(b2,0,h,Ų,ξ(p,2),j0);}}ż(q ü,q ʊ(k9),0);q Г 1;ʺ ə(q1,0,h,0,ξ(q,1),e0);ʺ ə(q2,0,h,0,ξ(q,2),e0);add_into(ξ(q,2),Ğ(g1,k9),ξ(q,2))Ӛ Š(q Ʒ 3*h+i,ϗ(Ō(e12+i)))Ԗ(g2,k9,q ʊ(4*h));}í projection_weighted_first(ƶ,ΰ,Ğ g,Ğ ş,ĝ q,ĝ p,ĝ Ǡ){ģ m=Ѹ/4,h=m/2,k9=m/4,n=ş Û;ӆ(t,Ķ,g,n,q,p)Ѵ x=Ǡ ϡ,y=Ǡ Ė(2*m,2*m),z=Ǡ Ė(4*m,2*m);ż(x ü,x Ƃ,0u);ż(y ü,y Ƃ,0u)ø(Ů n Į)x[m-1-i]=ş Ĕ ø(ɵ=1 Ͼ Ħ(ȸ,n)Į)y Ƈ i%2&&g Ĕ?Ͻ g Ĕ:g Ĕ Ȇ(y);t.Ŷ(x ʴ,0,z,y,ƨ)Ѵ p0=Ǡ ȑ(h),p1=Ǡ Ė(h,h)ø(Ů h Į){ʦ ĳ=z ư;p1 Ƈ ĳ?Ͻ ĳ:0;p0 Ƈ 2*i+1<m&&m-2-2*i<n?ş[m-2-2*i]:0;}ł tp0=
Ǡ ч,tp1=Ǡ Ė(2*m,m)Ѵ tq1=Ǡ Ė(3*m,m),tq2=Ǡ Ė(4*m,m),Ŏ=Ǡ Ė(5*m,m)ø(Ů h Į){ʦ a=q[7*h+i],b=p[7*h+i];tq1 Ƈ i%2&&a?Ͻ a:a;tq2 Ƈ i%2&&b?Ͻ b:b;}t.ı(p0,0,tp0)Ȇ(p1,0,tp1)Ȇ(tq1 ȑ(h),0,tq1)Ȇ(tq2 ȑ(h),0,tq2)ø(Ů k9 Į)p Ƈ p0 ư;Ɖ(tp0,tq1,Ŏ,ƨ)ø(Ů k9 Į){ʦ ĳ=Ŏ ư+p1 ư;p[h+i]=ĳ>=kP?ĳ-kP:ĳ;}Ù ɝ ǜ ǥ={{tp0,tq2},{tp1,tq1}};t.Ŗ(ǜ,Ŏ,ƨ)ø(Ů k9 Į)p[2*h+i]=Ŏ ư;Ɖ(tp1,tq2,Ŏ,ƨ)ø(Ů k9 Į)p[3*h+i]=Ŏ ư;}Ί ԉ(Ό){ģ e1=m/8 Ū Ŋ(7*Ԋ Ʈ(e1+1)Ʀ Ԋ Ʈ(e1),Ԋ Ʈ(m)ʳ Ԋ Ʈ(m/2));}í projection_last_levels(ƶ,ΰ,ĝ q,ĝ p,ĝ w,ҽ){ģ m=Ѹ/4,e1=m/8,k9=m/4,h=m/2;ĝ c ɚ,pc ɚ;{Ԋ carve(w)ø(ʶ 8 ȝ)c ђ carve ņ(e1+1)ø(ѿ 8 ȝ)pc ђ carve ņ(e1);}ԑ{ɍ ɚ;
#pragma GCC unroll 8
Ԑ
#pragma GCC unroll 7
Ć(ʶ 8 ȝ)Š(c ƫ Ʒ r,k%2?Ӥ(x ƫ):x ƫ);}Ć(ʶ 8 ȝ){Ġ x=q[16*e1+k];c ƫ[e1]=k%2&&x?Ͻ x:x;}ԑ{ɍ ɚ;
#pragma GCC unroll 8
Ć(Ů 8 Į)x Ƈ Ō(p Ʒ 16*(r+i));ʈ(x);
#pragma GCC unroll 8
Ć(ѿ 8 ȝ)Š(pc ƫ Ʒ r,x ƫ);}Ԋ carve_q(q),carve_p(p);ĝ tc ɚ,tp[7],Ƒ Ѐ,cn ɛ,pn ɛ ø(ʶ 8 ȝ){tc ђ carve_q ņ(k9)Ȇ(c ƫ,0,tc ƫ);}Ć(ѿ 7 ȝ){tp ђ carve_p ņ(k9)Ȇ(pc ƫ,0,tp ƫ);}Ć(ł&s:Ƒ)s=carve_q ņ(k9);ʏ(tc[5],2,Ƒ Ŭ),ʏ(tc Ѐ,ӕ,Ƒ ƹ),ʏ(tc ɛ,ӕ,Ƒ ǥ)У cn ђ carve_q ņ(k9+1)ø(ł&s:pn)s=carve_p ņ(k9);{Ù ɝ ù ƹ={{tc ƹ,tc ƹ}},ψ ǥ={{tc ǥ,tc ǥ},{tc ƹ,Ƒ ƹ}},г(ù,cn ƹ ȑ(k9));t.Ŗ(ψ,cn ǥ ȑ(k9));t.Ŗ(o0,cn Ѐ ȑ(k9));}Ġ o9 ɛ={0,ӕ,2,ӕ}У{Ù Ğ i3=c[Ӵ]ƣ f(o9 ƫ)ø(Ů e1 Ɲ)Š(cn ƫ Ȝ,ů(ƪ(Ō(cn ƫ Ȝ),ń(ŭ(Ō(i3 Ȝ),f),kP))))ĩ ŉ Ȣ=cn ƫ[e1]+ŉ(o9 ƫ)*i3[e1];cn ƫ[e1]=Ö(Ȣ Ұ);cn ƫ[k9]=cn ƫ Ŭ;cn ƫ Ŭ Ē}{ɝ ǜ[7]ø(ǉ 4 ų){Ć(Õ b=1;b<=2*j+1 ė b)ǜ[b-1]={tp[2*j+1-b],tc[b]}ĩ Ԅ ɝ>ʂ(ǜ,2*j+1)ĺ(j<2)t.Ŗ(ʂ,pn Ɓ);м(j==2)Ŗ<5>(Ķ,ǜ,pn Ɓ,Ȉ)і Ŗ<7>(Ķ,ǜ,pn Ɓ,Ȉ);add_into(pn Ɓ ȑ(e1),pc[2*j+1],pn Ɓ ȑ(e1));}}Ԋ next_q(q),next_p(p),next_w(w);ĝ tcn ɛ,tpn Ѐ У{tcn ђ next_q ņ(h)Ȇ(cn ƫ,0,tcn ƫ);}Ć(ǉ 3 ų){tpn ˉ next_p ņ(h)Ȇ(pn Ɓ,0,tpn Ɓ);}Ù ĝ n3=next_q ņ(h),j2=next_w ņ(m);Ɖ(tcn ƹ,tcn ƹ,n3)ø(Ů h Ɲ){Ҽ i<k9?Ӥ(ϗ(Ō(cn ǥ Ȝ))):đ;Š(j2 Ȝ,ń(ƪ(x,Ō(n3 Ȝ)),kP));}{Ġ x=j2[k9]+Ґ(cn ǥ[k9]);j2[k9]=x>=kP?
x-kP:x;j2[h]=j2 Ŭ;j2 Ŭ Ē ż(j2 ʊ(h+1),j2 Ƃ,0);}Ù ĝ r1=next_w ņ(h),r3=next_w ņ(h);Ɖ(tpn Ŭ,tcn ƹ,r1);add_into(r1 ȑ(k9),pn ƹ,r1 ȑ(k9))ĩ ɝ ǜ Ѐ={{tpn ǥ,tcn ƹ},{tpn ƹ,tcn ǥ},{tpn Ŭ,tcn Ѐ}};t.Ŗ(ǜ,r3);add_into(r3 ȑ(k9),pn Ѐ,r3 ȑ(k9))Ȇ(j2)ǫ Ŏ=next_q ņ(m);t.Ŷ(r1,0,Ŏ,j2);Ȕ ƒ(;k+8<=Ħ(Ư,h);k+=8)ū(a Ʒ k,ń(ƪ(Ō(r3 Ʒ k),Ō(Ŏ Ʒ k)),kP))ø(;k<Ư ȝ){Ġ x=(k<h?r3 ƫ:0)+Ŏ ƫ;a ђ x>=kP?x-kP:x;}}}ò ý projection_log(ǀ){if(n<=â Ʉ)Ú ʵ Ū Ī(â Ҏ(n));}Ί ʹ(ǀ){Ę â Ԋ ĺ(n<=â Ʉ)Ú 0 ȡ m=â Ҏ(n)Ū ß(â Ȋ Ʈ(m))ʳ Ԋ Ʈ(4*m)+Ԋ Ʈ(â ԉ(m));}í ɮ(ƶ,Ğ g,ҽ,Ѕ){ѻ n=Ư ĺ(n<=Ʉ)Ú projection_direct(g,a)ȿ,p=carve ņ(4*m),w=carve ņ(ԉ(m));ӆ(t,Ķ,g,n,q,p)ц}í weighted_projection(ƶ,Ğ g,Ğ ş,ҽ,Ѕ){ѻ n=Ư;ơ(n>Ʉ&&ş Û==n)ȿ,p=carve ņ(4*m),w=carve ņ(ԉ(m));projection_weighted_first(t,Ķ,g,ş,q,p,ĸ.Ȣ(6*m))ц}}ā Ȗ{ò ī Ϯ(ǣ&x){Ù ī ĳ=x.v ĩ ī l5=(ú о>(ī(ĳ*998244351u))*998244353u+ĳ)>>32 Ū Ħ(l5,l5-998244353u);}í append_word(л,ī ĳ){į θ();į Ӱ.v=ĳ;}í native_compose(ǣ*f,Õ nf,ǣ*g,Õ ng,л,ǀ){nf Ш(nf,n);ng Ш(ng,n)ȉ lg=compose_log(n)+(n>â Ʉ)й(ɠ(nf)Я(ng)Я(n)Я(compose_scratch(n)))Ǭ a ф(nf),
b ф(ng),c к ø(Ů nf Į)ӫ Ħ(f Ĕ.v,f Ĕ.v-Ƀ)ø(Ů ng Į)b Ƈ Ϯ(g Ĕ);compose_imported(t,Ğ(a),b,c,Ĭ(compose_scratch(n)),&t);ž(į,c à,c Û);}í native_weighted_projection(ǣ*g,Õ ng,ǣ*ş,л,ǀ,Ƞ){ng Ш(ng,n)ȉ lg=projection_log(n)+1 ȡ ƃ=ʹ(n)Я(6*â Ҏ(n))й(ɠ(ng)ʳ ß(n)Я(ƃ))Ǭ a ф(ng),b к,w к ø(Ů ng Į)ӫ i?Ϯ(g Ĕ):0 ø(Ů n Į)w Ƈ Ħ(ş Ĕ.v,ş Ĕ.v-Ƀ);weighted_projection(t,a,w,b,Ĭ(ƃ));ž(į,b à,þ);}í native_projection(ǣ*g,Õ ng,л,ǀ){ng Ш(ng,n)ȉ lg=projection_log(n)й(ɠ(ng)Я(n)Я(ʹ(n)))Ǭ a ф(ng),b к ø(Ů ng Į)ӫ Ϯ(g Ĕ);ɮ(t,a,b,Ĭ(ʹ(n)));append_fps_words ə(į,b à,η);}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ compose(Ù Ȼ&j7,Ù Ȼ&ʎ,ў){Ѽ;Ƙ Н ó(j7 Û);ơ(ǁ>=0&&(ʎ ű||ʎ Ŭ Ү))ĺ(ǁ ζ)Ú{};
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(ǁ>=64&&ǁ<=(1<<21)&&j7 Û>8&&ʎ Û>2&&j7[Ħ(j7 Û,Õ(ǁ))-1]!=Ȓ&&ʎ ƹ!=Ȓ&&ʎ ǥ!=Ȓ ȏ){F ă;ç Ȗ::native_compose(j7 à,j7 Û,ʎ à,ʎ Û,ă,ǁ)Ȧ}}
#endif
F f=j7.pre(ǁ),g=ʎ.pre(ǁ);ɞ(f);ɞ(g)ĺ(Ǳ<=1||ȸ<=1){ѫ ĺ(!f ű)ă Г f Ŭ Ȧ}ý ù=1 ǟ(g[ù]Ү)++ù ĺ(ó(Ǳ)>(ǁ-1)/ҵ){f Ř((ǁ-1)/ҵ);ɞ(f)ĺ(Ǳ<=1){ѫ ĺ(!f ű)ă Г f Ŭ Ȧ}}if(ҵ==Џ){ѫ;T Ǹ=1 ø(ö ó(Ǳ)&&(ǈ)i*ù<ǁ Į)ă[i*ù]=f Ĕ*Ǹ,Ǹ*=g[ù]Ȧ}if(ù>1){Ӄ=ù ø(Ż ҵ Ͼ Џ&&ď>1 Į)if(g Ĕ!=Ȓ)ď=Ø gcd(ď,i)ĺ(ď>1){F ʸ((ȸ-1)/ď+1)ø(λ ó(ʸ Û)Į)ʸ Ƈ g[i Ԏ];ʸ=compose(f,ʸ,(ǁ-1)/ď+1);ѫ ø(ö ó(ʸ Û)Į)ă[i Ԏ]=ʸ Ĕ Ȧ}}if(ǁ<=32||Ǳ<=8){F ă ø(Ż(ó(Ǳ))-1;i и i){ă=F::Ź(ă,g).pre(ǁ);ă+=f Ĕ;}ă ә Ȧ}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(Ű Ĝ&&ǁ<=(1<<21)){F ă;ç Ȗ::native_compose(f à,Ǳ,g à,ȸ,ă,ǁ)Ȧ}}
#endif
ϕ;ơ(ä<=(1<<22));ç NTT plan(ä);ħ ϔ>p(Ё),q(ä)ø(ö ó(Ǳ)Į){ġ p Ƈ f Ĕ.v і p Ƈ Ƹ in(f Ĕ ȃ);}Ć(ö Џ Į){ġ q Ƈ g Ĕ.v і q Ƈ Ƹ in(g Ĕ ȃ);}ϔ a=Ƹ one,b=Ƹ one;Ƹ compose(p à,q à,1,ä,a,b);ѫ ø(ö ǁ Į){ġ ă Ƈ T ɪ(p Ĕ)і ă Ƈ T ι(Ƹ į(p Ĕ));}Ѡ}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā ç Ȗ{í native_reversion(ǣ*in,Õ ʭ,л,ǀ){ʭ Ш(ʭ,n)ȉ lg=Ŋ(projection_log(n),ʃ power_log(ԕ))ȡ ɼ=Ŋ(ʹ(n),ß(ԕ)+ʃ Ҫ(ԕ))й(ɠ(ʭ)ʳ ß(n)Я(ɼ))Ǭ f ф(ʭ),g к,a к,ĸ ф(ɼ)ø(Ů ʭ Į)f Ƈ Ϯ(in Ĕ);ɮ(t,f,a,ĸ ȑ(ʹ(n)))Ѵ h=ĸ ȑ(ԕ);ʃ ɗ(a Ė(1),1,h);Ø ˁ(h ü,h Ƃ)Ǎ e=ϵ-á Ǹ(Ö(ԕ),ϵ-2)Ǎ c=in ƹ ˆ;ʃ Ǹ(t,h,e,Ħ(c.v,c.v-Ƀ),g Ė(1),Ԃ(ß(ԕ),ʃ Ҫ(ԕ)));ƙ(į,n);į θ(0);ž(į,g Ʒ 1,ȸ-1);}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ compositional_inverse(Ù Ȼ&f,ў){Ѽ;Ƙ Н ó(Ǳ);ơ(ǁ>=0&&(f ű||f Ŭ Ү))ĺ(ǁ ζ)Ú{}ĺ(ǁ у)Ú F(1);ơ(Ǳ>=2&&f ƹ!=Ȓ)ӵ(compositional_inverse<Ě>(Һ(f),ǁ))і{T w=f ƹ ˆ;ί=ʡ(ǁ,Ǳ);ý ù=2 ǟ(ù<î&&f[ù]Ү)++ù ĺ(ù==î){ѫ;ă ƹ=w Ȧ}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(ǁ>=64&&ǁ<=(1<<21)ȏ){F ă;ç Ȗ::native_reversion(f à,Ǳ,ă,ǁ)Ȧ}
#endif
ϕ;ơ(ä<=(1<<21))ĩ ǧ=ǁ-1 Ѵ&ё=ǆ inverses(ä);F h(n);{ȷ,*q=p+4*ä Ԗ(Ȯ(f),î,q);ʇ(q+î,Ё-î,0u);t.geometric(q,î,T(-1).v,w.v)Ѵ Ÿ=t.reversion_projection(p,q,ä);t.Ŏ(p+ä-n,p+ä-n,Ȯ(ё)+ä-n,n)ø(Е)h Ƈ T ɪ(p[ä-1-i]);ǆ Ζ(h,T(ä-1)*T ɪ(Ÿ));}h=h.log(n);ǆ Ζ(h,-ё[ä-1]);h=h.exp(n);ǆ Ζ(h,w);ѫ;ȩ(h ü,h Ƃ,ă ʷ 1)Ȧ}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ӂ{ˬ ƛ Ώ{Ѽ ͼ Ƽ=ѝ ͼ O=ǅ ͼ Word=ӈ ǆ Word;ċ Ƽ evaluate(Ҥ k5,Ù ħ F>&Ǌ,ħ Ƽ>&Ɣ,ý ä,ý ŀ,F ˀ,ƍ d2=Ś){Ƽ ă(Љ);
#if defined(__x86_64__) || defined(__i386__)
ġ{if(ŀ==8 ȏ){ȟ o4=ä*ŀ Ѵ&plan=ǆ ƀ(Ŋ(1,o4/2));Ű CompositionWords ĸ ĺ(!d2)ĸ.reset(new(ɴ)ϔ[2*o4])ø(ý Ȍ=1;Ȍ<=ä;Ȍ<<=1){ȟ î=o4/Ȍ,Ɵ=î/2 ø(ý ʛ=Ȍ;ʛ<2*Ȍ ė ʛ){ȟ Ų=(ʛ-Ȍ)*î ĺ(Ų>=ó(Љ))Ϸ ɓ ԍ=Ȯ(ˀ)+Ų ĺ(î==8){Ű evaluate_leaf8(Ȯ(ă)+Ų,ԍ,Ȯ(Ǌ[ʛ]),Ǌ[ʛ]Û,Ȯ(k5)+Ų,ʡ(8,Љ-Ų));}м(Ų+Ɵ>=ó(Љ)){ĵ(ԍ+Ɵ,Ɵ,ԍ);}м(!Ɣ Ӯ ű){ɓ a=d2?Ȯ(Ɣ Ӯ):ĸ Ӳ;ɓ b=d2?Ȯ(Ɣ ѱ):ĸ Ӳ+o4 ĺ(!d2){ĵ(Ȯ(Ɣ Ӯ),î,a)Ԗ(Ȯ(Ɣ ѱ),î,b);}Ƹ Ą Ҭ(ԍ,î);Ű Ŏ(a,a,ԍ,î);Ű Ŏ(b,b,ԍ,î);Ƹ Ą fft ԏ(a,î);Ƹ Ą fft ԏ(b,î)ĩ Word norm=ǘ î;ҡ(ԍ,b+Ɵ,Ɵ,norm);ҡ(ԍ+Ɵ,a+Ɵ,Ɵ,norm);}ŝ{Ť Word ϊ ɚ,l8 ɚ;ơ(î==16);Ű evaluate_middle8(ϊ,l8,ԍ,Ȯ(Ǌ ѱ),Ǌ ѱ Û,Ȯ(Ǌ Ӯ),Ǌ Ӯ Û)Ԗ(ϊ,8,ԍ)Ԗ(l8,8,ԍ+8);}}}Ѡ}}
#endif
ł dfs=Ѯ(ł&&self,ý ʛ,ý l,ý r,F ԍ)->Ĉ{if(l*ŀ>=ó(Љ))Ú ȉ î=(r-l)*ŀ,Ɵ=î/2 ĺ(r-l у||î<=8){
#if defined(__x86_64__) || defined(__i386__)
ġ{if(î==8 ȏ){ȟ þ=ʡ(8,Љ-l*ŀ);Ű evaluate_leaf8(Ȯ(ă)+l*ŀ,Ȯ(ԍ),Ȯ(Ǌ[ʛ]),Ǌ[ʛ]Û,Ȯ(k5)+l*ŀ,þ)а}}
#endif
Ƽ ө(î)ø(ö î Į)Ć(ý j Ē j+i<î&&j<ó(Ǌ[ʛ]Û)ų)ө[î-1-i-j]+=ԍ Ĕ*Ǌ[ʛ]Ɓ ø(Ż l*ŀ Ͼ ʡ(r*ŀ,Љ)Į){T ĳ ƒ(ý j=î-1;j и j)ĳ=ĳ*k5 Ĕ+ө Ɓ;ă Ƈ ĳ;}Ú;}ȟ ς=(l+r)/2 ĺ(ς*ŀ>=ó(Љ)){self(self,2*ʛ,l,ς,F(ԍ ʷ Ɵ,ԍ Ƃ))а}F ϊ,l8 ĺ(!Ɣ Ӯ ű){Ә(ú Ƽ&>(ԍ));F a=d2?ƅ(Ɣ Ӯ):Ɣ Ӯ;F b=d2?ƅ(Ɣ ѱ):Ɣ ѱ;ǆ ȇ(a,a,ԍ,î);ǆ ȇ(b,b,ԍ,î);ǆ Ʊ(a);ǆ Ʊ(b)ĩ Word norm=ǘ î;ҡ(Ȯ(b),Ȯ(b)+Ɵ,Ɵ,norm);ҡ(Ȯ(a),Ȯ(a)+Ɵ,Ɵ,norm);b Ř(Ɵ);a Ř(Ɵ);ϊ=ƅ(b);l8=ƅ(a);}ŝ{ϊ Ř(Ɵ);l8 Ř(Ɵ);ƍ done=Ś;
#if defined(__x86_64__) || defined(__i386__)
ġ{if(î==16 ȏ){Ű evaluate_middle8(Ȯ(ϊ),Ȯ(l8),Ȯ(ԍ),Ȯ(Ǌ ѱ),Ǌ ѱ Û,Ȯ(Ǌ Ӯ),Ǌ Ӯ Û);done=Ɔ;}}
#endif
if(!done){F a=F::Ź(ԍ,Ǌ ѱ),b=F::Ź(ԍ,Ǌ Ӯ)Ԗ(a ʷ Ɵ,Ɵ,ϊ ü)Ԗ(b ʷ Ɵ,Ɵ,l8 ü);}}self(self,2*ʛ,l,ς,ƅ(ϊ));self(self,2*ʛ+1,ς,r,ƅ(l8));};dfs(dfs,1,0,ä,ƅ(ˀ))Ȧ}ċ ħ F>build(Ù ѝ&xs,ý&ä,ý ŀ,ƍ l4,ħ ħ T>>*Ɣ=Ƿ){ȟ f5=(ó(Ԉ)+ŀ-1)/ŀ ǟ(ä<f5)ä<<=1;ħ F>Ǌ(Ё)ĺ(Ɣ)Ɣ->resize(Ё)ø(ö ä Į){if(i>=f5){Ǌ[ä+i]={1};ӊ;}F p;p.reserve(ŀ+1);p ӱ(Ӹ)ø(ý j=i*ŀ;j<ʡ((i+1)*ŀ,Ԉ)ų){ȟ d=ó(p Û)-1 ĺ(l4){p ӱ(0)ø(ң=d+1;k>=1;--k)p ƫ-=xs Ɓ*p[k-1];}ŝ{p ӱ(p Ӱ)ø(ң=d;k>=1;--k)p ђ p[k-1]-xs Ɓ*p ƫ;p Ŭ*=-xs Ɓ;}}Ǌ[ä+i]=ƅ(p);}Ć(Ż ä-1;i>=1;--i){if(Ǌ ư Û у&&Ǌ ư Ŭ==Ӹ){Ǌ Ƈ Ǌ ɔ;ӊ;}ý d9=31-__builtin_clz((ϔ)i);ί=(ä>>d9)*ŀ ĺ(Ɣ&&î>=32){ł&a=(*Ɣ)ɔ Ѵ&b=(*Ɣ)ư Ѵ extend_transform=Ѯ(Ƽ&a,ʧ p){if(a ű){a Ř(î);ȩ(p ü,p Ƃ,a ü);Ә(a);}ŝ{ȟ Ɵ=î/2;Ƽ Ȏ(Ɵ)Ԗ(p ü,ʡ(Ɵ,p Û),Ȏ ü)ĺ(ó(p Û)>Ɵ)Ȏ Ŭ-=p[Ɵ];ǆ ɘ(Ȏ,Ӹ,ǆ ntt_roots(î)[î/2]);Ә(Ȏ);a ӯ(a Ƃ,Ȏ ü,Ȏ Ƃ);}};extend_transform(a,Ǌ ɔ);extend_transform(b,Ǌ ư);ѝ c(î);ǆ ȇ(c,a,b,î)ĺ(i>1)(*Ɣ)Ƈ c;ǆ Ʊ(c)ĩ ǧ=(ý)(Ǌ ɔ Û+Ǌ ư Û-1);Ǌ Ĕ Ř(n)ĩ T ё=T ι(ǘ î);ġ ҡ(Ȯ(Ǌ Ĕ),Ȯ(c),Ħ(n,î),ё ȃ)і Ć(ȓ Ħ(n,î)ų)Ǌ Ĕ ˉ c Ɓ*ё ĺ(n==î+1){Ù T Μ=Ǌ ɔ Ӱ*Ǌ ư Ӱ;Ǌ Ĕ Ŭ-=Μ;Ǌ Ĕ[î]=Μ;}}ŝ{Ǌ Ƈ F
::Ź(Ǌ ɔ,Ǌ ư);}}Ú Ǌ;}};}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ӂ{ā ˎ{Ę ʃ Č ͼ ʃ Half ͼ ʃ ć ͼ ʃ inverse_log ͼ ʃ ȵ ͼ ʃ Ǟ;ò ӄ ϵ=ʃ ϵ;Ũ ě:public ʃ ě{Ϳ ě(Õ č):ʃ ě(č){}ĝ ś(ǀ){ł span=ʃ ě::ś(n)ĺ(!zero_initialized())ż(span ü,span Ƃ,0u)Ū span;}};Ϧ{Ę ā ʃ ê;}}ā ˎ{Ϧ{Ȅ add_mod(ɍ,ĭ y){Ú ń(ƪ(x,y),kP);}Ȅ subtract_mod(ɍ,ĭ y){Ú ń(é(ƪ(x,ɟ),y),kP);}Ȅ to_montgomery(ɍ){Ú ń(ŭ(x,ƭ(kR)),kP);}í forward_small(ҳ f,Ό,Ȕ,М){if(m у)Ú ȡ h=m/2 ƣ s=ʮ(ķ,k)ʋ{Ģ lo=f Ɓ,hi=ń(ŭ(f ǹ,s),kP);f ˉ add_mod(lo,hi),f ǹ=subtract_mod(lo,hi);}forward_small(f,h,Ӵ,ķ);forward_small(f+h,h,Ӵ+1,ķ);}í Ҧ(ҳ f,Ό,Ȕ,Ɠ ô){if(m у)Ú ȡ h=m/2;Ҧ(f,h,Ӵ,ô);Ҧ(f+h,h,Ӵ+1,ô)ƣ s=ʮ(ô,k)ʋ{Ģ a=f Ɓ,b=f ǹ;f ˉ add_mod(a,b),f ǹ=ń(ŭ(é(ƪ(a,ɟ),b),s),kP);}}í transpose8(ĭ(&r)ɚ){ĭ t ɚ,u ҹ(ö 8 ƽ 2)ͻ,t Υ ǿ(r Ĕ,r Υ)ø(ö 8 ƽ 4)Ć(ˋ 2 ȝ)u[i ʳ k]=ɻ(t[i+k],t[i+k+2]),u[i ʳ k+1]Ǿ(t[i+k],t[i+k+2])ɫ ѹ,з}ԅ montgomery_scalar(Ö x,Ö y){ӄ kInverseR=á Ǹ(kR,ӕ)Ū ʀ(ʀ(x,y),kInverseR);}ˮ ʩ(һ,Ό,Ҡ,Ȕ,Ņ c){ȱ,p=ɟ,r=ĕ(kR);ɍ[33],xo[33],y[33],yo[33];
#pragma GCC unroll 33
Ć(ɵ Ē i<=m Į)x Ƈ Ō(ύ i),xo Ƈ ñ(x Ĕ,32);
#pragma GCC unroll 33
Ć(Ŀ<=k ų)y ˉ Ō(b Ʀ j),yo ˉ ñ(y Ɓ,32)Ъ;
#pragma GCC unroll 65
Ć(Ӆ<=m+k ė t){н=đ,Ȏ=đ ȡ ù=t>k?t-k:0,Ȣ Ш(t,m);
#pragma GCC unroll 17
Ć(ɵ=ù;i<=Ȣ Į){Ŧ=ð(Ŧ,Þ(x Ĕ,y[t-i]));Ȏ=ð(Ȏ,Þ(xo Ĕ,yo[t-i]));}if(Ȣ-ù>=12)Ŧ=ѷ(Ŧ),Ȏ=ѷ(Ȏ);ĥ Š(c ϝ,ů(ǝ));}}í multiply_lanes_any(һ,Ό,Ҡ,Ȕ,Ņ c){if(m==k){switch(m){case 1:Ú ʩ(a,1,b,1,c)ϼ 2:Ú ʩ(a,2,b,2,c)ϼ 4:Ú ʩ(a,4,b,4,c)ϼ 8:Ú ʩ(a,8,b,8,c)ϼ 16:Ú ʩ(a,16,b,16,c);}}ʩ(a,m,b,k,c);}Ũ Stack{Ϳ Stack(ĝ i6):base_(i6 à),ͽ{ģ ӟ=ß(n)ĺ(top_+ӟ>size_)ǩ;Ǜ p=base_+top_;top_+=ӟ Ū p;}Ⱦ Ņ base_;Õ size_,top_ Ē};}Ũ ȣ{Ⱥ ċ ɿ č(ý Δ){Ú ß(2*á ϛ(Ŋ(Δ,ʵ)));}ȣ(ě&ŋ,ý Δ):lg_max_(Δ){if(Δ<3||Δ>Č::Ҕ)ǩ ȉ lg=Ŋ(Δ,ʵ)ȡ n8=á ϛ(lg),entries=(ǻ lg)Ș ø(ң=3;k<=Δ ȝ)leaf_scale_ ђ á Ǹ(Ö(1)<<(k-3),â ӕ);}ý Δ()Ù{Ú lg_max_;}Ĉ ı(Ğ in,Õ ƌ,ɐ)Ù{ƞ ĩ Ɏ src=Ο ĺ(ǲ<64)Ú small(src,į,0);run(į,src,Ǆ{Ə},0);}Ĉ ɇ(Ğ in,Õ ƌ,ɐ)Ù{ƞ;ʐ(2*ǲ)ĩ Ɏ src=Ο ĺ(ǲ<64)Ú small(src,į,1)ĩ Ҟ}Ĉ ć(Ğ in,ɐ,Ö c=1)Ù{ƞ;ʐ(ǲ)ĺ(ǲ<64){ģ nv=ǲ/8;ϥ щ f ˉ Ō(in ʥ j);Ҧ(f,nv,0,ō)ƣ s(ŧ(ǲ,c))щ Š(į ʥ j,ń(ŭ(f Ɓ,s),kP))а}run(į,Ɏ(Ƿ,0,0),Ɉ{ō,in à},ŧ(ǲ,c));}Ĉ inverse_upper(Ğ in,ɐ,Ö c=1)Ù{ƞ;ʐ(2*ǲ)ȡ Ӿ ƣ s(ŧ(ǲ,c))ĺ(ǲ<64){ϥ щ f ˉ Ō(in ʥ j);Ҧ(f,nv,1,ō)щ v ˉ ń(ŭ(f Ɓ,s),kP)а}Ù Ǉ Ŝ(Ə,ō,Ɉ{ō,in à-ǲ})ǡ{ƾ 4 Ȳ ő(į ʥ t*h,h,4+t
)ĩ Τ w(ō,1)ʋ{ϥ={v Ɓ,Ӌ,v ɋ,v Ɋ};υ(f,w)Ȳ v[j+t*h]=ń(ŭ(f ѯ,s),kP);}Ú;}ƾ 2;ő(į à,h,2);ő(į ʥ h,h,3)ƣ scale_z(ʀ(ŧ(ǲ,c),ō ƹ))ʋ{Ģ u=v Ɓ,w=Ӌ;v ˉ ń(ŭ(ƪ(u,w),s),kP),Ӌ=ń(ŭ(Ǚ(u,w),scale_z),kP);}}Ĉ leaf_products(һ,Ҡ,ɐ)Ù{ѻ Ȟ=ǲ/8;ɥ;Ε(Ļ Ŭ,Ō(a),Ϛ(Ə,0))ø(ӛ Ȟ ė p){if(p+1<Ȟ)Ε(Ļ[(p+1)&1],Ō(ύ p+8),Ϛ(Ə,p+1));Š(į ʥ p,ń(Ǽ(Ļ[p&1],b Ʀ p),kP));}}Ĉ standard_to_lanes(Ɠ in,Õ ď,ǀ,ȧ)Ù{ƞ ø(ӛ n/8 ė p){ĭ f ҹ(Õ l Ē l<8 ė l)f[l]ӷ(in+l Ԏ Ʀ p);transpose8(f)ƣ z=ʮ(Ə,p);ĭ lo ɛ,hi ɛ Ȳ{Ģ zb Ҝ(f[t+4],z);lo ѯ=ƪ(f ѯ,zb),hi ѯ=Ǚ(f ѯ,zb);}forward_h1(lo,Τ(Ə,2*p));forward_h1(hi,Τ(Ə,2*p+1))Ȳ Š(ѓ 64*p ϝ,ů(lo ѯ)),Š(ѓ 64*p+32 ϝ,ů(hi ѯ));}}Ĉ leaf_product_sums(һ,Ҡ,Ɠ c,Ɠ d,ɐ)Ù{ѻ Ȟ=ǲ/8;ɥ ǥ;Ε(Ļ Ŭ Ŭ,Ō(a),Ϛ(Ə,0));Ε(Ļ Ŭ ƹ,Ō(c),Ϛ(Ə,0))ø(ӛ Ȟ ė p){if(p+1<Ȟ){Œ w=Ϛ(Ə,p+1);Ε(Ļ[(p+1)&1]Ŭ,Ō(ύ p+8),w);Ε(Ļ[(p+1)&1]ƹ,Ō(c Ʀ p+8),w);}Ģ ab=Ǽ(Ļ[p&1]Ŭ,b Ʀ p),cd=Ǽ(Ļ[p&1]ƹ,d Ʀ p);Š(į ʥ p,ů(ϓ));}}Ŵ ѣ(һ,Ҡ,ɐ){ƞ ø(Ů ǲ Ɲ)Š(į Ȝ,ń(Ŕ(Ō(a+i),Ō(b+i)),kP));}Ŵ pointwise_product_sums(һ,Ҡ,Ɠ c,Ɠ d,ɐ){ƞ;ȱ,p=ɟ ø(Ů ǲ Ɲ){Ҽ Ō(a+i),y ӷ(b+i),z ӷ(c+i),w ӷ(d+i);н=ð(Þ(x,y),Þ(z,w));ĭ Ȏ=ð(ɹ,Þ(ñ
(z,32),ñ(w,32)));ĥ Š(į Ȝ,ń(ǝ,kP));}}Ⱦ Ö ŧ(ǀ,Ö c)Ù{ʦ lg=Ī(n)ĺ(lg>Č::Ҕ)ǩ Ū ʀ(leaf_scale_[lg],c);}Ĉ ʐ(ǀ)Ù{if(!ϩ(n)||n<8||Ī(n)>lg_max_)ǩ;}â Ɏ source(Ğ in,Õ ƌ,ǀ)Ù{ʐ(n)ĺ(ƌ>n||in Û>n-ƌ)ǩ Ū â Ɏ(in à,in Û,ƌ);}Ĉ small(Ù â Ɏ&src,ɐ,Õ ŀ)Ù{ѻ nv=ǲ/8;ϥ щ f ˉ src(j);forward_small(f,nv,ŀ,Ə)щ Š(į ʥ j,f Ɓ);}Ϥ run(ҽ,Ù â Ɏ&in,Ϩ,Ö ŧ)Ù{ƞ ĩ Ǉ Ŝ(Ə,ō,b9)Ϊ{Ι(v,h,Ȉ,ō,ƭ(ŧ));}ŝ{ƾ 2;ʠ forward_top2(in,v,h)ѳ Ѓ(Ƭ::Ƚ)Ӧ(v,h,Ȉ,ŧ);}}ý lg_max_;Ņ Ə,*ō;Ö leaf_scale_[Č::Ҕ+1]={};};ƛ LaneLayout{ċ ɿ kWords=8;ċ ӄ kBase=32 ͼ ȼ=â ĭ;ƛ Node{ȼ ũ,Μ;};Ŵ Ź(һ,Ό,Ҡ,Ȕ,Ņ c){â multiply_lanes_any(a,m,b,k,c);}ċ Õ î(Ö ũ){Ú ɬ(Ŋ<Ö>(ũ,1));}Ŵ Ŏ(Ù ȣ&,һ,Ҡ,ɐ){ȣ::ѣ(a,b,į);}ċ ȼ Μ(Ɠ c,ȼ ũ){Ù ȼ ȯ=ň(_mm256_slli_epi32(ũ,3),ɜ)Ū _mm256_i32gather_epi32(õ ȟ*>(c),ȯ,4);}ċ ȼ ʂ(ȼ x,ȼ y){Ú ň(x,y);}ċ ȼ lead_product(ȼ x,ȼ y){Ú â ń(â Ŕ(x,y),â kP);}ċ ȼ top(ȼ ũ,ȼ Μ,Õ î){Ú _mm256_and_si256(_mm256_cmpeq_epi32(ũ,â ĕ(Ö(î))),Μ);}Ŵ ʫ(Ņ c,ȼ x){â Š(c,â subtract_mod(â Ō(c),x));}Ŵ ƪ(Ņ c,ȼ x){â Š(c,â add_mod(â Ō(c),x));}Ŵ put(Ņ c,ȼ x){â Š(c,x);}};ƛ Ǘ{ċ ɿ kWords=1;ċ ӄ kBase Ē Ę ȼ=Ö;ƛ Node
{ȼ ũ,Μ;};Ŵ Ź(Ɠ,Õ,Ɠ,Õ,Ņ){}ċ Õ î(Ö ũ){Ú ʘ(8,ɬ(ũ));}Ŵ Ŏ(Ù ȣ&t,һ,Ҡ,ɐ){t.leaf_products(a,b,į);}ċ ȼ Μ(Ɠ c,ȼ ũ){Ú c[ũ];}ċ ȼ ʂ(ȼ x,ȼ y){Ú x+y;}ċ ȼ lead_product(ȼ x,ȼ y){Ú â montgomery_scalar(x,y);}ċ ȼ top(ȼ ũ,ȼ Μ,Õ î){Ú ũ==î?Μ:0;}Ŵ ʫ(Ņ c,ȼ x){*c=*c>=x?*c-x:*c+â Ͻ x;}Ŵ ƪ(Ņ c,ȼ x){*c=*c+x>=â kP?*c+x-â kP:*c+x;}Ŵ put(Ņ c,ȼ x){*c=x;}};ƛ DropTransforms{Ĉ ŕ()(Õ,Õ,Ğ)Ù{}};Ő Layout,Ũ Leaves,Ũ Keep=DropTransforms>Ũ Ώ{Ⱥ Ę ȼ=ӈ Ƶ ȼ ͼ Node=ӈ Ƶ Node;ċ æ ƍ kPlaces=requires(Keep k,ɵ){k.place(i,i,i,i);};ƛ Root{ĝ ļ;Node ʛ;Õ î;};ċ Õ ƃ(Ƞ,Õ o4){Ú ß(þ+1)Ʀ ß(Ƶ kWords*(Ƶ î(Ö(o4))+1))+4096;}Ώ(Ù ȣ&t,Ù Leaves&Ȟ,Ѕ,Keep keep={}):t_(t),leaves_(Ȟ),keep_(keep),ʄ(ĸ),Ȑ(Ȟ.þ()),Ͱ(ʄ ņ(Ȑ+1)){Ͱ Ŭ ƒ(ѿ Ȑ ȝ)Ͱ[k+1]=Ͱ ƫ+Ȟ.ũ(k);}Root ˀ(){ģ þ=Ȑ,î=Ƶ î(ũ(0,þ));ɿ w=Ƶ kWords;Ǜ c=ʄ ņ(w*(î+1))ĺ(þ у||ũ(0,þ)<=Ƶ kBase){ʇ(c,w*(î+1),0)Ū{{c,w*(î+1)},ļ(0,þ,c),î};}Ǜ a=ʄ ņ(w*î)ȡ ς=split(0,þ);Ņ left_out=a,*right_out=c Ѓ(kPlaces)left_out=keep_.place(0,ς,þ,w*î),right_out=left_out Я(w*î)ĩ Node ϊ=build(0,ς,left_out,î),l8=build(ς,þ,right_out,î);Ƶ Ŏ(t_,left_out
,right_out,{a,w*î})Ѓ(requires{keep_.ˀ(Ğ{});})keep_.ˀ(Ğ(a,w*î));t_.ć({a,w*î},{c,w*î})Ь Ѷ put(c+w*î,top)Ū{{c,w*(î+1)},ʛ,î};}Õ split(Ї,п)Ù{Ġ Ʃ=Ͱ[lo]+(Ͱ[hi]-Ͱ[lo])/2;Õ ς=Õ(Ø lower_bound(Ͱ+lo+1,Ͱ+hi-1,Ʃ)-Ͱ)ĺ(ς>lo+1&&Ͱ[ς]>=Ʃ&&Ʃ-Ͱ[ς-1]<Ͱ[ς]-Ʃ)--ς Ū ς;}Ö ũ(Ї,п)Ù{Ú Ͱ[hi]-Ͱ[lo];}Ⱦ Node ļ(Ї,п,Ņ c)Ù{if(hi-lo у)Ú leaves_.Ō(lo,c);ɿ w=Ƶ kWords,kMax=ʘ(Ƶ kBase,1);ƛ Part{Ņ c;Õ ũ;};Part part[kMax];љ i6 ǥ[w*2*kMax];Ƞ=hi-lo;Ņ to=i6 Ŭ;ȼ ũ{}ø(ѿ þ ȝ){Ù ȼ d=leaves_.Ō(lo+k,to).ũ;ũ=k?Ƶ ʂ(ũ,d):d;part ђ{to,leaves_.ũ(lo+k)};to+=w*(part ƫ.ũ+1);}Ć(Õ Ȍ=1;þ>1 ė Ȍ){to=þ==2?c:i6[Ȍ&1];Õ next ƒ(Ȕ Ē k+1<þ;k+=2){Ù Part&a=part ƫ,&b=part[k+1];Ƶ Ź(a.c,a.ũ,b.c,b.ũ,to);part[next++]={to,a.ũ+b.ũ};to+=w*(a.ũ+b.ũ+1);}if(þ%2){Ù Part&a=part[þ-1]Ԗ(a.c,w*(a.ũ+1),to);part[next++]={to,a.ũ};}þ=next;}Ú{ũ,Ƶ Μ(c,ũ)};}Node build(Ї,п,ȧ,Õ ɖ){ɿ w=Ƶ kWords ĺ(hi-lo у||Ѝ<=Ƶ kBase){Ù Node ʛ=ļ(lo,hi,į);t_.ı({į,w*(Ѝ+1)},0,{į,w*ɖ})Ӕ}ģ î=Ƶ î(Ѝ),č=w*î,Ѥ;Ң,*b,*c;Õ ς Ѓ(kPlaces){ς=split(lo,hi);a=keep_.place(lo,ς,hi,č),b=a Я(č),c=ʄ ņ(č+w);}ŝ{a=ɖ>=2*î?ѓ č:ʄ ņ(č
);b=c=ʄ ņ(č+w);ς=split(lo,hi);}Ù Node ϊ=build(lo,ς,a,î),l8=build(ς,hi,b,î)Ь;Ƶ Ŏ(t_,a,b,{į,č})ĺ(ɖ>î){t_.ć({į,č},{c,č})Ѷ ʫ(c,top);t_.ɇ({c,č},0,{ѓ č,č});Ƶ ƪ(c,top);Ƶ put(c+č,top)ø(Ό=2*î;m<ɖ;m*=2)t_.ɇ({c,č+w},0,{ѓ w*m,w*m});}ʄ ȍ Ӕ}Ù ȣ&t_ ĩ Leaves&leaves_;Keep keep_;â Stack ʄ;Õ Ȑ;Ņ Ͱ;};í lane_columns(Ɠ c,Ƞ,Ġ(&ä)ɚ,Ǜ(&į)ɚ){ѻ i5=*Ø max_element(ä,ä+8)ø(ǉ i5 Ӗ){ĭ r ҹ(Ů 8 Į)r Ƈ j+i<þ?Ō(c Ʀ(j+i)):đ;transpose8(r)ø(ý l Ē l<8 ė l)if(j<ä[l])ū(į[l]+j,r[l]);}}}ā ˎ{Ϧ{ĉ Ȅ ˍ(н,ĭ Ȏ,Õ Ơ){ȱ,p=ɟ,r=ĕ(kR)Ъ ĺ(Ơ>12)Ŧ=ѷ(Ŧ),Ȏ=ѷ(Ȏ);ĥ Ú ů(ǝ);}Ą<Õ K,Õ C,Õ B>ˮ э(Ɠ w,Ɠ q,ȧ){Ǝ(K<=16);ĭ wo[K+C],qo[K+1];
#pragma GCC unroll 32
Ć(Ů K+C Į)wo Ƈ ñ(Ō(w Ʀ i),32);
#pragma GCC unroll 17
Ć(Ŀ<=K ų)qo ˉ ñ(Ō(q Ʀ j),32)ø(Õ t0 Ē t0<C;t0+=B){н[B],Ȏ[B];
#pragma GCC unroll 8
Ć(Õ s Ē s<B ė s)Ŧ[s]=Ȏ[s]=đ;
#pragma GCC unroll 17
Ć(Ŀ<=K ų){Ģ y ӷ(q Ʀ j);
#pragma GCC unroll 8
Ć(Õ s Ē s<B&&t0+s<C ė s){Ŧ[s]=ð(Ŧ[s],Þ(Ō(w Ʀ(K+t0+s-j)),y));Ȏ[s]=ð(Ȏ[s],Þ(wo[K+t0+s-j],qo Ɓ));}}
#pragma GCC unroll 8
Ć(Õ s Ē s<B&&t0+s<C ė s)Š(į Ʀ(t0+s),ˍ(Ŧ[s],Ȏ[s],K+1));}}Ɩ í э(Ɠ w,Ȕ,Ɠ q,Ƞ,ȧ){Ć(Ӆ<þ ė t){н=đ,Ȏ=đ ø(Ŀ<=k ų){Ҽ Ō(w Ʀ(k+t-j)),y ӷ(q Ʀ j)ɳ}Š(į ϝ,ˍ(Ŧ,Ȏ,k+1));}}ƛ Ѣ{Ɠ k5;Õ n2;Ƞ()Ù{Ú n2;}Ö ũ(Õ)Ù{Ú 1;}ǎ Node Ō(Ȕ,Ņ c)Ù{Ģ Μ=to_montgomery(é(ɟ,â Ō(k5 Ʀ k)));Š(c,ĕ(kR));Š(c+8,Μ)Ū{ĕ(1),Μ};}};ƛ LaneProducts{Ɠ c;Õ ď;Ö h4;Ƞ()Ù{Ú 8;}Ö ũ(Õ)Ù{Ú h4;}Ǘ::Node Ō(Ȕ,ȧ)Ù{ĵ(c+k Ԏ,h4+1,į)Ū{h4,į[h4]};}};Ũ Ύ{Ⱥ ƛ Keeper{Ύ*Š;Ņ place(Õ,Õ ς,Õ,Õ č)Ù{Ú Š->place(ς,č);}Ĉ ˀ(Ğ ƀ)Ù{Š->root_=ƀ;}};Ĉ reset(ĝ č,ĝ ȯ){words_=č,used_=0,pair_=ȯ à;}Ņ place(Õ ς,Õ č){ģ ӟ=2 ӎ(č)ĺ(used_+ӟ>words_ Û)ǩ;pair_[ς]=Ö(used_);used_+=ӟ Ū words_ Ʒ pair_[ς];}Ɠ pair(Õ ς)Ù{Ú words_ Ʒ pair_[ς];}Ğ ˀ()Ù{Ú root_;}Ⱦ Ğ root_;ĝ words_;Õ used_ Ē Ņ pair_=Ƿ;};Ő Layout,Ũ Tree>Õ kept_words(Ù Tree&Ǌ,Ї,п){if(hi-lo у||Ǌ.Ѝ<=Ƶ kBase)Ú 0 ȡ Χ,č=Ƶ kWords*Ƶ î(Ǌ.Ѝ)Ū 2 ӎ(č)+kept_words<Layout>(Ǌ,lo,ς)+kept_words<Layout>(Ǌ,ς,hi);}Ą<Õ D>Ĉ product_level(Ɠ c7,ȧ,Õ j3){Ć(Ů j3 Į)ʩ(c7 Ʀ(D+1)*(2*i),D,c7 Ʀ(D+1)*(2*i+1),D,į Ʀ(2*D+1)*i);}ɿ middle_block(Õ d){Ú d>=16?0:d==8?3:d==4?4:2;}Ą<Õ D>
Ĉ middle_level(Ɠ k3,Ɠ ƕ,ȧ,Õ j3){Ć(Ů j3 Į){Ɠ w=k3 ү*D*(i/2),*q=ƕ Ʀ(D+1)*(i^1)Ѓ(middle_block(D)ζ)э(w,D,q,D,į Ʀ D*i)і э<D,D,middle_block(D)>(w,q,į Ʀ D*i);}}Ő F>Ĉ with_degree(Õ d,F&&f){switch(d){case 1 Ѭ 1>()ϼ 2 Ѭ 2>()ϼ 4 Ѭ 4>()ϼ 8 Ѭ 8>()ϼ 16 Ѭ 16>();default:ǩ;}}Ũ BaseTree{protected:BaseTree(Ù Ѣ&n2,Ņ Ƴ):slots_(n2),ϭ(Ƴ){}Ņ Ӡ(Ї,Ƞ,Ɠ*ƕ){Ņ at=ю ø(ѿ þ ȝ)slots_.Ō(lo+k,at ү*k);ƕ Г at;at+=16*þ ø(Õ d=1,j=1;2*d<þ;d*=2,++j){with_degree(d,Ѯ<Õ D>(){Ĺ(D<=ǎ kBase/4)product_level<D>(ƕ[j-1],at,þ/(2*D));});ƕ ˉ at;at+=8*(2*d+1)*(þ/(2*d));}Ú at;}Ĉ descend_levels(Ї,Ƞ,Ɠ w,Ɠ Ù*ƕ,Ǜ*Ǯ){Ɠ k3=w ø(Õ d=þ/2,j=Ī(þ)-1;;d/=2,--j){Ǜ į=d у?ϭ Ʀ(lo-lo_):Ǯ[j&1];with_degree(d,Ѯ<Õ D>(){middle_level<D>(k3,ƕ Ɓ,į,þ/D);})ĺ(d у)Ú;k3=į;}}Ɠ build(Õ v,Ї,п){Ǜ c=free_ ĺ(hi-lo у){slots_.Ō(lo,c);free_+=16 Ū c;}ģ ς=lo+(hi-lo)/2;α[2*v]=build(2*v,lo,ς);α[2*v+1]=build(2*v+1,ς,hi)ĺ(v у)Ú Ƿ;Ǜ į=free_;multiply_lanes_any(α[2*v],ς-lo,α[2*v+1],hi-ς,į);free_+=8*(hi-lo+1)Ū į;}Ĉ descend(Õ v,Ї,п,Ɠ w){if(hi-lo у)Ú Š(ϭ Ʀ(lo-lo_),Ō(w))ͷ э(w,hi-ς,α[2*v+1],ς-lo
,ϊ);э(w,ς-lo,α[2*v],hi-ς,l8);descend(2*v,lo,ς,ϊ);descend(2*v+1,ς,hi,l8);free_=ϊ;}љ ю[8*320];Ɠ α[64];Ņ free_=ю ĩ Ѣ&slots_;Ņ ϭ;Õ lo_ Ē};Ũ BaseDescent:private BaseTree{Ⱥ BaseDescent(Ù Ѣ&n2,Ņ Ƴ):BaseTree(n2,Ƴ){}Ĉ run(Ї,п,Ɠ w){ģ þ=hi-lo ĺ(þ у)Ú Š(ϭ Ʀ lo,Ō(w))ĺ(ϩ(þ)){Ɠ ƕ[6];Ņ at=Ӡ(lo,þ,ƕ);Ǜ Ǯ ǥ={at,at Ʀ þ}Ū descend_levels(lo,þ,w,ƕ,Ǯ);}ӽ}};}Ũ ʨ{Ⱥ ċ Õ č(Ό){б,þ=m8/8,ţ=ʌ(þ)+1 ȡ o3=Ϣ ȡ kept_lanes=ţ*(32*þ+64*(þ/32+1))ȡ kept_top=14 ӎ(o3)Ū ȣ Ʈ(ɶ(m8))Я(m8)Я(LaneTree::ƃ(þ,þ))Я(kept_lanes)Я(þ)Я(8*((þ+8)/8*8))Я(TopTree::ƃ(8,m8))Я(kept_top)Я(8);}ċ Õ descend_words(Ό){б ȡ h5=ǎ î(Ö(m8/8)),o3=Ϣ Ū 2 ӎ(8*ʘ(8,h5))+5 ӎ(o3)+4 ӎ(8*h5)+4096;}ʨ(ě&ŋ,Ğ k5):m8_((Љ+7)/8*8),Ȑ(m8_/8),t_(ŋ,ɶ(m8_)){ƞ;Ǜ k0 ф(m8_)à;ȩ(k5 ü,k5 Ƃ,k0);slots_={k0,Ȑ};lanes_.emplace(t_,slots_,Ĭ(LaneTree::ƃ(Ȑ,Ȑ)),Ύ::Keeper{&ҍ});ҍ.reset(Ĭ(kept_words<LaneLayout>(*lanes_,0,Ȑ)),Ĭ(Ȑ))ĩ LaneTree::Root h6=lanes_->ˀ()ȡ ď=(Ȑ+8)/8*8;Ǜ İ ф(8 Ԏ)à;Ġ ä=Ö(Ȑ+1);Ġ ȴ ɚ={ä,ä,ä,ä,ä,ä,ä,ä};Ǜ į ɚ={İ,İ+ď,İ ʳ ď,İ+3 Ԏ,İ+4 Ԏ,İ+5 Ԏ,İ+6 Ԏ,İ+7 Ԏ};lane_columns(h6.ļ à,h6.î+1,ȴ,į);
lane_products_={İ,ď,Ö(Ȑ)};top_.emplace(t_,lane_products_,Ĭ(TopTree::ƃ(8,m8_)),Ύ::Keeper{&ѡ});ѡ.reset(Ĭ(kept_words<Ǘ>(*top_,0,8)),Ĭ(8));α=top_->ˀ().ļ ȑ(m8_+1);}ʨ(Ù ʨ&)=delete;ʨ&ŕ=(Ù ʨ&)=delete;Õ size()Ù{Ú m8_;}Ğ Ŏ()Ù{Ú α;}Ğ product_transform()Ù{Ú ѡ.ˀ();}Ĉ descend(Ğ w,ĝ Ƴ,Ö c,Ѕ){ƞ;Stack ǚ(ĸ)ȡ h5=ǎ î(Ö(Ȑ))ĩ LaneState ˀ=lane_state(w,c,ǚ);BaseDescent Ƽ(slots_,Ƴ à)ĺ(Ȑ<=ǎ kBase)Ú Ƽ.run(0,Ȑ,ˀ.state Ʀ(h5-Ȑ));descend_lanes(0,Ȑ,ˀ.state,ˀ.u,ǚ,Ƽ);}protected:Ę LaneTree=Ώ<LaneLayout,â Ѣ,â Ύ::Keeper>ͼ TopTree=Ώ<Ǘ,â LaneProducts,â Ύ::Keeper>;ċ ý ɶ(Õ m8){ģ т=8*ǎ î(Ö(m8/8)),top=Ϣ Ū Ī(Ŋ(т,top));}Ĉ halve(Ņ p,Õ č)Ù{ѻ h=č/2;t_.inverse_upper({p+h,h},{p+h,h},Ͻ 1);t_.ı({p+h,h},0,{p+h,h})ø(Ů h Ɲ)Š(p+i,add_mod(Ō(p+i),Ō(p+h+i)));}Ĉ ļ(Ņ p,Õ č,Ö u)Ù{ƞ ĺ(č==8)Ú t_.ć({p,8},{p,8},u)ȡ h=č/2;Ġ c=Ɵ(u);t_.ć({p,h},{p,h},c);t_.inverse_upper({p+h,h},{p+h,h},Ͻ c)ø(Ů h Ɲ){Ģ a ӷ(p+i),b ӷ(p+h+i);Š(p+i,subtract_mod(a,b)),Š(p+h+i,add_mod(a,b));}}ƛ LaneState{Ņ state;Ö u;};LaneState lane_state(Ğ w,Ö c,Ѿ){ѻ h5=ǎ î(Ö(Ȑ)),ď=ʘ(8,h5)ĩ ƍ ƀ
=Ȑ>ǎ kBase;Ǜ т=Л(8 Ԏ)ȡ в Ǜ İ=Л(8 Ԏ)ĺ(!ƀ)ʇ(İ,8 Ԏ,0)ȡ î=Ǘ::î(Ö(m8_));Ǜ ˀ=Л(î);t_.ı(w,î-m8_,{ˀ,î});Ö u=1;descend_top(0,8,ˀ,c,ǚ,İ,h5,ď,ƀ?&u:Ƿ)ĺ(ƀ){t_.standard_to_lanes(İ,ď,h5,т);}ŝ{Ć(ǉ ď Ӗ){ĭ r ҹ(Õ l Ē l<8 ė l)r[l]ӷ(İ+l Ԏ+j);transpose8(r)ø(Ů 8 Į)Š(т Ʀ(j+i),r Ĕ);}}ǚ ȍ Ū{т,u};}Ĉ descend_top(Ї,п,Ɠ s,Ö u,Ѿ,Ņ İ,Õ h5,Õ ď,Ņ i1){Ù TopTree&Ǌ=*top_ ȡ î=Ǘ::î(Ǌ.Ѝ),Χ ȶ{ģ xlo=n1[side]Ŭ,xhi=n1[side]ƹ,ʎ=Ǘ::î(Ǌ.ũ(xlo,xhi))ȡ в Ǜ p=Л(î);t_.leaf_products(s,ѡ.pair(ς)+(1-side)ӎ(î),{p,î})ĺ(xhi-xlo у&&i1){halve(p,î)Ԗ(p,h5,İ+xlo Ԏ);*i1=Ɵ(u);}м(xhi-xlo у){ļ(p,î,u)Ԗ(p+î-h5,h5,İ+xlo Ԏ);}м(2*ʎ==î){halve(p,î);descend_top(xlo,xhi,p,Ɵ(u),ǚ,İ,h5,ď,i1);}ŝ{ļ(p,î,u);Ǜ q=Л(ʎ);t_.ı({p+î-ʎ,ʎ},0,{q,ʎ});descend_top(xlo,xhi,q,1,ǚ,İ,h5,ď,i1);}ǚ ȍ;}}Ĉ descend_lanes(Ї,п,Ɠ s,Ö u,Ѿ,â BaseDescent&Ƽ){Ù LaneTree&Ǌ=*lanes_ ȡ č=8*ǎ î(Ö(Ǌ.Ѝ)),Χ ȶ{ģ xlo=n1[side]Ŭ,xhi=n1[side]ƹ,ũ=Ǌ.ũ(xlo,xhi)ȡ ʎ=8*ǎ î(Ö(ũ)),в Ǜ p=Л(č);ȣ::ѣ(s,ҍ.pair(ς)+(1-side)ӎ(č),{p,č})ĺ(xhi-xlo у||ũ<=ǎ kBase){ļ(p,č,u);Ƽ.run(xlo,xhi,p+č-8*ũ);}м(2*ʎ==č){halve(p,č);descend_lanes
(xlo,xhi,p,Ɵ(u),ǚ,Ƽ);}ŝ{х descend_lanes(xlo,xhi,q,1,ǚ,Ƽ);}ǚ ȍ;}}ċ Ö Ɵ(Ö u){Ú ʀ(u,(â kP+1)/2);}Õ m8_,Ȑ;ȣ t_;â Ѣ slots_{};â LaneProducts lane_products_{};â Ύ ҍ,ѡ;Ø optional<LaneTree>lanes_;Ø optional<TopTree>top_;Ğ α;};Ϧ{ò ɿ kDirectEvaluation=ǻ 22;í ԇ(О,Ğ k5,ĝ Ƴ){ɿ ɡ=4 ƣ to_r(kR)ø(Ů Љ Ɲ*ɡ){љ x[8*ɡ]={},v[8*ɡ]ȡ here Ш<Õ>(8*ɡ,Љ-i)Ԗ(k5 ʊ(i),here,x);ĭ xm[ɡ],acc[ɡ]ø(Ȫ ɡ ė c)xm[c]=ń(ŭ(Ō(x Ʀ c),to_r),kP),acc[c]=đ ø(ʍ=Ǳ;j-->0;){Ģ fj=ĕ(f Ɓ)ø(Ȫ ɡ ė c)acc[c]=ń(ƪ(Ŕ(acc[c],xm[c]),fj),2*kP);}Ć(Ȫ ɡ ė c)Š(v Ʀ c,ń(acc[c],kP))Ԗ(v,here,Ƴ ʊ(i));}}ƛ Division{Õ big,k,î;Division(ǀ,Õ m8):big(Ŋ(n,m8)),k((big+1)/2),î(ʘ(64,ɬ(Ŋ(Ӵ-1,big)))){}ý log()Ù{Ú Ŋ(inverse_log(k),Ī(î));}Õ ƃ()Ù{Ú ß(big)Я(k)Я(ȵ(k))+3 ӎ(î);}};í divide(ƶ,Ù Division&s,О,Ğ q,Ğ qt,ĝ d,Ѕ){ģ big=s.big,k=s.k,ɼ=big-k,n=s.î;Stack ǚ(ĸ)ǫ g{Л(big),big},h{Л(k),k};ʇ(g ü,big-Ǳ,0)ӏ(f ü,f Ƃ,g ʊ(big-Ǳ));ć(t,q,h,{Л(ȵ(k)),ȵ(k)})ǫ a{Л(n),n},b{Л(n),n},th{Л(n),n}Ǎ lower=[n](Õ ä){Ú ä<=n/2?ƨ:Ȉ;}Ȇ(g ȑ(k),0,a)Ȇ(h,0,th);Ɖ(a,th,a,lower(k))Ԗ(a ü,k,d ü)Ȇ(d ȑ(k),0,a)ĺ(qt Û!=n){ģ
 Ơ Ш(Ѹ,n)Ԗ(q ü,Ơ,b ü)ĺ(Ѹ>n)b Г Ö((ŉ(b Ŭ)+q[n])Ұ)Ȇ(b ȑ(Ơ),0,b);qt=b;}Ɖ(qt,a,a,k>=n/2?Ƨ:Ȉ)ø(Ů ɼ Į)b Ƈ g[k+i]>=a[k+i]?g[k+i]-a[k+i]:g[k+i]+Ͻ a[k+i];t.Ŷ(b ȑ(ɼ),0,a,th,lower(ɼ))Ԗ(a ü,ɼ,d ʊ(k));}}Ϧ{Ί evaluate_tree_words(ǀ,Ό){б ĩ Division ȴ(n,m8)Ū ʨ Ʈ(m)+Č Ʈ(ȴ.log())Я(ȴ.big)Я(Ŋ(ȴ.ƃ(),ʨ::descend_words(m)))Я(m8);}í evaluate_tree(ě&ŋ,О,Ğ k5,ĝ Ƴ){ģ m=Љ;ʨ Ǌ(ŋ,k5)ȡ m8=Ǌ Û ĩ Division ȴ(Ǳ,m8)ĩ Č t(ŋ,ȴ.log())ǫ d ф(ȴ.big)ǫ ĸ ф(Ŋ(ȴ.ƃ(),ʨ::descend_words(m)));divide(t,ȴ,f,Ǌ.Ŏ(),Ǌ.product_transform(),d,ĸ)ĩ ƍ ӥ=m8==m&&Λ(Ƴ à)%32 ζ ǫ all=ӥ?Ƴ:Ĭ(m8);Ǌ.descend(d Ė(ȴ.big-m8,m8),all,kR,ĸ)ĺ(!ӥ)ĵ(all ü,m,Ƴ ü);}}Ί evaluate_words(ǀ,Ό){Ú n*m<=â kDirectEvaluation?0:â evaluate_tree_words(n,m);}í evaluate(ě&ŋ,О,Ğ k5,ĝ Ƴ){if(Ǳ*Љ<=â kDirectEvaluation)Ú â ԇ(f,k5,Ƴ);â evaluate_tree(ŋ,f,k5,Ƴ);}}ā ˎ{Ϧ{Ȅ ѷ(ĭ s){Ú ð(Þ(ñ(s,32),ĕ(kR)),Ĵ(s,đ,0xAA));}Ą<Õ K>ĉ Ȅ cross_sum(Ģ(&Ŧ)[K],Ģ(&Ȏ)[K],Õ Ơ){Ĺ(K у){Ú ˍ(Ŧ Ŭ,Ȏ Ŭ,Ơ);}ŝ{if(Ơ<=17)Ú ˍ(ð(Ŧ Ŭ,Ŧ ƹ),ð(Ȏ Ŭ,Ȏ ƹ),Ơ)Ū ˍ(ð(ѷ(Ŧ Ŭ),ѷ(Ŧ ƹ)),ð(ѷ(Ȏ Ŭ),ѷ(Ȏ ƹ)),0);}}Ɩ í Ҍ(һ,ǀ,Ҡ,Ȕ,Ɠ c,Ɠ d,ȧ){Ć(Ӆ<n+k ė t){
н ǥ={},Ȏ ǥ={}ȡ ù ǥ={t>k?t-k:0,t>n?t-n:0},Ȣ ǥ={Ħ(t,ԕ),Ħ(t,k-1)}ø(ɵ=ù Ŭ;i<=Ȣ Ŭ Į){Ҽ Ō(ύ i),y ӷ(d Ʀ(t-i));Ŧ Г ð(Ŧ Ŭ,Þ(x,y));Ȏ Г ð(Ȏ Ŭ,ɹ);}Ć(ɵ=ù ƹ;i<=Ȣ ƹ Į){Ҽ Ō(b Ʀ i),y ӷ(c Ʀ(t-i));Ŧ ƹ=ð(Ŧ ƹ,Þ(x,y));Ȏ ƹ=ð(Ȏ ƹ,ɹ);}Š(į ϝ,cross_sum(Ŧ,Ȏ,(Ȣ Ŭ+1-ù Ŭ)+(Ȣ ƹ+1-ù ƹ)));}}Ą<Õ D,Õ B>ˮ Ҍ(һ,Ҡ,Ɠ c,Ɠ d,ȧ){ɿ kPad=B-1,kSums=2*D>17?2:1;ɍ[D ʳ kPad],xo[D ʳ kPad],y[D ʳ kPad],yo[D ʳ kPad],co[D+1],dq[D+1]ø(Ů kPad Į)x Ƈ xo Ƈ y Ƈ yo Ƈ x[D+kPad+i]=xo[D+kPad+i]=y[D+kPad+i]=yo[D+kPad+i]=đ;
#pragma GCC unroll 16
Ć(Ů D Į){x[kPad+i]ӷ(ύ i),xo[kPad+i]=ñ(x[kPad+i],32);y[kPad+i]ӷ(b Ʀ i),yo[kPad+i]=ñ(y[kPad+i],32);}
#pragma GCC unroll 17
Ć(Ŀ<=D ų)co ˉ ñ(Ō(c Ʀ j),32),dq ˉ ñ(Ō(d Ʀ j),32)ø(Õ t0 Ē t0<2*D;t0+=B){н[B][kSums]={},Ȏ[B][kSums]={}ȡ ù=t0+1>D?t0+1-D:0,Ȣ Ш(D,t0+B-1)ø(ʍ=ù;j<=Ȣ ų){Ģ qd ӷ(d Ʀ j),qc ӷ(c Ʀ j);
#pragma GCC unroll 8
Ć(Õ s Ē s<B ė s){ģ i=kPad+t0+s-j;Ŧ[s]Г ð(Ŧ[s]Ŭ,Þ(x Ĕ,qd));Ȏ[s]Г ð(Ȏ[s]Ŭ,Þ(xo Ĕ,dq Ɓ));Ŧ[s][kSums-1]=ð(Ŧ[s][kSums-1],Þ(y Ĕ,qc));Ȏ[s][kSums-1]=ð(Ȏ[s][kSums-1],Þ(yo Ĕ,co Ɓ));}}
#pragma GCC unroll 8
Ć(Õ s Ē s<B ė s){ģ t=t0+s,Ơ=2*(Ħ(t,D-1)+1-(t>D?t-D:0))ĺ(t<2*D)Š(į ϝ,cross_sum(Ŧ[s],Ȏ[s],Ơ));}}}Ą<Õ D>ˮ Ҍ(һ,Ҡ,Ɠ c,Ɠ d,ȧ){Ǝ(2*D<=17);ɍ[D],xo[D],y[D],yo[D],p[D+1],po[D+1],q[D+1],qo[D+1];
#pragma GCC unroll 8
Ć(Ů D Į){x Ƈ Ō(ύ i),xo Ƈ ñ(x Ĕ,32);y Ƈ Ō(b Ʀ i),yo Ƈ ñ(y Ĕ,32);}
#pragma GCC unroll 9
Ć(Ŀ<=D ų){p ˉ Ō(c Ʀ j),po ˉ ñ(p Ɓ,32);q ˉ Ō(d Ʀ j),qo ˉ ñ(q Ɓ,32);}
#pragma GCC unroll 16
Ć(Ӆ<2*D ė t){ģ ù=t>D?t-D:0,Ȣ Ш(t,D-1);н=đ,Ȏ=đ;
#pragma GCC unroll 8
Ć(ɵ=ù;i<=Ȣ Į){Ŧ=ð(Ŧ,ð(Þ(x Ĕ,q[t-i]),Þ(y Ĕ,p[t-i])));Ȏ=ð(Ȏ,ð(Þ(xo Ĕ,qo[t-i]),Þ(yo Ĕ,po[t-i])));}Š(į ϝ,ˍ(Ŧ,Ȏ,2*(Ȣ+1-ù)));}}Ą<Õ D>Ĉ cross_level(Ɠ Ͽ,Ɠ ƕ,ȧ,Õ j3){Ć(Ů j3 Į){Ɠ r=Ͽ Ʀ D*(2*i),*q=ƕ Ʀ(D+1)*(2*i)Ѓ(D<=8)Ҍ<D>(r,r Ʀ D,q,q Ʀ(D+1),ѓ 16*D*i)і Ҍ<D,2>(r,r Ʀ D,q,q Ʀ(D+1),ѓ 16*D*i);}}Ȅ inverse_montgomery(ɍ){ĭ ă=ĕ(kR),Ǹ=x ø(Ö e=ӕ;e;e>>=1){if(e&1)ă=Ŕ(ă,Ǹ);Ǹ=Ŕ(Ǹ,Ǹ);}Ѡ}Ũ ҋ:private BaseTree{Ⱥ ҋ(Ù Ѣ&n2,Ɠ y,Ό):BaseTree(n2,ϴ),y_(y),m_(m){}Ɠ run(Ї,п,Ɠ w){ģ þ=hi-lo;lo_=lo ĺ(þ у){Š(ϭ,Ō(w));ş(1)Ū ϭ;}if(ϩ(þ))Ú run_levels(þ,w);ӽ ş(þ);ascend(1,lo,hi,sums_)Ū sums_;}Ⱦ Ɠ run_levels(Ƞ,Ɠ w){Ɠ ƕ[6];Ņ at=Ӡ(lo_,þ,ƕ);Ǜ buffers ǥ={at,at Ʀ þ};descend_levels(lo_,þ,w,ƕ,buffers);ş(þ);Ɠ Ͽ=ϭ ø(Õ d=1,j Ē d<þ;d*=2,++j){Ǜ į=2*d==þ?sums_:buffers[j&1];with_degree(d,Ѯ<Õ D>(){cross_level<D>(Ͽ,ƕ Ɓ,į,þ/(2*D));});Ͽ=į;}Ú sums_;}Ĉ ascend(Õ v,Ї,п,ȧ){if(hi-lo у)Ú Š(į,Ō(ϭ Ʀ(lo-lo_)))ͷ ascend(2*v,lo,ς,ϊ);ascend(2*v+1,ς,hi,l8);Ҍ(ϊ,ς-lo,l8,hi-ς,α[2*v],α[2*v+1],į);free_=ϊ;}Ĉ ş(Ƞ){ɿ ɡ=4 κ iota=ɜ,one=ĕ(kR)ȡ f5=(þ+ɡ-1)/ɡ;ĭ ʜ[kMaxSlots]ø(Õ g Ē
 g<f5 ė g){
#pragma GCC unroll 4
Ć(Ȫ ɡ ė c){ģ k=ɡ*g+c,ù=8*(lo_+k);ɍ=one ĺ(k<þ){x ӷ(ϭ Ʀ k)ĺ(Ў 8>m_){x=_mm256_blendv_epi8(one,x,τ(ĕ(Ö(m_-ù)),iota));Š(ϭ Ʀ k,x);}}ʜ ђ g?Ŕ(ʜ[k-ɡ],x):x;}}Ģ*Ù Ȣ=ʜ+ɡ*(f5-1)κ p01=Ŕ(Ȣ Ŭ,Ȣ ƹ),p23=Ŕ(Ȣ ǥ,Ȣ Ѐ)κ ć=inverse_montgomery(Ŕ(p01,p23))κ i01=Ŕ(ć,p23),i23=Ŕ(ć,p01);ĭ e[ɡ]={Ŕ(i01,Ȣ ƹ),Ŕ(i01,Ȣ Ŭ),Ŕ(i23,Ȣ Ѐ),Ŕ(i23,Ȣ ǥ)}ø(Õ g=f5;g-->0;){
#pragma GCC unroll 4
Ć(Ȫ ɡ ė c){ģ k=ɡ*g+c ĺ(k>=þ)ӊ κ x ӷ(ϭ Ʀ k),l4=g?Ŕ(e[c],ʜ[k-ɡ]):e[c];e[c]=Ŕ(e[c],x);Š(ϭ Ʀ k,ů(Ŕ(l4,Ō(y_ Ʀ(lo_+k)))));}}}ċ ɿ kMaxSlots=ǎ kBase;љ ϴ[8*kMaxSlots];љ sums_[8*kMaxSlots];Ɠ y_;Õ m_;};}Ũ Ϭ:public ʨ{Ⱥ Ϭ(ě&ŋ,Ğ k5):ʨ(ŋ,k5),m_(Љ){}ċ Õ ƃ(Ό){б,h5=ǎ î(Ö(m8/8))ȡ ď=ʘ(8,h5),o3=Ϣ Ū descend_words(m)+10 ӎ(8 Ԏ)+6 ӎ(o3)+4096;}Ĉ Ҋ(Ğ w,Ğ y,ĝ r,Ѕ){ƞ;Stack ǚ(ĸ)ȡ h5=ǎ î(Ö(Ȑ)),ď=ʘ(8,h5)ĩ LaneState ˀ=lane_state(w,kR,ǚ);ҋ Ƽ(slots_,y à,m_);Ǜ Ͽ=Л(8 Ԏ)ĺ(Ȑ<=ǎ kBase){ĵ(Ƽ.run(0,Ȑ,ˀ.state Ʀ(h5-Ȑ)),8*Ȑ,Ͽ);}ŝ{т(0,Ȑ,ˀ.state,ˀ.u,ǚ,Ƽ,Ͽ,h5);t_.ć({Ͽ,8*h5},{Ͽ,8*h5});}Ǜ İ=ˀ.state ø(ǉ Ȑ Ӗ){ĭ v ҹ(Ů 8 Į)v Ƈ Ō(Ͽ Ʀ(j+i));transpose8(v)ø(Õ l Ē l<8 ė l)Š(İ+l Ԏ+j,v[l]);}ģ î=Ǘ::î(Ö(m8_));top(0,8,İ,ď,ǚ,r à,î);t_.ć(r ȑ(î),r ȑ(î));}Ⱦ Ĉ т(Ї,п,Ɠ s,Ö u,Ѿ,â ҋ&Ƽ,ȧ,Õ ɖ){ƞ ĩ LaneTree&Ǌ=*lanes_ ȡ î=ǎ î(Ö(Ǌ.Ѝ)),č=8*î ȡ Χ,в Ɠ Ù n7 ǥ={ҍ.pair(ς),ҍ.pair(ς)Я(č)};Ǜ Ͽ ǥ={ɖ>=2*î?ѓ č:į,Л(č)}ȶ{ģ xlo=n1[side]Ŭ,xhi=n1[side]ƹ,ũ=Ǌ.ũ(xlo,xhi)ȡ ʎ=8*ǎ î(Ö(ũ)),g5=ǚ.mark();Ǜ p=Л(č);ȣ::ѣ(s,n7[1-side],{p,č})ĺ(xhi-xlo у||ũ<=ǎ kBase){ļ(p,č,u);Ɠ Ù ʂ=Ƽ.run(xlo,xhi
,p+č-8*ũ);t_.ı({ʂ,8*ũ},0,{Ͽ[side],č});}м(2*ʎ==č){halve(p,č);т(xlo,xhi,p,Ɵ(u),ǚ,Ƽ,Ͽ[side],î);}ŝ{х т(xlo,xhi,q,1,ǚ,Ƽ,Ͽ[side],î);}ǚ.release(g5);}ȣ::pointwise_product_sums(Ͽ Ŭ,n7 ƹ,Ͽ ƹ,n7 Ŭ,{į,č});double_to(į,č,ɖ/î,ǚ);ǚ ȍ;}Ĉ top(Ї,п,Ɠ İ,Õ ď,Ѿ,ȧ,Õ ɖ){Ù TopTree&Ǌ=*top_ ĺ(hi-lo у)Ú t_.ı({İ+lo Ԏ,Ȑ},0,{į,ɖ})ȡ î=Ǘ::î(Ǌ.Ѝ),Χ,в Ɠ Ù n7 ǥ={ѡ.pair(ς),ѡ.pair(ς)Я(î)};Ǜ Ͽ ǥ={ɖ>=2*î?ѓ î:į,Л(î)};top(lo,ς,İ,ď,ǚ,Ͽ Ŭ,î);top(ς,hi,İ,ď,ǚ,Ͽ ƹ,î);t_.leaf_product_sums(Ͽ Ŭ,n7 ƹ,Ͽ ƹ,n7 Ŭ,{į,î});double_to(į,î,ɖ/î,ǚ);ǚ ȍ;}Ĉ double_to(ȧ,Õ č,Õ Ÿ,Ѿ)Ù{if(Ÿ у)Ú;Ǜ c=Л(č);t_.ć({į,č},{c,č})ø(ǀ=č;n<Ÿ*č;n*=2)t_.ɇ({c,č},0,{ѓ n,n});}Õ m_;};Ί interpolate_words(Ό){б ĩ â Division ȴ(m,m8)Ū ʨ Ʈ(m)+Č Ʈ(ȴ.log())Я(Ŋ(m+1,m8))Я(Ϣ)Я(Ŋ(ȴ.ƃ(),Ϭ::ƃ(m)));}í Ҋ(ě&ŋ,Ğ k5,Ğ Ƴ,ĝ c){ģ m=Љ;Ϭ Ǌ(ŋ,k5)ȡ m8=Ǌ Û,î=Ϣ ǫ y ф(Ŋ(m+1,m8))ӏ(Ǌ.Ŏ()ü,Ǌ.Ŏ()ʊ(m+1),y ü);Ǟ(y ȑ(m+1),y ʴ)ĩ â Division ȴ(m,m8)ĩ Č t(ŋ,ȴ.log())ǫ Ļ ф(î)ǫ ĸ ф(Ŋ(ȴ.ƃ(),Ϭ::ƃ(m)));â divide(t,ȴ,y ʴ,Ǌ.Ŏ(),Ǌ.product_transform(),Ļ ȑ(m8),ĸ);ȩ(Ƴ ü,Ƴ Ƃ,y ü);y[m]Ē Ǌ.Ҋ(Ļ ȑ(m8),y ȑ(m8),Ļ,ĸ)ӏ(Ļ ü,Ļ ʊ(m),c ü);
}}ā ˎ{í import_words(ˢ in,ɐ){ƞ;ɵ ƒ(ʲ in Û Ɲ){Ҽ Ž(õ Ɠ>(in Ȝ));Š(į Ȝ,ů(Ŕ(x,ĕ(1))));}Ć(Ͼ in Û Į)į Ƈ in Ĕ ȃ;}í import_montgomery(ˢ in,ɐ){Ć(Ů in Û Į){Ù ī x=Ӫ;į Ƈ Ħ(x,x-Ƀ);}}ò Ʋ export_words(Ğ in){Ʋ į;ž(į,in à,in Û)Ū į;}ò Ʋ native_evaluate(ˢ f,ˢ k5){ě ŋ(evaluate_words(Ǳ,Љ)Я(Ǳ)ʳ ß(Љ))Ѵ a ф(Ǳ),x ф(Љ),y ф(Љ);import_montgomery(f,a);import_words(k5,x);evaluate(ŋ,a,x,y)Ū export_words(y);}ò Ʋ native_interpolate(ˢ k5,ˢ Ƴ){ģ n=Љ й(interpolate_words(n)+3 ӎ(n))Ѵ x к,y к,c к;import_words(k5,x);import_montgomery(Ƴ,y);Ҋ(ŋ,x,y,c)Ū export_words(c);}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ Ҋ(Ù ѝ&xs,Ù ѝ&ys){Ѽ ͼ Ƽ=ѝ;Ƙ ӵ(Ҋ<Ě>(Һ(xs),Һ(ys)));ơ(Ԉ==ys Û)ĩ ǧ=ó(Ԉ)ĺ(n ζ)Ú{};
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(n>=64&&n ɰ ȏ)Ú F(ç ˎ::native_interpolate(xs,ys));}
#endif
æ ý ŀ=8 Ю ȉ î=ä*ŀ;F ˀ(î)ø(Е)ˀ[î-n+i]=T(n-i)*Ǌ ƹ Ĕ Ѵ flip=Ѯ{Ć(λ Ё Į){Ø ˁ(Ǌ Ĕ ü,Ǌ Ĕ Ƃ);Ƽ&a=Ɣ Ĕ ĺ(a ű)ӊ ȉ î=ó(Ư),d=ó(Ǌ Ĕ Û)-1 ø(ý j=2;j<î;j<<=1)Ø ˁ(a ʷ j,a ü ʳ j)ĺ(d==î/2){Ć(ý j=î/2;j<î ų)a ˉ-a Ɓ;}ŝ{ʦ&ķ=ǆ ntt_roots(î);Ƽ phase(î,Ӹ)ø(ý Ɵ=1;Ɵ<î;Ɵ<<=1){Ù T ɯ=ķ[Ɵ].pow(d)ø(ȓ Ɵ ų)phase[Ɵ+j]=phase Ɓ*ɯ;}Ć(ȓ î ų)a Ɓ*=phase Ɓ;}}};ˀ=ˀ.Α(Ǌ ƹ,î);Ƽ den=ç Ώ<T>::evaluate(xs,Ǌ,Ɣ,ä,ŀ,ƅ(ˀ));flip();ħ F>ĳ(Ё);Ƽ ş(n);ǆ batch_inverse(ş,den);ǆ ȇ(ş,ş,ys,n)ø(ý i Ē i*ŀ<n Į){ȟ d Ш(ŀ,n-i*ŀ);F&f=ĳ[ä+i];f Ř(d)ĩ F&q=Ǌ[ä+i];
#if defined(__x86_64__) || defined(__i386__)
ġ{if(d==8 ȏ){Ű interpolate_leaf8(Ȯ(f),Ȯ(q),Ȯ(xs)+i*ŀ,Ȯ(ş)+i*ŀ);ӊ;}}
#endif
Ć(ý j=i*ŀ;j<i*ŀ+d ų){T ʑ=1;f[d-1]+=ş Ɓ ø(ң=d-2;k и k){ʑ=q[k+1]+xs Ɓ*ʑ;f ƫ+=ş Ɓ*ʑ;}}}Ć(Ż ä-1;i>=1;--i){if(ĳ ư ű){ĳ Ƈ ƅ(ĳ ɔ);ӊ;}ȟ î=ó(Ɣ ɔ Û)ĺ(î&&ó(Ɣ ư Û)==î){Ƽ a=ƅ(ĳ ɔ),b=ƅ(ĳ ư);a Ř(î);b Ř(î);Ә(a);Ә(b);ġ Ű җ(Ȯ(a),Ȯ(b),Ȯ(Ɣ ɔ),Ȯ(Ɣ ư),î)і Ć(ȓ î ų)a ˉ a Ɓ*Ɣ ư Ɓ+b Ɓ*Ɣ ɔ Ɓ;ǆ intt(a);a Ř(Ǌ Ĕ Û-1);ĳ Ƈ ƅ(a);}ŝ{ĳ Ƈ ĳ ɔ*Ǌ ư+ĳ ư*Ǌ ɔ;}}Ú ƅ(ĳ ƹ);}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ T linear_recurrence(Ù ѝ&g4,Ù ѝ&ļ,ǈ k){Ѽ;Ƙ;ơ(k>=0&&g4 Û==ļ Û)ȉ d=ó(g4 Û)ĺ(k<d)Ú g4 ƫ;ý с=d Ѵ zero=[](T x){ġ Ú x Ү і Ú x ȃ ζ;}ǟ(с&&zero(ļ[с-1]))--с ĺ(с ζ)Ú 0;ƴ{ӈ ǆ W a(с),c(с)ø(ö с Į){ӫ Ě ι(g4[d-с+i]ȃ);c Ƈ Ě ι(ļ Ĕ ȃ);}Ú T ι(linear_recurrence<Ě>(a,c,k-d+с)ȃ);}if(с у)Ú g4[d-1]*ļ Ŭ.pow(k-d+1);F q(с+1);q Г 1 ø(ö с Į)q Υ=-ļ Ĕ;F p=F::Ź(F(g4 Ƃ-с,g4 Ƃ),q).pre(с)Ū bostan_mori(ƅ(p),ƅ(q),k-d+с);}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
ӂ{
#if defined(__x86_64__) || defined(__i386__)
Ü í ԇ(ҿ ː,Ù ҿ f,ǀ,Ù ҿ k5,Ό){Ę I=ę I8 ø(Ů m ƽ 32){Ť ę U Η[32]={},į[32]ȡ þ Ш<Õ>(32,m-i)ĺ(þ<8){Ć(ѿ þ ȝ){Ě ĳ=0,x=Ҿ(k5[i+k])ø(ʍ=n;j-->0;)ĳ=ĳ*x+Ҿ(f Ɓ);ː[i+k]=ĳ.v;}Ϸ}Ù ҿ xs=k5+i ĺ(þ<32){ĵ(xs,þ,Η);xs=Η;}I x0=ę œ(xs),x1=ę œ(xs+8);I x2=ę œ(xs ү),x3=ę œ(xs+24);I a0=đ,a1=a0,a2=a0,a3=a0 ø(ʍ=n;j-->0;){I c ʖ(f Ɓ);a0=ę ɾ(ę ɏ(a0,x0),c);a1=ę ɾ(ę ɏ(a1,x1),c);a2=ę ɾ(ę ɏ(a2,x2),c);a3=ę ɾ(ę ɏ(a3,x3),c);}ҿ ǂ=þ==32?ҷ:į;ę ŗ(ǂ,a0);ę ŗ(ǂ+8,a1);ę ŗ(ǂ ү,a2);ę ŗ(ǂ+24,a3)ĺ(þ<32)ĵ(į,þ,ҷ);}}
#endif
}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ ѝ multipoint_eval(Ù Ȼ&f,Ù ѝ&xs){Ѽ ͼ W=Ď<Ě>ͼ Ƽ=ѝ;Ƙ ӵ(W(multipoint_eval<Ě>(Һ(f),Һ(xs))))ĺ(xs ű)Ú{}ĺ(Ǳ<=32||Ԉ<=32){
#if defined(__x86_64__) || defined(__i386__)
ġ{if(((Ԉ>=8&&Ǳ>=8&&(Ԉ>=32||Ǳ>=32))||(Ԉ>=4096&&Ǳ>=4))ȏ){Ƽ r(Ԉ);ç ԇ(Ȯ(r),Ȯ(f),Ǳ,Ȯ(xs),Ԉ)Έ}}
#endif
Ƽ r;r.reserve(Ԉ)ø(T x:xs)r ӱ(f.eval(x))Έ}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(Ǳ*Ԉ>(ǻ 22)&&Ŋ(Ǳ,Ԉ)ɰ ȏ)Ú ç ˎ::native_evaluate(f,xs);}
#endif
æ ý ŀ=8;ί=ŀ ǟ(î<(ý)Ŋ(Ǳ,Ԉ))î<<=1;ơ(î ɰ)Ю;F ˀ=f;ˀ Ř(î);Ø ˁ(ˀ ü,ˀ Ƃ);ˀ=ˀ.Α(Ǌ ƹ,î)ĺ(ä*ŀ<î)ˀ.erase(ˀ ü,ˀ Ƃ-ä*ŀ)Ū ç Ώ<T>::evaluate(xs,Ǌ,Ɣ,ä,ŀ,ƅ(ˀ),Ɔ);}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ӂ{ƛ Ȭ{Ę T=Ě ͼ F=Ȼ ͼ O=ǅ;ċ æ ý Ӟ=64;ƛ ǌ{ǖ F,4>a{F{1},F{},F{},F{1}};mutable ǖ F,4>Ǥ;ý ƌ Ē ƍ ȭ=Ɔ;};ċ F high(ʧ a,ң){Ú k>=џ?F{}:F(a ʷ k,a Ƃ);}ċ F ѷ(ʧ a,ǧ){if(џ<=n){F ă=a;ă Ř(n)Ȧ}F ă(n)ø(ö џ Į)ă[i&(ԕ)]+=ɨ Ȧ}ċ ʧ Ǩ(Ù ǌ&š,ý ȯ,ǧ){F&Ǥ=š.Ǥ[ȯ]ĺ(Ǥ ű){Ǥ=ѷ(Ή[ȯ],n);Ә(Ǥ);}while(ý(Ǥ Û)<n){ȟ Ɵ=Ǥ Û;F Ȏ(Ɵ)ĩ F&p=Ή[ȯ]ø(ö ý(p Û)Į)Ȏ[i&(Ɵ-1)]+=(i&Ɵ)?-p Ĕ:p Ĕ;ǆ ɘ(Ȏ,Ӹ,ǆ ntt_roots(2*Ɵ)[Ɵ]);Ә(Ȏ);Ǥ ӯ(Ǥ Ƃ,Ȏ ü,Ȏ Ƃ);}Ú Ǥ;}ċ F ҁ(ʧ a,ʧ b,ʧ c,ʧ d,ǧ,F*Ǥ=Ƿ){F ă(a ü,a ʷ n);ę җ(Ȯ(ă),Ȯ(c),Ȯ(d),Ȯ(b),n)ĺ(Ǥ)*Ǥ=ă;ǆ intt(ă),ɞ(ă)Ȧ}ċ ǌ multiply_column(Ù ǌ&a,Ù ǌ&b){Қ,ă.ȭ=Ś ĺ(ă.ƌ<=64){Ć(ъ){ă.a ɔ=a.a ɔ*b.a Ŭ+a.a ư*b.a ǥ;ɞ(ă.a ɔ);}Ѡ}ǧ=1 ǟ(n<=ă.ƌ)n<<=1;ǖ Ù F*,4>x{},y{}ɫ{x Ƈ&Ǩ(a,i,n)ĺ(!(i&1))y Ƈ&Ǩ(b,i,n);}Ć(ъ)ă.a ɔ=ҁ(*x ɔ,*y Ŭ,*x ư,*y ǥ,n,&ă.Ǥ ɔ)Ȧ}Ŵ step_column(ǌ&š,ʧ q){Ć(ł&Ǥ:š.Ǥ)Ǥ.clear();F next=Ή Ŭ-q*Ή ǥ;ɞ(next);Ή Г ƅ(Ή ǥ),Ή ǥ=ƅ(next);š.ƌ+=Ŋ(0,ý(Ѹ)-1),š.ȭ=Ś;}ċ ǌ Ź(Ù ǌ&a,Ù ǌ&b,ƍ e3=Ś){Қ,ă.ȭ=Ś ĺ(e3)ă.a Ѐ.clear()ȉ χ=e3?1:2 ĺ(ă.ƌ<=64){Ć(ö χ Į)Ć(Ϡ){ă.a Й=a.a ɔ*b.a Ɓ+a.a ư*b.a[2+j];ɞ(ă.a Й);}Ѡ}ǧ=1 ǟ(n<=ă.ƌ)n<<=1;ǖ Ù F*,4>x{},y{}ɫ{if(i<2*χ)x Ƈ&Ǩ
(a,i,n);y Ƈ&Ǩ(b,i,n);}Ć(ö χ Į)Ć(Ϡ)ă.a Й=ҁ(*x ɔ,*y Ɓ,*x ư,*y[2+j],n,&ă.Ǥ Й)Ȧ}Ŵ ɯ(ǌ&š,ʧ q){Ć(ł&Ǥ:š.Ǥ)Ǥ.clear();F c=Ή Ŭ-q*Ή ǥ,d=Ή ƹ-q*Ή Ѐ;ɞ(c),ɞ(d);Ή Г ƅ(Ή ǥ),Ή ƹ=ƅ(Ή Ѐ);Ή ǥ=ƅ(c),Ή Ѐ=ƅ(d);š.ƌ+=Ŋ(0,ý(Ѹ)-1),š.ȭ=Ś;}Ŵ apply(Ù ǌ&š,F&a,F&b){if(š.ȭ)Ú ĺ(Ư<=128){F c=Ή Ŭ*a+Ή ƹ*b;b=Ή ǥ*a+Ή Ѐ*b,a=ƅ(c);ɞ(a),ɞ(b)а}ý ä=џ-š.ƌ,n=1;ơ(ä>0)ǟ(n<ä)n<<=1;F x=ѷ(a,n),y=ѷ(b,n);Ә(x),Ә(y);ǖ Ù F*,4>m ɫ m Ƈ&Ǩ(š,i,n);a=ҁ(*m Ŭ,x,*m ƹ,y,n);b=ҁ(*m ǥ,x,*m Ѐ,y,n);ơ(џ==ä&&η<Ư);}ċ F apply_first(Ù ǌ&š,ʧ a,ʧ b){if(š.ȭ)Ú a ȉ ä=џ-š.ƌ ĺ(ä у){T ĳ=(Ή Ŭ ű||a ű?Ȓ:Ή Ŭ Ŭ*a Ŭ)+(Ή ƹ ű||b ű?Ȓ:Ή ƹ Ŭ*b Ŭ)Ū ĳ Ү?F{}:F{ĳ};}if(Ư<=128){F ă=Ή Ŭ*a+Ή ƹ*b;ɞ(ă)Ȧ}ǧ=1 ǟ(n<ä)n<<=1;F x=ѷ(a,n),y=ѷ(b,n);Ә(x),Ә(y)Ū ҁ(Ǩ(š,0,n),x,Ǩ(š,1,n),y,n);}Ŵ җ(T*į,Ù T*a,Ù T*b,ǧ,T x,T y,T z){
#if defined(__x86_64__) || defined(__i386__)
if(n>=8&&ę Ĝ){combine8(į,a,b,n,x ȃ,y ȃ,z ȃ)а}
#endif
Ć(Е)į Ƈ x*ɨ+y*b[i-1]+z*Ϲ;}
#if defined(__x86_64__) || defined(__i386__)
ˌ combine8(T*į,Ù T*a,Ù T*b,ǧ,ī x,ī y,ī z){Ę I=ę I8;I vx ʖ(x),qx ʖ(ę ť(x));I vy ʖ(y),qy ʖ(ę ť(y));I vz ʖ(z),qz ʖ(ę ť(z));К{I u=ę ƥ(ę œ(õ Ù ī*>(a+i)),vx,qx);I v=ę ƥ(ę œ(õ Ù ī*>(b+i-1)),vy,qy);I w=ę ƥ(ę œ(õ Ù ī*>(b+i)),vz,qz);ę ŗ(õ ī*>(ѓ i),ę ɾ(u,ę ɾ(v,w)));}T tx=T ι(x),ty=T ι(y),tz=T ι(z)ø(ț)į Ƈ tx*ɨ+ty*b[i-1]+tz*Ϲ;}
#endif
ċ ǌ ӥ(ʧ a,ʧ b,ý Ʃ=-1){æ ý ε=2*Ӟ+8;ơ(Ư<=2*Ӟ+1);Ť T k6 Ѐ[ε]={},χ Ѐ ǥ[ε]={};T*c=k6 Ŭ+1,*d=k6 ƹ+1,*e=k6 ǥ+1;T*ξ Ѐ ǥ;ý ȴ Ѐ ǥ={{1,0},{0,1},{0,0}}ø(ö 3 Į)Ć(Ϡ)ξ Ĕ ˉ χ Ĕ Ɓ+1;ȩ(a ü,a Ƃ,c);ȩ(b ü,b Ƃ,d);ξ Ŭ Ŭ Г ξ ƹ ƹ Г 1 ĩ ǧ=Ư ĺ(Ʃ<0)Ʃ=n/2;ý cd=ԕ,dd=ý(η)-1;ý e4=2*Ʃ-(ԕ);T m5=1,m6=1 ǟ(dd>=Ʃ){ȟ d7=cd-dd;e4+=d7 ȉ ʗ=Ŋ(0,e4)ĩ T a9=d[dd];T m7=m5 ĺ(d7 у){Ù T x=a9*a9,y=-a9*c[cd]ĩ T z=c[cd]*d[dd-1]-a9*c[cd-1];җ(e+ʗ,c+ʗ,d+ʗ,dd-ʗ,x,y,z);m7*=x ø(Ϡ){ȟ î=Ŋ(ȴ Ŭ Ɓ,ȴ ƹ Ɓ?ȴ ƹ Ɓ+1:0);җ(ї,ξ Ŭ Ɓ,ξ ƹ Ɓ,î,x,y,z);ż(ї+î,ї+Ŋ(î,ȴ ǥ Ɓ),Ȓ);ȴ ǥ ˉ î;}}ŝ{ȩ(c+ʗ,c+cd+1,e+ʗ)ø(Ϡ){ĵ(ξ Ŭ Ɓ,ȴ Ŭ Ɓ,ї);ż(ї+ȴ Ŭ Ɓ,ї+Ŋ(ȴ Ŭ Ɓ,ȴ ǥ Ɓ),Ȓ);ȴ ǥ ˉ ȴ Ŭ Ɓ;}Ć(ý ƌ=d7;ƌ и ƌ){Ù T Ÿ=e[dd+ƌ]ø(Ż ʗ;i<=dd+ƌ Į)e Ƈ a9*e Ĕ-(i>=ƌ?Ÿ*d[i-ƌ]:Ȓ)ø(Ϡ){ȟ î=Ŋ(ȴ ǥ Ɓ,ȴ ƹ Ɓ?ȴ ƹ Ɓ+ƌ:0)ø(ö î Į)ї Ƈ a9*ї Ĕ-(i>=ƌ?Ÿ*ξ ƹ Ɓ[i-ƌ]:Ȓ);ȴ ǥ ˉ î;}m7*=a9;}}ь(c,d),ь(d,e)ø(Ϡ){ь(ξ Ŭ Ɓ,ξ ƹ Ɓ),ь(ξ ƹ Ɓ,ї);ь(ȴ Ŭ Ɓ,ȴ ƹ Ɓ),ь(ȴ ƹ Ɓ,ȴ ǥ Ɓ);}m5=m6,m6=m7;cd=dd,dd=cd-1 ǟ(dd>=Ʃ&&d[dd]Ү)--dd;}ǌ ă;ă.ƌ=ԕ-cd,ă.ȭ=ă.ƌ ζ;T ć=(m5*m6)ˆ;T e2 ǥ={ć*m6,ć*m5}ø(ъ)Ć(Ϡ){ă.a Й=F(ξ Ĕ Ɓ,ξ Ĕ Ɓ+ȴ Ĕ Ɓ);ǆ Ζ(ă.a Й,e2 Ĕ);ɞ(ă.a Й);
}Ѡ}ċ ǌ jump(F a,F b,ң,ƍ e3=Ś){Ù ǧ=џ-1,Ʃ=n-k;ơ(Ư>η&&0<=k&&k<=n)ĺ(ý(η)<=Ʃ)Ú ǌ{}ĺ(n>Ӵ)Ú jump(high(a,n-Ӵ),high(b,n-Ӵ),k,e3)ĺ(k<=Ӟ)Ú ӥ(a,b,Ʃ);ý ù=1 ǟ(2*ù<k)ù*=2 ȉ cut=Ŋ(0,n-2*ù);ǌ ϊ=jump(high(a,cut),high(b,cut),ù)ĺ(ϊ.ȭ){ł[q,r]=a.divmod(b);ɯ(ϊ,q),a=ƅ(b),b=ƅ(r);}ŝ apply(ϊ,a,b)ĺ(ý(η)<=Ʃ)Ú ϊ ȉ ψ=џ-1-Ʃ ȉ skip=Ŋ(0,џ-1-2*ψ)Ū Ź(jump(high(a,skip),high(b,skip),ψ,e3),ϊ,e3);}ċ ǌ Ɵ(F a,F b){ơ(Ư>η);ý m=џ/2;ǌ ă ĺ(ý(η)<=m)Ú ă ĺ(Ư<=2*Ӟ)Ú ӥ(a,b);ă=Ɵ(high(a,m),high(b,m));apply(ă,a,b)ĺ(ý(η)<=m)Ѡ ł[q,r]=a.divmod(b);ɯ(ă,q),a=ƅ(b),b=ƅ(r)ĺ(ý(η)<=m)Ѡ ң=2*m+1-џ Ū Ź(Ɵ(high(a,k),high(b,k)),ă);}};}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā ç ē{ā ӝ{ƞ ͼ ƺ::ѷ ͼ ƺ::ʫ ͼ ƺ::ґ ͼ ƺ::kLongDivision;Ũ Stack{Ϳ Stack(ĝ i6):base_(i6 à),ͽ{ģ ӟ=ß(n)ĺ(ӟ>size_-top_)throw Ø bad_alloc();Ǜ p=base_+top_;top_+=ӟ Ū p;}Ⱦ Ņ base_;Õ size_,top_ Ē};ԅ Ҁ(Ö x,Ö y){Ú Ö(ŉ(x)*y Ұ);}ԅ mod_sub(Ö x,Ö y){Ú x>=y?x-y:x+Ͻ y;}ԅ mod_negate(Ö x){Ú x?Ͻ x:0;}ԅ mod_inverse(Ö x){Ú á Ǹ(x,ӕ);}í combine3(Ɠ u,Ɠ v,ȧ,ř ʝ,ř to,Ö x,Ö y,Ö z){Ģ xs=ĕ(x),ys=ĕ(y),zs=ĕ(z)ø(ř j=ʝ&~ř(7);j<to Ӗ){Ģ a=Ž(u+j),b=Ž(v+j-1),c=Ž(v+j)κ Ŧ=ð(ð(Þ(a,xs),Þ(b,ys)),Þ(c,zs))κ Ȏ=ð(ð(Þ(ñ(a,32),xs),Þ(ñ(b,32),ys)),Þ(ñ(c,32),zs));ū(ѓ j,ń(ґ(Ŧ,Ȏ),kP));}}ˮ fill_pair(М,Ɠ x0,Ɠ x1,Õ g,ʅ(&Ļ)ǥ ɛ){ģ at=32*g κ a0 ɛ={Ō(x0+at),Ō(x0+at+8),Ō(x0+at ү),Ō(x0+at+24)}κ a1 ɛ={Ō(x1+at),Ō(x1+at+8),Ō(x1+at ү),Ō(x1+at+24)};ʿ(Ļ Ŭ,a0,ķ,g);ʿ(Ļ ƹ,a1,ķ,g);}Ą<Õ K>í ƕ(М,Ɠ x0,Ɠ x1,Ɠ Ù(&y_in)[K]ǥ,Ǜ(&j6)[K],ǀ){Ɠ y[K]ǥ;ȧ[K]ø(ѿ K ȝ)y ƫ Г y_in ƫ Ŭ,y ƫ ƹ=y_in ƫ ƹ,į ђ j6 ƫ ƣ ν(kR);ɥ ǥ ɛ;fill_pair(ķ,x0,x1,0,Ļ Ŭ)ø(Õ g Ē g<n/32 ė g){if(g+1<n/32)fill_pair(ķ,x0,x1,g+1,Ļ[(g+1)&1])ĩ ʅ(&w)ǥ ɛ=Ļ[g&1]ȡ at=32*g;
#pragma GCC unroll 4
Κ Ć(ѿ K ȝ){Ģ s ӑ(ƪ(Ǽ(w Ŭ ѯ,y ƫ Ŭ+at ϝ),Ǽ(w ƹ ѯ,y ƫ ƹ+at ϝ)));Š(į ƫ+at ϝ,ń(ŭ(s,ν),kP));}}}ƛ ǌ{Ņ ʮ ɛ;Õ ä ɛ;Ö at0 ɛ;Ņ ƀ ɛ;Õ î ɛ;Õ room,progress;};Ũ HalfGcd{Ⱥ ċ æ ř kDirect=64;ċ ý ɶ(Õ i9){Ú Ŋ(ʵ,ý(ʌ(ʘ(i9,2)-1)));}ċ Õ ƃ(Õ i9){Ú 48*(ǻ ɶ(i9))+4096;}ċ Õ č(Õ i9){Ú Č Ʈ(ɶ(i9))Я(ƃ(i9));}HalfGcd(ě&ŋ,Õ i9,ř ӥ=kDirect):Ǫ(ŋ,ɶ(i9)),ʄ(Ĭ(ƃ(i9))),direct_(ӥ){}ǌ š(Ȕ,Õ room){ǌ r ɫ{r.ʮ Ƈ ś(k+1)à;r.ƀ Ƈ room?ś(room)à:Ƿ;r.î Ĕ Ē}r.room=room Έ}Ĉ jump(һ,ř n,Ҡ,ř m,ř k,ǌ&į,ī ӟ=0xF){Ù ř Ʃ=n-k ĺ(m<Ʃ)Ú ȭ(į)ĺ(n>Ӵ){Ù ř s=n-Ӵ Ū jump(a+s,n-s,b+s,m-s,k,į,ӟ);}if(k<=direct_)Ú euclid(a,n,b,m,k,į)ȡ Ѥ,room=ʘ(64,ɬ(Õ(k)))ĩ ř k1=first_jump(k),s1=Ŋ<ř>(0,n-2*k1);ǌ r1=š(Õ(k1),room);jump(a+s1,n-s1,b+s1,m-s1,k1,r1);ř c4 Ē Ɠ c=Ƿ,*d=Ƿ ĺ(r1 ʱ ζ){r1=š(Õ(n-m),room);Ǜ ɼ=ś(Õ(n+1))à;divide_step(a,n,b,m,r1,ɼ);c=b,d=ɼ,c4=m;}ŝ{c4=n-ř(r1 ʱ)ĩ ř s2=Ŋ<ř>(0,2*Ʃ-c4);Ǜ cs=ś(Õ(c4+1))à,*Ù ds=ś(Õ(c4+1))à;apply(a,n,b,m,r1,c4,s2,cs,ds);c=cs,d=ds;}ř ϖ=c4-1 ǟ(ϖ>=Ʃ&&d[ϖ]ζ)--ϖ ĺ(ϖ<Ʃ){copy(r1,į)Ū ʄ ȍ;}Ù ř k2=c4-Ʃ,s2=Ŋ<ř>(0,c4-2*k2);ǌ r2=š(Õ(k2),room);jump(c+s2,c4-s2
,d+s2,ϖ-s2,k2,r2,(ӟ&3?3u:0u)|(ӟ&12?12u:0u));җ(r2,r1,į,ӟ);ʄ ȍ;}Ⱦ ċ ř first_jump(ř k){ģ u=Õ(k)Ū ř(ϩ(u)?u/2:Ø bit_floor(u));}ĝ ś(ǀ){Ú{ʄ ņ(n),n};}Ņ take_padded(ǀ){Ǜ p=ʄ ņ(n ү);ʇ(p,n ү,0)Ū p+8;}Ŵ ȭ(ǌ&į){į.ʮ Ŭ Г į.ʮ Ѐ Г 1;į.ä Г į.ä Ѐ=1;į.ä ƹ=į.ä ǥ Ē į Ӎ Г į Ӎ Ѐ=1,į Ӎ ƹ=į Ӎ ǥ ƒ(ł&l:į.î)l Ē į ʱ Ē}Ŵ copy(Ù ǌ&ʝ,ǌ&to){Т{ĵ(ʝ.ʮ Ĕ,ʝ.ä Ĕ,to.ʮ Ĕ);to.ä Ƈ ʝ.ä Ĕ;to Ӎ Ƈ ʝ Ӎ Ĕ;to.î Ĕ Ē}to ʱ=ʝ ʱ;}Ĉ euclid(һ,ř n,Ҡ,ř m,ř k,ǌ&į){ģ Ѥ ĩ ř Ʃ=n-k;ř e4=n-Ӵ,lo=Ŋ<ř>(0,e4);Ņ p Ѐ ø(ł&x:p)x=take_padded(Õ(n+1));ȩ(a+lo,a+n+1,p Ŭ+lo);ȩ(b+lo,b+m+1,p ƹ+lo)ȡ cap=Õ(k)+8;Ņ ξ Ѐ ǥ;Õ m2 Ѐ ǥ={{1,0},{0,1},{0,0}}ø(ł&r:ξ)Ć(ł&e:r){e=take_padded(cap);ʇ(e,cap,0);}ξ Ŭ Ŭ Г 1,ξ ƹ ƹ Г 1;Ņ c=p Ŭ,*d=p ƹ,*e=p ǥ;ř c4=n,ϖ=m ǟ(ϖ>=Ʃ){Ù ř d7=c4-ϖ;Ġ a9=d[ϖ];e4+=d7;lo=Ŋ<ř>(0,e4)ĺ(d7 у){Ġ beta=c[c4];Ġ e6=mod_sub(Ҁ(a9,c[c4-1]),Ҁ(beta,d[ϖ-1]));Ġ b0=Ҁ(a9,kR);Ġ x=Ҁ(a9,b0),y=Ͻ Ҁ(beta,b0),z=mod_negate(Ҁ(e6,kR));combine3(c,d,e,lo,ϖ,x,y,z)ø(ъ){ģ sc=m2 Ŭ Ĕ,sd=m2 ƹ Ĕ ȡ se=Ŋ(sc,sd?sd+1:0);combine3(ξ Ŭ Ĕ,ξ ƹ Ĕ,Ҷ,0,ř(se),x,y,z)ĺ(m2 ǥ Ĕ>se)ż(Ҷ+se,Ҷ+m2 ǥ Ĕ,0);m2 ǥ Ƈ se
;}}ŝ{pseudo_divide(c,c4,d,ϖ,e,lo,a9,ξ,m2);}Ǜ old=c;c=d,d=e,e=old ø(ъ){ь(ξ Ŭ Ĕ,ξ ƹ Ĕ);ь(ξ ƹ Ĕ,Ҷ);ь(m2 Ŭ Ĕ,m2 ƹ Ĕ);ь(m2 ƹ Ĕ,m2 ǥ Ĕ);}c4=ϖ;ϖ=c4-1 ǟ(ϖ>=Ʃ&&d[ϖ]ζ)--ϖ;}Ć(ъ)Ć(Ϡ){ĵ(ξ Ĕ Ɓ,m2 Ĕ Ɓ,į.ʮ Й);į.ä Й=m2 Ĕ Ɓ;į Ӎ Й=m2 Ĕ Ɓ?ξ Ĕ Ɓ Ŭ:0;į.î Й Ē}į ʱ=Õ(n-c4);ʄ ȍ;}Ŵ pseudo_divide(Ɠ c,ř c4,Ɠ d,ř ϖ,Ņ e,ř lo,Ö a9,Ņ(&ξ)Ѐ ǥ,Õ(&m2)Ѐ ǥ){ȩ(c+lo,c+c4+1,e+lo)ø(ъ){ĵ(ξ Ŭ Ĕ,m2 Ŭ Ĕ,Ҷ)ĺ(m2 ǥ Ĕ>m2 Ŭ Ĕ)ż(Ҷ+m2 Ŭ Ĕ,Ҷ+m2 ǥ Ĕ,0);m2 ǥ Ƈ m2 Ŭ Ĕ;}Ć(ř t=c4-ϖ;t и t){Ġ i7=mod_negate(e[ϖ+t])ø(ř j=lo;j<=ϖ+t ų){Ġ dj=j-t>=0?d[j-t]:0;e ˉ Ö((ŉ(a9)*e Ɓ+ŉ(i7)*dj)Ұ);}Ć(ъ){Ǜ re=Ҷ;Ɠ Ù rd=ξ ƹ Ĕ ȡ sd=m2 ƹ Ĕ,ƌ=Õ(t)ȡ se=Ŋ(m2 ǥ Ĕ,sd?sd+ƌ:0)ø(ǉ se ų){Ġ dj=j>=ƌ?rd[j-ƌ]:0;re ˉ Ö((ŉ(a9)*re Ɓ+ŉ(i7)*dj)Ұ);}m2 ǥ Ƈ se;}}}Ĉ divide_step(һ,ř n,Ҡ,ř m,ǌ&r,Ņ ɼ){ģ d7=Õ(n-m);Ņ q=r.ʮ Ѐ ĺ((d7+1)*Õ(m+1)<=kLongDivision){ȩ(a,a+n+1,ɼ);Ġ Μ=mod_inverse(b[m])ø(ɵ=d7+1;i-->0;){Ġ qi=Ҁ(ɼ[Õ(m)+i],Μ);q Ƈ qi ø(ř t Ē t<=m ė t)ɼ[i+Õ(t)]=mod_sub(ɼ[i+Õ(t)],Ҁ(qi,b ѯ));}}ŝ{Ù О(a,Õ(n+1)),g(b,Õ(m+1))й(divide_words(Ǳ,ȸ));divide(ŋ,f,g,ĝ(q,d7+1),ĝ(ɼ,Õ(m)));ż(ɼ+m,ɼ+n+1,0);}Ć(ɵ Ē i<=
d7 Į)q Ƈ mod_negate(q Ĕ);r.ʮ ƹ Г r.ʮ ǥ Г 1;r.ä Г 0,r.ä ƹ=r.ä ǥ=1,r.ä Ѐ=d7+1;r Ӎ Г 0,r Ӎ ƹ=r Ӎ ǥ=1,r Ӎ Ѐ=q Ŭ ø(ł&l:r.î)l Ē r ʱ=d7;}Ĉ transform_of(Ɠ c,Õ ä,ɐ)Ù{ģ ǯ=ǲ ĺ(ä<=ǯ)Ú Ǫ.ı(Ğ(c,ä),0,į);ѷ(Ğ(c,ä),į);Ǫ.ı(į);}Ĉ extend(ǌ&r,ý i,Õ ǯ)Ù{if(r.î Ĕ==ǯ)Ú ǫ t(r.ƀ Ĕ,ǯ)ȡ Ɵ=ǯ/2,ä=r.ä Ĕ ĺ(r.î Ĕ==Ɵ&&ä<=Ɵ+1){Ù ĝ ʓ=t Ė(Ɵ)ĺ(ä<=Ɵ){Ǫ.ɇ(Ğ(r.ʮ Ĕ,ä),0,ʓ);}ŝ{ĵ(r.ʮ Ĕ,Ɵ,ʓ à);ʓ Г mod_sub(ʓ Ŭ,r.ʮ Ĕ[Ɵ]);Ǫ.ɇ(ʓ,0,ʓ);}}ŝ{transform_of(r.ʮ Ĕ,ä,t);}r.î Ƈ ǯ;}Ĉ apply(һ,ř n,Ҡ,ř m,ǌ&r1,ř c4,ř s2,Ņ c,Ņ d){ģ Ѥ ȡ ǯ=ʘ(64,ɬ(Õ(n+1)/2))ɫ extend(r1,i,ǯ)ǫ ta Ӷ,tb Ӷ;transform_of(a,Õ(n+1),ta);transform_of(b,Õ(m+1),tb)ǫ pc Ӷ,pd Ӷ;apply_products(r1,ta,tb,pc,pd)ĩ ř l=ř(ǯ),mixed=c4-l ĺ(mixed<0){ȩ(pc ʷ s2,pc ʷ c4+1,c+s2);ȩ(pd ʷ s2,pd ʷ c4+1,d+s2)Ū ʄ ȍ;}ģ ʗ=Õ(mixed+1)ǫ lc Ӷ,ld Ӷ ȡ є=ʘ(64,ɬ(2*ʗ-1))ĺ(є<ǯ){ǌ r=r1 ɫ{r.ƀ Ƈ ś(є)à;Ǫ.ı(Ğ(r1.ʮ Ĕ,Ħ(ʗ,r1.ä Ĕ)),0,ĝ(r.ƀ Ĕ,є));}Ù ĝ sa=ta ȑ(є),sb=tb ȑ(є);Ǫ.ı(Ğ(a,ʗ),0,sa);Ǫ.ı(Ğ(b,Ħ(ʗ,Õ(m+1))),0,sb);apply_products(r,sa,sb,lc ȑ(є),ld ȑ(є));}ŝ{Ǫ.ı(Ğ(a,ʗ),0,ta);Ǫ.ı(Ğ(b,Ħ(ʗ,Õ(m+1))),0,tb);apply_products(r1,ta,tb
,lc,ld);lc Г Ö((ŉ(r1 Ӎ Ŭ)*a Ŭ+ŉ(r1 Ӎ ƹ)*b Ŭ)Ұ);ld Г Ö((ŉ(r1 Ӎ ǥ)*a Ŭ+ŉ(r1 Ӎ Ѐ)*b Ŭ)Ұ);}Ù ř ù Ш(s2,mixed+1);ȩ(lc ʷ ù,lc ʷ mixed+1,c+ù);ȩ(ld ʷ ù,ld ʷ mixed+1,d+ù)ĩ ř ʝ=Ŋ(s2,mixed+1);ȩ(pc ʷ ʝ,pc ʷ l,c+ʝ);ȩ(pd ʷ ʝ,pd ʷ l,d+ʝ);ʫ(pc à,lc à,c+l,Õ(mixed+1));ʫ(pd à,ld à,d+l,Õ(mixed+1));ʄ ȍ;}Ĉ apply_products(Ù ǌ&r1,Ğ ta,Ğ tb,ĝ c,ĝ d)Ù{ģ ǯ=c Û Ǎ t=Ѯ(ý i){Ú Ğ(r1.ƀ Ĕ,ǯ);}ĩ ɝ pc ǥ={{t(0),ta},{t(1),tb}},pd ǥ={{t(2),ta},{t(3),tb}};Ǫ.Ŗ(pc,c);Ǫ.Ŗ(pd,d);}Ĉ җ(ǌ&r2,ǌ&r1,ǌ&į,ī ӟ){ģ Ѥ ȡ p=r2 ʱ+r1 ʱ,ǯ=ʘ(64,ɬ(p))ø(ъ)if(ӟ&(3u<<2*i))extend(r2,2*i,ǯ),extend(r2,2*i+1,ǯ)ø(Ϡ)if(ӟ&(5u<<j))extend(r1,j,ǯ),extend(r1,2+j,ǯ)ǫ buffer Ӷ ĩ ƍ keep=į.room>=ǯ;Ņ t ɛ ɫ t Ƈ keep?į.ƀ Ĕ:ś(ǯ)à ø(ъ){Ɠ x0=r2.ƀ ɔ,*x1=r2.ƀ ư;Ɠ Ù y ǥ ǥ={{r1.ƀ Ŭ,r1.ƀ ǥ},{r1.ƀ ƹ,r1.ƀ Ѐ}}ĩ ī ξ=ӟ>>2*i&3 ĺ(ξ==3){Ǜ to ǥ={t ɔ,t ư};ƕ<2>(Ǫ.ķ(),x0,x1,y,to,ǯ);}м(ξ){ȟ j=ξ у?0:1;Ɠ Ù yj ƹ ǥ={{y Ɓ Ŭ,y Ɓ ƹ}};Ǜ to ƹ={t Й};ƕ<1>(Ǫ.ķ(),x0,x1,yj,to,ǯ);}}Ć(ъ)Ć(Ϡ){ȟ ij=2*i+j;į.î[ij]Ē Ġ x0=Ö((ŉ(r2 Ӎ ɔ)*r1 Ӎ Ɓ+ŉ(r2 Ӎ ư)*r1 Ӎ[2+j])Ұ);į Ӎ[ij]=x0 ĺ(!(ӟ>>ij&1))ӊ ĺ(keep)į.î[ij]=ǯ;Ǫ.ć(
Ğ(t[ij],ǯ),buffer);Ǜ e=į.ʮ[ij]Ԗ(buffer à,Ħ(p+1,ǯ),e)ĺ(p==ǯ)e[ǯ]=mod_sub(e Ŭ,x0),e Г x0;Õ ä=p+1 ǟ(ä>0&&e[ä-1]ζ)--ä;į.ä[ij]=ä;}į ʱ=p;ʄ ȍ;}Č Ǫ;Stack ʄ;ř direct_;};}Ί inverse_mod_words(ǀ,Ό){ģ d=m-1 Ū divide_words(n,m)Я(d)Я(З(n,m))+ӝ::HalfGcd Ʈ(ʘ(d,1));}ò ř inverse_mod(ě&ŋ,О,Ğ g,ĝ h){Ę ā ӝ ĩ ř d=ř(ȸ)-1 ĺ(d ζ)Ú 0 ǫ b ф(Õ(d))ȡ rs=remainder_size(Ǳ,ȸ);divide(ŋ,f,g,Ĭ(З(Ǳ,ȸ)),b ȑ(rs));ř m=ř(rs)-1 ǟ(m>=0&&b[Õ(m)]ζ)--m ĺ(m<0)Ú-1;HalfGcd gcd(ŋ,Õ(d));ǌ r=gcd.š(Õ(d),0);gcd.jump(g à,d,b à,m,d,r,0x2)ĺ(ř(r ʱ)!=d)Ú-1;Ġ c=Ö((ŉ(r Ӎ Ŭ)*g Ŭ+ŉ(r Ӎ ƹ)*b Ŭ)Ұ);Ġ ŧ=mod_inverse(c);Õ t=r.ä ƹ ǟ(t>0&&r.ʮ ƹ[t-1]ζ)--t ø(Ů t Į)h Ƈ Ҁ(r.ʮ ƹ Ĕ,ŧ)Ū ř(t);}}ā ç ē{ò ƍ native_inv_mod(ǣ*a,ǀ,ǣ*b,Ό,л){ě ŋ(inverse_mod_words(n,m)Я(n)Я(m)Я(m))Ѵ f к,g ф(m),h Θ Ǎ ä=inverse_mod(ŋ,f,g,h)ĺ(ä<0)Ú Ś;ƙ(į,Õ(ä));ƺ::append_words(į,h à,Õ(ä),932051910)Ū Ɔ;}Ő Sink>í native_gcd_matrix(ǣ*a,ǀ,ǣ*b,Ό,ī ӟ,Sink sink){ě ŋ(ӝ::HalfGcd Ʈ(ԕ)Я(n)Я(m))Ѵ f к,g Θ;ӝ::HalfGcd gcd(ŋ,ԕ)Ѵ š=gcd.š(ԕ,0);gcd.jump(f à,ř(n)-1,g à,ř(m)-1,ř(n)-1,š,ӟ);sink(š);}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ӂ{ò Ȭ::ǌ polynomial_gcd_jump(Ù Ȭ::F&a,Ù Ȭ::F&b){
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(Ư>=129&&Ư<=(1u<<23)&&ę Ĝ){Ȭ::ǌ į;ʃ native_gcd_matrix(ʰ,Ư,b à,η,3,[&į](Ù ʃ ӝ::ǌ&š){į.ƌ=ý(š ʱ),į.ȭ=š ʱ ζ ø(Ϡ){į.a Ɓ.clear();į.a Ɓ.reserve(š.ä Ɓ);ʃ ƺ::append_words(į.a Ɓ,š.ʮ Ɓ,š.ä Ɓ,á kR);}})Ū į;}
#endif
Ú Ȭ::jump(a,b,џ-1,Ɔ);}}ˬ ƛ PolynomialExtendedGCD{Ȼ gcd,x,y;};ˬ Ȼ polynomial_gcd(Ȼ a,Ȼ b){Ę H=ç Ȭ;Ƙ ӵ(polynomial_gcd(Һ(a),Һ(b)))і{ɞ(a),ɞ(b)ĺ(Ư<η)a.swap(b)ĺ(!b ű&&Ư==η){ł r=a.divmod(b).ψ;a=ƅ(b),b=ƅ(r);}if(!b ű){ł š=ç polynomial_gcd_jump(a,b)ĺ(š.ƌ==џ-1)Ú{Ӹ};a=H::apply_first(š,a,b);}if(!a ű)a/=a Ӱ Ū a;}}ˬ PolynomialExtendedGCD<T>polynomial_extended_gcd(Ȼ a,Ȼ b){Ę H=ç Ȭ;Ƙ;ƴ{ł ă=polynomial_extended_gcd(Һ(a),Һ(b))Ū{ǆ ƿ(ă.gcd),ǆ ƿ(ă.x),ǆ ƿ(ă.y)};}ŝ{ɞ(a),ɞ(b)ĺ(a ű&&b ű)Ú{};ӈ H::ǌ š ǟ(!b ű&&Ư<=η){ł[q,r]=a.divmod(b);H::ɯ(š,q),a=ƅ(b),b=ƅ(r);}if(!b ű){ł part=ç polynomial_gcd_jump(a,b);a=H::apply_first(part,a,b);š=H::Ź(part,š,Ɔ);}T ć=a Ӱ ˆ;a*=ć,Ή Ŭ*=ć,Ή ƹ*=ć Ū{ƅ(a),ƅ(Ή Ŭ),ƅ(Ή ƹ)};}}ˬ Ø optional<Ď<T>>polynomial_inv_mod(Ȼ a,Ȼ i8){ç ǅ::trim(i8);ơ(!i8 ű)ĺ(i8 Û у)Ú Ȼ{}ͼ H=ç Ȭ;Ƙ;ƴ{ł ă=polynomial_inv_mod(Һ(a),Һ(i8))ĺ(!ă)Ú Ø nullopt Ū ǆ ƿ(*ă);}ŝ{
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(i8 Û>=129&&i8 Û<=(1u<<23)&&Ư<=(1u<<23)ȏ){Ȼ į ĺ(!ç ʃ native_inv_mod(ʰ,Ư,i8 à,i8 Û,į))Ú Ø nullopt Ū į;}
#endif
if(Ư>=i8 Û)a=a.divmod(i8).ψ;ɞ(a)ĺ(a ű)Ú Ø nullopt Ѵ š=H::jump(i8,a,ý(i8 Û)-1,Ɔ)ĺ(š.ƌ!=ý(i8 Û)-1)Ú Ø nullopt;T ˏ=(Ή Ŭ ű?Ȓ:Ή Ŭ Ŭ*i8 Ŭ)+(Ή ƹ ű?Ȓ:Ή ƹ Ŭ*a Ŭ);Ή ƹ/=ˏ Ū ƅ(Ή ƹ);}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ ɮ(Ù Ȼ&f,Ù Ȼ&ş,ў){Ѽ;Ƙ Н ó(ş Û);ơ(ǁ>=0&&ǁ<=(1<<22)&&ş Û<=(1U<<21))ĺ(ǁ ζ)Ú{}ĺ(ǁ у)Ú F{ş ű?Ȓ:ş Ŭ};ƴ{ӈ ǆ W ʎ(Ħ(Ǳ,ş Û))ø(ö ó(ʎ Û)Į)ʎ Ƈ Ě ι(f Ĕ ȃ)Ū ǆ ƿ(ɮ<Ě>(ʎ,Һ(ş),ǁ));}ŝ{ǧ=ó(ş Û)ǟ(n&&ş[ԕ]Ү)--n ĺ(n ζ)Ú F(ǁ);ί=ʡ(n,Ǳ),ù=1 ǟ(ù<î&&f[ù]Ү)++ù;T c=f ű?Ȓ:f Ŭ ĺ(ù>=î){ѫ;T ĳ=ş Ŭ ø(ł&x:ă)x=ĳ,ĳ*=c Ȧ}if(ù>1){Ӄ=ù ø(Ż ҵ Ͼ î&&ď>1 Į)if(f Ĕ!=Ȓ)ď=Ø gcd(ď,i)ĺ(ď>1){F ʸ((î-1)/ď+1),w((ԕ)/ď+1)ø(ö ó(ʸ Û)Į)ʸ Ƈ f[i Ԏ]ø(ö ó(w Û)Į)w Ƈ ş[i Ԏ]Ū ɮ(ʸ,w,ǁ);}}ý þ Ш(ǁ,(ԕ)/ҵ);F ă;ý Ȣ=î-1 ǟ(f[Ȣ]Ү)--Ȣ ĺ(ù==Ȣ){ă Ř(þ);T ĳ=1 ø(ö þ Į)ă Ƈ ş[i*ù]*ĳ,ĳ*=f[ù];}м(n<=32||þ<=8){ă Ř(þ);F h(f ü,f ʷ Ȣ+1),Ǹ{Ӹ};h Ŭ ƒ(ö þ Į){T ĳ ƒ(ȓ ó(Ǹ Û)ų)ĳ+=Ǹ Ɓ*ş Ɓ;ă Ƈ ĳ ĺ(i+1<þ){Ǹ=F::Ź(Ǹ,h)ĺ(ó(Ǹ Û)>n)Ǹ Ř(n);}}}ŝ
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(n<=(1<<21)ȏ){ç Ȗ::native_weighted_projection(f à,î,ş à,ă,n,þ);}ŝ
#endif
{ă Ř(þ);ý ä=1 ǟ(ä<n)ä<<=1;ơ(ä<=(1<<21));ȷ,*q=p+4*ä Ԗ(Ȯ(ş),n,p);ʇ(p+n,ä-n,0u);q Ŭ ƒ(Ż 1;i<=Ȣ Į)q Ƈ(-f Ĕ).v;ʇ(q+Ȣ+1,Ё-Ȣ-1,0u);ƍ unit=n==ä&&ş[ԕ]==Ӹ&&Ø all_of(ş ü,ş ʷ ԕ,[](T x){Ú x Ү;})Ѵ Ÿ=t.reversion_projection(p,q,ä,unit?Ƿ:p);t.ŧ(p,þ,t.į(Ÿ))ø(ö þ Į)ă Ƈ T ɪ(p Ĕ);}if(c Ү){ă ә Ȧ}F b(ǁ);T ԓ=1 ø(λ ǁ Į)ԓ*=i;T g9=ԓ ˆ ø(Ż ǁ-1;i и i){b Ƈ g9 ĺ(i<þ)ă Ĕ*=g9;g9*=i;}ǆ ɘ(b,Ӹ,c);ă=F::Ź(ă,b);ă ә;ԓ=1 ø(ö ǁ Į)ă Ĕ*=ԓ,ԓ*=i+1 Ȧ}}ˬ Ȼ ɮ(Ù Ȼ&f,ǧ,ў){ơ(n>=0&&n<(1<<21));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
ġ{if(n>=64&&Ǳ>2&&f Ŭ Ү&&f ƹ!=Ȓ&&f ǥ!=Ȓ ȏ){ȟ þ=ǁ<0?n+1:ǁ;ơ(þ>=0&&þ<=(1<<22))ĺ(þ ζ)Ú{};Ȼ ă;ç Ȗ::native_projection(f à,Ǳ,ă,n+1);ă Ř(þ)Ȧ}}
#endif
Ȼ ş(n+1);ş[n]=1 Ū ɮ(f,ş,ǁ);}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ӂ{ā taylor_factorials{ò ӄ kStep=1024;Ӂ Block>ò æ ł factorial_block=[]{ǖ Ö,16>ă{};ŉ ĳ=1 Ѓ(Block>0)ĳ=factorial_block<Block-1>Ӱ ø(ö 16 Į){Ć(Ö j=1;j<=kStep ų)ĳ=ĳ*((Block*16+i)*kStep+j)%998244353;ă Ƈ Ö(ĳ);}Ѡ}();ò æ ł kTable=[]<Õ...I>(Ø index_sequence<I...>){Ú ǖ Ö,513>{1,factorial_block<I/16>[I%16]...};}(Ø make_index_sequence<512>{});}ā taylor_detail{ā ntt=ʉ::ntt ȅ ƪ ȅ ĕ ȅ Ǚ ȅ kP ȅ kR ȅ Đ ȅ Ǹ ȅ ń ȅ φ ͼ ʉ::Subtrees ȅ ĭ ͼ ʃ â Ō ͼ ʃ â ʗ ͼ ʃ â Š;ԅ ć(Ö x){Ú Ǹ(x,ӕ);}ԅ mont(Ö x){Ú Đ(x,kR);}Ȅ ȕ(ɍ){Ú ʤ;}ĉ Ȅ Ŕ(ĭ a,ĭ a4,ĭ b,ĭ b4){Ģ ni=ĕ(ntt::Ƥ kNI),p=ɟ;н=Þ(a,b),Ȏ=Þ(a4,b4);ĥ Ú ǝ;}ˮ ʈ(ĭ(&r)ɚ){ĭ t ɚ,u ɚ;
#pragma GCC unroll 4
Ć(ö 8 ƽ 2){ͻ;t Υ ǿ(r Ĕ,r Υ);}
#pragma GCC unroll 2
Ć(ö 8 ƽ 4){u Ƈ ɻ(t Ĕ,t[i+2]);u Υ Ǿ(t Ĕ,t[i+2]);u[i+2]=ɻ(t Υ,t[i+3]);u[i+3]Ǿ(t Υ,t[i+3]);}
#pragma GCC unroll 4
Т{ѹ;з}}æ ý ȫ=4,ɤ=8*ȫ;Ő F>Ĉ fill(ĭ(&x)[ȫ],F f){љ Ƴ[ɤ]Ԓ Ƴ[s]=f(s)ø(ʚ ȫ ė v)x ӓ ӷ(Ƴ Ʀ v);}Ę Block=ĭ[ȫ]ɚ;ƛ Chain{ɍ[ȫ],Ƽ[ȫ],b6[ȫ],u,ɯ;Ő X,Ũ Base>Chain(X g4,Base b7,Ö n6):u(đ),ɯ(ĕ(n6)){fill(x,g4);fill(Ƽ,b7)ø(ʚ ȫ ė v)b6 ӓ=ȕ(Ƽ ӓ);}ĉ Ĉ advance(Block&ŀ,ý t){
#pragma GCC unroll 4
Ć(ʚ ȫ ė v){ŀ ӓ ѯ=x ӓ;x ӓ=Ŕ(x ӓ,ȕ(x ӓ),ƪ(Ƽ ӓ,u),ƪ(b6 ӓ,u));}u=ń(ƪ(u,ɯ),kP);}};Ũ TaylorShift{Ϳ TaylorShift(ǀ):n_(n){lg_=Ŋ(10,ý(ʌ(2*ԕ)));lg_+=lg_%2;chunk_=((n+ɤ-1)/ɤ+15)/16*16 ĺ(chunk_/16%2 ζ)chunk_+=16 ȡ ǯ=î(),Ķ=ntt::â ϛ(lg_)ȡ č=2*ǯ+3*kPadding ʳ Ķ;ϴ=ɀ(č)ĺ(!ϴ.get_deleter().region)ʇ(ϴ Ӳ,č,0u);b_=ϴ Ӳ+kPadding;a_=b_+ǯ+kPadding;Ə=a_+ǯ+kPadding;ō=Ə+Ķ;}Ņ ļ(){Ú a_;}Ɠ ƌ(Ö c){Ę ā ntt::ê ȡ ǯ=î(),q=ǯ/64;build_table(Ə,ǯ/16,kRoots Ŭ);build_table(ō,ǯ/16,kRoots ƹ);lane_factorials();ş(c)Ѵ*av=Ț(a_)Ѵ*bv=Ț(b_);ʉ::forward_radix8(av,q,Ə);ʉ::forward_radix8(bv,q,Ə)ø(Ӆ<8 ė t){Ö&x=b_[8*q*t];x=(x Ұ+(t<4?1:Ͻ 1))Ұ;}Ù Subtrees Ѧ(Ə,ō)Ȳ Ѧ.Ϙ(av+t*q,bv+t*q,q,t);ntt::Ƥ inverse_identity(av,q,ō)ø(Õ t=4;t<8 ė t)Ѧ.Ϙ(av+t*q,bv+t*q,q,t);ntt::Ƥ ć(av+4*q,q,ō+φ(1),ō+φ(2));ǂ()Ū b_;}Ⱦ ċ ɿ kPadding=1040;Õ î()Ù{Ú ǻ lg_;}Õ start(ý s)Ù{Ú s*chunk_;}Ĉ lane_factorials(){Ę taylor_factorials::kStep Ǎ Ԍ=[this](ý s){Ú Ö(start(s+1)/kStep*kStep);};ɍ[ȫ],m[ȫ],þ[ȫ];fill(x,Ѯ(ý s){Ú mont(taylor_factorials::kTable[Ԍ(s)/kStep]);});fill(m,Ѯ(ý s){Ú mont(
Ԍ(s)+1);});fill(þ,Ѯ(ý s){Ú Ö(start(s+1)-Ԍ(s));})κ unit=ĕ(kR)ø(Ö j Ē j<kStep-1 ų){Ģ ɯ=ĕ(j);
#pragma GCC unroll 4
Ć(ʚ ȫ ė v){Ģ Ÿ=_mm256_blendv_epi8(unit,m ӓ,τ(þ ӓ,ɯ));x ӓ=Ŕ(x ӓ,ȕ(x ӓ),Ÿ,ȕ(Ÿ));m ӓ=ń(ƪ(m ӓ,unit),kP);}}љ h7[ɤ]ø(ʚ ȫ ė v)Š(h7 Ʀ v,x ӓ);Ġ o6=ć(kR);Ӝ Г 1 Ԓ Ӝ[s+1]=Đ(h7[s]Ұ,o6);Ö f=1 ø(ý s Ē s<=ɤ ė s)ϫ[s]=f,f=Đ(f,Ӝ[s]);f=ć(f)ø(ý s=ɤ;s и s)ϫ[s]=Đ(ϫ[s],f),f=Đ(f,Ӝ[s]);}Ĉ ş(Ö c){Ġ c3=Ǹ(c,Ö(chunk_)),c6=mont(ć(c));ǖ Ö,ɤ>c5 Ԓ c5[s]=s?Đ(c5[s-1],c3):c3;Chain ʭ([this](ý s){Ú mont(Ӝ[s]);},[this](ý s){Ú mont(Ö(start(s)+1));},kR);Chain kernel(Ѯ(ý s){Ú Đ(c5[s],ϫ[s+1]);},Ѯ(ý s){Ú Đ(Ö(start(s+1)),c6);},Ͻ c6)ȡ ď=chunk_;Block ı ǥ,b5 ǥ;
#pragma GCC unroll 8
С ʭ.advance(ı Ŭ,t),kernel.advance(b5 Ŭ,t)ø(ʍ=0,ԍ Ē j<ď Ӗ,ԍ^=1){Ǜ a=a_+j;Ǜ e=b_+î()/2-ď+j;
#pragma GCC unroll 8
С{ʭ.advance(ı[ԍ^1],t);kernel.advance(b5[ԍ^1],t)ȉ v=ȫ*t/8,ù=ȫ*t%8 ĺ(ù ζ)ʈ(ı[ԍ]ӓ),ʈ(b5[ԍ]ӓ);
#pragma GCC unroll 4
Ć(ý l=ù;l<Ў ȫ ė l){ģ at=(8*v+l)Ԏ κ f ӷ(a+at),x=ı[ԍ]ӓ[l];Š(a+at,ń(Ŕ(f,ȕ(f),x,ȕ(x)),kP));Š(e-at,ń(b5[ԍ]ӓ[l],kP));}}}}Ĉ ǂ(){Ġ ŧ=Đ(mont(ć(Ö(î()/8))),kR);Chain chain(Ѯ(ý s){Ú Đ(Đ(ϫ[s+1],Ö(start(s+1))),ŧ);},[this](ý s){Ú mont(Ö(start(s+1)-1));},Ͻ kR)ȡ ď=chunk_,Ɵ=î()/2;Block ŀ ǥ;
#pragma GCC unroll 8
С chain.advance(ŀ Ŭ,7-t)ø(ʍ=0,ԍ Ē j<ď Ӗ,ԍ^=1){Ɠ Ù u=a_+ď-8-j;Ǜ į=b_+ď-8-j;
#pragma GCC unroll 8
С{chain.advance(ŀ[ԍ^1],7-t)ȉ v=ȫ*t/8,ù=ȫ*t%8 ĺ(ù ζ)ʈ(ŀ[ԍ]ӓ);
#pragma GCC unroll 4
Ć(ý l=ù;l<Ў ȫ ė l){ģ at=(8*v+l)Ԏ κ g ӑ(Ǚ(Ō(u+at),Ō(u+at+Ɵ))),z=ŀ[ԍ]ӓ[l];Š(ѓ at,ń(Ŕ(g,ȕ(g),z,ȕ(z)),kP));}}}}Õ n_,chunk_;ý lg_;Ņ a_,*b_,*Ə,*ō;FpsWords ϴ;ǖ Ö,ɤ+1>Ӝ,ϫ;};í ƌ(Ʋ&f,Ě c){TaylorShift t(Ǳ);ɵ ƒ(ʲ Ǳ Ɲ){ʦ x=ʃ â Ž(õ Ù ī*>(f Ȝ));Š(t.ļ()+i,ń(x,kP));}Ć(Ͼ Ǳ Į)t.ļ()Ƈ Ħ(f Ĕ.v,f Ĕ.v-kP)Ǎ*į=t.ƌ(c ȃ);i ƒ(ʲ Ǳ Ɲ)ʃ â ū(õ ī*>(f Ȝ),Ō(ѓ i))ø(Ͼ Ǳ Į)f Ƈ Ҿ(į Ĕ);}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ taylor_shift(Ȼ f,Ø type_identity_t<T>c){Ѽ;Ƙ ӵ(taylor_shift<Ě>(Һ(f),Ě ι(c ȃ)))і{Ù ǧ=ó(Ǳ)ĺ(n<=1||c Ү)Ú f;ơ(ī(n)<ī(Ҳ));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(n>=256&&n<=(1<<19)ȏ){ç taylor_detail::ƌ(f,c)Ū f;}
#endif
F a(n),b(n),ş;ç Ө(Ȯ(a),Ȯ(b),n);Ű Ŏ(Ȯ(a),Ȯ(a),Ȯ(f),n);Ø ˁ(a ü,a Ƃ);ş=b;ǆ ɘ(b,Ӹ,c);F prod=F::Ź(a,b);Ø ˁ(prod ü,prod ʷ n);Ű Ŏ(Ȯ(f),Ȯ(prod),Ȯ(ş),n)Ū f;}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#undef Õ
#undef Ö
#undef Ø
#undef Ù
#undef Ú
#undef Û
#undef Ü
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
#undef ì
#undef í
#undef î
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
#undef ü
#undef ý
#undef þ
#undef ā
#undef Ă
#undef ă
#undef Ą
#undef ą
#undef Ć
#undef ć
#undef Ĉ
#undef ĉ
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
#undef Ħ
#undef ħ
#undef Ĩ
#undef ĩ
#undef Ī
#undef ī
#undef Ĭ
#undef ĭ
#undef Į
#undef į
#undef İ
#undef ı
#undef ĳ
#undef Ĵ
#undef ĵ
#undef Ķ
#undef ķ
#undef ĸ
#undef Ĺ
#undef ĺ
#undef Ļ
#undef ļ
#undef Ľ
#undef Ŀ
#undef ŀ
#undef Ł
#undef ł
#undef Ń
#undef ń
#undef Ņ
#undef ņ
#undef Ň
#undef ň
#undef ŉ
#undef Ŋ
#undef ŋ
#undef Ō
#undef ō
#undef Ŏ
#undef ŏ
#undef Ő
#undef ő
#undef Œ
#undef œ
#undef Ŕ
#undef ŕ
#undef Ŗ
#undef ŗ
#undef Ř
#undef ř
#undef Ś
#undef ś
#undef Ŝ
#undef ŝ
#undef ş
#undef Š
#undef š
#undef ţ
#undef Ť
#undef ť
#undef Ŧ
#undef ŧ
#undef Ũ
#undef ũ
#undef Ū
#undef ū
#undef Ŭ
#undef ŭ
#undef Ů
#undef ů
#undef Ű
#undef ű
#undef Ų
#undef ų
#undef Ŵ
#undef ŵ
#undef Ŷ
#undef Ÿ
#undef Ź
#undef ź
#undef Ż
#undef ż
#undef Ž
#undef ž
#undef ſ
#undef ƀ
#undef Ɓ
#undef Ƃ
#undef ƃ
#undef Ƅ
#undef ƅ
#undef Ɔ
#undef Ƈ
#undef ƈ
#undef Ɖ
#undef Ƌ
#undef ƌ
#undef ƍ
#undef Ǝ
#undef Ə
#undef Ɛ
#undef Ƒ
#undef ƒ
#undef Ɠ
#undef Ɣ
#undef ƕ
#undef Ɩ
#undef Ƙ
#undef ƙ
#undef ƚ
#undef ƛ
#undef Ɯ
#undef Ɲ
#undef ƞ
#undef Ɵ
#undef Ơ
#undef ơ
#undef Ƣ
#undef ƣ
#undef Ƥ
#undef ƥ
#undef Ʀ
#undef Ƨ
#undef ƨ
#undef Ʃ
#undef ƪ
#undef ƫ
#undef Ƭ
#undef ƭ
#undef Ʈ
#undef Ư
#undef ư
#undef Ʊ
#undef Ʋ
#undef Ƴ
#undef ƴ
#undef Ƶ
#undef ƶ
#undef Ʒ
#undef Ƹ
#undef ƹ
#undef ƺ
#undef ƻ
#undef Ƽ
#undef ƽ
#undef ƾ
#undef ƿ
#undef ǀ
#undef ǁ
#undef ǂ
#undef Ǆ
#undef ǅ
#undef ǆ
#undef Ǉ
#undef ǈ
#undef ǉ
#undef Ǌ
#undef ǋ
#undef ǌ
#undef Ǎ
#undef ǎ
#undef ǐ
#undef Ǒ
#undef Ǔ
#undef ǔ
#undef Ǖ
#undef ǖ
#undef Ǘ
#undef ǘ
#undef Ǚ
#undef ǚ
#undef Ǜ
#undef ǜ
#undef ǝ
#undef Ǟ
#undef ǟ
#undef Ǡ
#undef ǡ
#undef Ǣ
#undef ǣ
#undef Ǥ
#undef ǥ
#undef Ǧ
#undef ǧ
#undef Ǩ
#undef ǩ
#undef Ǫ
#undef ǫ
#undef Ǭ
#undef ǭ
#undef Ǯ
#undef ǯ
#undef ǰ
#undef Ǳ
#undef ǲ
#undef ǳ
#undef Ǵ
#undef ǵ
#undef Ƕ
#undef Ƿ
#undef Ǹ
#undef ǹ
#undef Ǻ
#undef ǻ
#undef Ǽ
#undef ǽ
#undef Ǿ
#undef ǿ
#undef Ȁ
#undef ȁ
#undef Ȃ
#undef ȃ
#undef Ȅ
#undef ȅ
#undef Ȇ
#undef ȇ
#undef Ȉ
#undef ȉ
#undef Ȋ
#undef ȋ
#undef Ȍ
#undef ȍ
#undef Ȏ
#undef ȏ
#undef Ȑ
#undef ȑ
#undef Ȓ
#undef ȓ
#undef Ȕ
#undef ȕ
#undef Ȗ
#undef ȗ
#undef Ș
#undef ș
#undef Ț
#undef ț
#undef Ȝ
#undef ȝ
#undef Ȟ
#undef ȟ
#undef Ƞ
#undef ȡ
#undef Ȣ
#undef ȣ
#undef Ȥ
#undef ȥ
#undef Ȧ
#undef ȧ
#undef Ȩ
#undef ȩ
#undef Ȫ
#undef ȫ
#undef Ȭ
#undef ȭ
#undef Ȯ
#undef ȯ
#undef Ȱ
#undef ȱ
#undef Ȳ
#undef ȳ
#undef ȴ
#undef ȵ
#undef ȶ
#undef ȷ
#undef ȸ
#undef ȹ
#undef Ⱥ
#undef Ȼ
#undef ȼ
#undef Ƚ
#undef Ⱦ
#undef ȿ
#undef ɀ
#undef Ɂ
#undef ɂ
#undef Ƀ
#undef Ʉ
#undef Ʌ
#undef Ɇ
#undef ɇ
#undef Ɉ
#undef ɉ
#undef Ɋ
#undef ɋ
#undef Ɍ
#undef ɍ
#undef Ɏ
#undef ɏ
#undef ɐ
#undef ɑ
#undef ɒ
#undef ɓ
#undef ɔ
#undef ɕ
#undef ɖ
#undef ɗ
#undef ɘ
#undef ə
#undef ɚ
#undef ɛ
#undef ɜ
#undef ɝ
#undef ɞ
#undef ɟ
#undef ɠ
#undef ɡ
#undef ɢ
#undef ɣ
#undef ɤ
#undef ɥ
#undef ɦ
#undef ɧ
#undef ɨ
#undef ɩ
#undef ɪ
#undef ɫ
#undef ɬ
#undef ɭ
#undef ɮ
#undef ɯ
#undef ɰ
#undef ɱ
#undef ɲ
#undef ɳ
#undef ɴ
#undef ɵ
#undef ɶ
#undef ɷ
#undef ɸ
#undef ɹ
#undef ɺ
#undef ɻ
#undef ɼ
#undef ɽ
#undef ɾ
#undef ɿ
#undef ʀ
#undef ʁ
#undef ʂ
#undef ʃ
#undef ʄ
#undef ʅ
#undef ʆ
#undef ʇ
#undef ʈ
#undef ʉ
#undef ʊ
#undef ʋ
#undef ʌ
#undef ʍ
#undef ʎ
#undef ʏ
#undef ʐ
#undef ʑ
#undef ʒ
#undef ʓ
#undef ʔ
#undef ʕ
#undef ʖ
#undef ʗ
#undef ʘ
#undef ʙ
#undef ʚ
#undef ʛ
#undef ʜ
#undef ʝ
#undef ʞ
#undef ʟ
#undef ʠ
#undef ʡ
#undef ʢ
#undef ʣ
#undef ʤ
#undef ʥ
#undef ʦ
#undef ʧ
#undef ʨ
#undef ʩ
#undef ʪ
#undef ʫ
#undef ʬ
#undef ʭ
#undef ʮ
#undef ʯ
#undef ʰ
#undef ʱ
#undef ʲ
#undef ʳ
#undef ʴ
#undef ʵ
#undef ʶ
#undef ʷ
#undef ʸ
#undef ʹ
#undef ʺ
#undef ʻ
#undef ʼ
#undef ʽ
#undef ʾ
#undef ʿ
#undef ˀ
#undef ˁ
#undef ˆ
#undef ˇ
#undef ˈ
#undef ˉ
#undef ˊ
#undef ˋ
#undef ˌ
#undef ˍ
#undef ˎ
#undef ˏ
#undef ː
#undef ˑ
#undef ˠ
#undef ˡ
#undef ˢ
#undef ˣ
#undef ˤ
#undef ˬ
#undef ˮ
#undef Ͱ
#undef ͱ
#undef Ͳ
#undef ͳ
#undef Ͷ
#undef ͷ
#undef ͻ
#undef ͼ
#undef ͽ
#undef Ϳ
#undef Ά
#undef Έ
#undef Ή
#undef Ί
#undef Ό
#undef Ύ
#undef Ώ
#undef ΐ
#undef Α
#undef Β
#undef Γ
#undef Δ
#undef Ε
#undef Ζ
#undef Η
#undef Θ
#undef Ι
#undef Κ
#undef Λ
#undef Μ
#undef Ν
#undef Ξ
#undef Ο
#undef Π
#undef Ρ
#undef Σ
#undef Τ
#undef Υ
#undef Φ
#undef Χ
#undef Ψ
#undef Ω
#undef Ϊ
#undef Ϋ
#undef ά
#undef έ
#undef ή
#undef ί
#undef ΰ
#undef α
#undef β
#undef γ
#undef δ
#undef ε
#undef ζ
#undef η
#undef θ
#undef ι
#undef κ
#undef λ
#undef μ
#undef ν
#undef ξ
#undef ο
#undef π
#undef ρ
#undef ς
#undef σ
#undef τ
#undef υ
#undef φ
#undef χ
#undef ψ
#undef ω
#undef ϊ
#undef ϋ
#undef ό
#undef ύ
#undef ώ
#undef Ϗ
#undef ϐ
#undef ϑ
#undef ϒ
#undef ϓ
#undef ϔ
#undef ϕ
#undef ϖ
#undef ϗ
#undef Ϙ
#undef ϙ
#undef Ϛ
#undef ϛ
#undef Ϝ
#undef ϝ
#undef ϟ
#undef Ϡ
#undef ϡ
#undef Ϣ
#undef ϣ
#undef Ϥ
#undef ϥ
#undef Ϧ
#undef ϧ
#undef Ϩ
#undef ϩ
#undef Ϫ
#undef ϫ
#undef Ϭ
#undef ϭ
#undef Ϯ
#undef ϯ
#undef ϰ
#undef ϱ
#undef ϲ
#undef ϳ
#undef ϴ
#undef ϵ
#undef Ϸ
#undef ϸ
#undef Ϲ
#undef Ϻ
#undef ϻ
#undef ϼ
#undef Ͻ
#undef Ͼ
#undef Ͽ
#undef Ѐ
#undef Ё
#undef Ђ
#undef Ѓ
#undef Є
#undef Ѕ
#undef І
#undef Ї
#undef Ј
#undef Љ
#undef Њ
#undef Ћ
#undef Ќ
#undef Ѝ
#undef Ў
#undef Џ
#undef А
#undef Б
#undef В
#undef Г
#undef Д
#undef Е
#undef Ж
#undef З
#undef И
#undef Й
#undef К
#undef Л
#undef М
#undef Н
#undef О
#undef П
#undef Р
#undef С
#undef Т
#undef У
#undef Ф
#undef Х
#undef Ц
#undef Ч
#undef Ш
#undef Щ
#undef Ъ
#undef Ы
#undef Ь
#undef Э
#undef Ю
#undef Я
#undef а
#undef б
#undef в
#undef г
#undef д
#undef е
#undef ж
#undef з
#undef и
#undef й
#undef к
#undef л
#undef м
#undef н
#undef о
#undef п
#undef р
#undef с
#undef т
#undef у
#undef ф
#undef х
#undef ц
#undef ч
#undef ш
#undef щ
#undef ъ
#undef ы
#undef ь
#undef э
#undef ю
#undef я
#undef ѐ
#undef ё
#undef ђ
#undef ѓ
#undef є
#undef ѕ
#undef і
#undef ї
#undef ј
#undef љ
#undef њ
#undef ћ
#undef ќ
#undef ѝ
#undef ў
#undef џ
#undef Ѡ
#undef ѡ
#undef Ѣ
#undef ѣ
#undef Ѥ
#undef ѥ
#undef Ѧ
#undef ѧ
#undef Ѩ
#undef ѩ
#undef Ѫ
#undef ѫ
#undef Ѭ
#undef ѭ
#undef Ѯ
#undef ѯ
#undef Ѱ
#undef ѱ
#undef Ѳ
#undef ѳ
#undef Ѵ
#undef ѵ
#undef Ѷ
#undef ѷ
#undef Ѹ
#undef ѹ
#undef Ѻ
#undef ѻ
#undef Ѽ
#undef ѽ
#undef Ѿ
#undef ѿ
#undef Ҁ
#undef ҁ
#undef Ҋ
#undef ҋ
#undef Ҍ
#undef ҍ
#undef Ҏ
#undef ҏ
#undef Ґ
#undef ґ
#undef Ғ
#undef ғ
#undef Ҕ
#undef ҕ
#undef Җ
#undef җ
#undef Ҙ
#undef ҙ
#undef Қ
#undef қ
#undef Ҝ
#undef ҝ
#undef Ҟ
#undef ҟ
#undef Ҡ
#undef ҡ
#undef Ң
#undef ң
#undef Ҥ
#undef ҥ
#undef Ҧ
#undef ҧ
#undef Ҩ
#undef ҩ
#undef Ҫ
#undef ҫ
#undef Ҭ
#undef ҭ
#undef Ү
#undef ү
#undef Ұ
#undef ұ
#undef Ҳ
#undef ҳ
#undef Ҵ
#undef ҵ
#undef Ҷ
#undef ҷ
#undef Ҹ
#undef ҹ
#undef Һ
#undef һ
#undef Ҽ
#undef ҽ
#undef Ҿ
#undef ҿ
#undef Ӏ
#undef Ӂ
#undef ӂ
#undef Ӄ
#undef ӄ
#undef Ӆ
#undef ӆ
#undef Ӈ
#undef ӈ
#undef Ӊ
#undef ӊ
#undef Ӌ
#undef ӌ
#undef Ӎ
#undef ӎ
#undef ӏ
#undef Ӑ
#undef ӑ
#undef Ӓ
#undef ӓ
#undef Ӕ
#undef ӕ
#undef Ӗ
#undef ӗ
#undef Ә
#undef ә
#undef Ӛ
#undef ӛ
#undef Ӝ
#undef ӝ
#undef Ӟ
#undef ӟ
#undef Ӡ
#undef ӡ
#undef Ӣ
#undef ӣ
#undef Ӥ
#undef ӥ
#undef Ӧ
#undef ӧ
#undef Ө
#undef ө
#undef Ӫ
#undef ӫ
#undef Ӭ
#undef ӭ
#undef Ӯ
#undef ӯ
#undef Ӱ
#undef ӱ
#undef Ӳ
#undef ӳ
#undef Ӵ
#undef ӵ
#undef Ӷ
#undef ӷ
#undef Ӹ
#undef ӹ
#undef Ӻ
#undef ӻ
#undef Ӽ
#undef ӽ
#undef Ӿ
#undef ӿ
#undef Ԁ
#undef ԁ
#undef Ԃ
#undef ԃ
#undef Ԅ
#undef ԅ
#undef Ԇ
#undef ԇ
#undef Ԉ
#undef ԉ
#undef Ԋ
#undef ԋ
#undef Ԍ
#undef ԍ
#undef Ԏ
#undef ԏ
#undef Ԑ
#undef ԑ
#undef Ԓ
#undef ԓ
#undef Ԕ
#undef ԕ
#undef Ԗ
#undef ԗ
#undef Ԙ
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
#undef Ð
#undef Ñ
#undef Ò
#undef Ó
#undef Ô
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#line 4 "fps/linear_recurrence.hpp"

#line 1 "test/fixtures/fps_io.hpp"



#line 1 "components/fast_io.hpp"


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#line 11 "components/fast_io.hpp"
#include <charconv>
#line 14 "components/fast_io.hpp"
#include <cstdio>
#line 18 "components/fast_io.hpp"
#include <string>
#include <string_view>
#line 21 "components/fast_io.hpp"
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

#line 1 "fps/fps.hpp"


#line 4 "fps/fps.hpp"

#line 1 "components/fps_io.hpp"


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#line 1 "components/fast_io_bulk.hpp"


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#line 11 "components/fast_io_bulk.hpp"
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

#line 1 "components/fps_output.hpp"


#line 4 "components/fps_output.hpp"

#line 11 "components/fps_io.hpp"
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
3,c3),low(4,c4);}À void divide16(Ã Ó*x,Ð&lo,Ð&hi,Ã Ì&k){Ô at=[x](int i)Ä{Í _mm256_loadu_si256(Ç Ò*>(x+i));};Ô both=[x](int i)Ä{Í _mm256_broadcastsi128_si256(_mm_loadu_si128(Ç Ã __m128i*>(x+i)));};Ò l É(at(0),both(8),0xF0),h É(at(8),both(4),0x0F);lo=divide(l,Æ(l,32),k);hi=divide(h,Æ(h,32),k);}inline void format(char*p,Ã Ó*x,È Ñ,Ã Ì&k){Ð dl,dh;divide16(x,dl,dh,k);for(È i=16;i<Ñ;i+=16,p+=160){Ð nl,nh;divide16(x+i,nl,nh,k);store(p,dl,dh,k);dl=nl,dh=nh;}store(p,dl,dh,k);}}inline Î È kBlock=25600;inline Î È kTextBytes=10*kBlock;}namespace fps_detail{À Á io_values(Á x){Ô p=_mm256_set1_epi32(mint::mod()),ni=_mm256_set1_epi32(998244351);auto even=_mm256_and_si256(x,_mm256_set1_epi64x(0xffffffffULL));auto odd=Æ(x,32);even=_mm256_add_epi64(even,Å(Å(even,ni),p));odd=_mm256_add_epi64(odd,Å(Å(odd,ni),p));Ô value É(Æ(even,32),odd,0xaa);Í _mm256_min_epu32(value,_mm256_sub_epi32(value,p));}inline void read_coefficients(FastInput&input,std::vector<mint>&out,È n){if(!n)Í;if(n>out.max_size()-out.size())throw
 std::length_error("fps input");prepare_fps_output(out,out.size()+n);fast_io_detail::BulkInput::read_blocks(input,n,[&](Ã unsigned*values,È Ñ){append_fps_words<false>(out,values,Ñ);});}inline void write_coefficients(FastOutput&output,Ã mint*in,È n){alignas(32)unsigned values[25600];alignas(32)char text[256000];Ô&k=Ï kConstants;for(È i=0;i<n;i+=25600){Ô Ñ=std::min<È>(25600,n-i),full=Ñ/16*16;È j=0;for(;j+8<=Ñ;j+=8){alignas(32)unsigned words[8];std::memcpy(words,in+i+j,sizeof(words));_mm256_storeu_si256(Ç Á*>(values+j),io_values(_mm256_load_si256(Ç Ò*>(words))));}for(;j<Ñ;++j)values[j]=in[i+j].val();if(full)Ï format(text,values,full,k);if(full<Ñ){alignas(32)unsigned rest[16]={};alignas(16)char last[160];std::copy_n(values+full,Ñ-full,rest);Ï format(last,rest,16,k);std::memcpy(text+10*full,last,10*(Ñ-full));}if(i+Ñ==n)text[10*Ñ-1]='\n';fast_io_detail::BulkOutput::write(output,std::string_view(text,10*Ñ));}if(!n)output.write('\n');}}
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

#line 7 "test/fixtures/fps_io.hpp"

inline fps read_fps(FastInput &input, int n) {
	fps result;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fps_detail::read_coefficients(input, result, n);
		return result;
	}
#endif
	result.resize(n);
	for (auto &x : result) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	return result;
}

inline void write_fps(FastOutput &output, const fps &f) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fps_detail::write_coefficients(output, f.data(), f.size());
		return;
	}
#endif
	for (auto x : f) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}


#line 4 "test/fps_linear_recurrence.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int d = 0; long long k = 0; input.read(d); input.read(k);
	auto a = read_fps(input, d), c = read_fps(input, d);
	output.println(linear_recurrence<mint>(a, c, k).val());
}
