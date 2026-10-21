#ifndef CP_FPS_BACKEND_HPP
#define CP_FPS_BACKEND_HPP
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
#include "../math/mint.hpp"
#define Õ std::size_t
#define Ö std::uint32_t
#define Ø std::
#define Ù const
#define Ú return
#define Û .size()
#define Ü fps_detail::
#define Ý __attribute__((target("avx2")))
#define ß _mm256_mul_epu32
#define à Arena::footprint
#define â .data()
#define ã blocked_ntt::detail::
#define ä detail::
#define å blocked_ntt::detail
#define æ size
#define ç _mm256_set1_epi32
#define è constexpr
#define é data()
#define ê _mm256_sub_epi32
#define ë detail
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
#define Ě Arena
#define ě vectorized()
#define Ĝ Ø span<Ö>
#define ĝ Ø span<Ù Ö>
#define Ğ Ø span<
#define ğ Ù Ö
#define Ġ if è(Ø is_same_v<T,Ü Mint>)
#define ġ Ù Vec
#define Ģ Ù Õ
#define ģ Ø is_same_v<
#define Ĥ even=ð(even,ß(ß(even,ni),p));odd=ð(odd,ß(ß(odd,ni),p));
#define ĥ Ø min
#define Ħ Ø vector<
#define ħ if(terms==6)update(Ø integral_constant<ý,6>{});else if(terms==5)update(Ø integral_constant<ý,5>{});else if(terms==4)update(Ø integral_constant<ý,4>{});else if(terms==3)update(Ø integral_constant<ý,3>{});else if(terms==2)update(Ø integral_constant<ý,2>{});else update(Ø integral_constant<ý,1>{});
#define Ĩ ;Ù
#define ĩ Ø countr_zero
#define Ī Mint
#define ī unsigned
#define Ĭ arena.take
#define ĭ Vec
#define Į ė i
#define į out
#define İ columns
#define ı forward
#define ĳ _mm256_blend_epi32
#define Ĵ value
#define ĵ Ø copy_n
#define Ķ tables
#define ķ roots
#define ĸ scratch
#define Ĺ if è
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
#define Ű Ü ę
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
#define ƀ Ĺ(!ģ T,Ü Ī>)
#define Ɓ transform
#define Ƃ [j]
#define ƃ .end()
#define Ƅ scratch_words
#define ƅ true>
#define Ɔ Ø move
#define Ƈ true
#define ƈ Ĕ=
#define Ɖ Ø is_trivially_copyable_v<
#define Ɗ t.inverse_product
#define ƌ error
#define ƍ shift
#define Ǝ bool
#define Ə static_assert
#define Ɛ roots_
#define Ƒ _mm256_shuffle_epi32
#define ƒ twice
#define Ɠ =0 ø
#define Ɣ ğ*
#define ƕ frequency
#define Ɩ products
#define Ɨ [[gnu::noinline]]
#define ƙ Ę O=Ü Operations<T>
#define ƚ prepare_fps_output
#define ƛ sizeof
#define Ɯ struct
#define Ɲ Ù U*
#define ƞ ;i+=8
#define Ɵ Ę ā ë
#define Ơ half
#define ơ terms
#define Ƣ assert
#define ƣ root_quotients
#define Ƥ Ĩ Factor
#define ƥ kernels::
#define Ʀ fixed_mul8
#define Ƨ +8*
#define ƨ Half::kUpper
#define Ʃ Half::kLower
#define ƪ target
#define ƫ add
#define Ƭ [k]
#define ƭ Bottom
#define Ʈ Factor
#define Ư ::č
#define ư a Û
#define Ʊ [2*i+1]
#define Ʋ ntt_inverse
#define Ƴ values
#define ƴ Layout::
#define Ƶ Ù Č&t
#define ƶ Ħ Ī>
#define Ʒ â+
#define Ƹ plan.
#define ƹ [1]
#define ƺ division_detail
#define ƻ base
#define Ƽ ;i+=
#define ƽ Ģ h=nv/
#define ƾ export_work
#define ƿ Õ n
#define ǀ deg
#define ǁ output
#define ǂ ForwardBottom
#define Ǆ Operations<T>
#define ǅ O::
#define ǆ Recursion
#define Ǉ long long
#define ǈ Õ j Ē j<
#define ǉ tree
#define Ǌ =e ø(ö 8 Į){I8 z=œ(Ļ+8-i),r=ç(ļ Ĕ);e=ð(e,ß(z,r));o=ð(o,ß(ñ(z,32),r));}I8 ni=ç(998244351);e=ð(e,ß(ß(e,ni),p));o=ð(o,ß(ß(o,ni),p));x=ĳ(ñ(e,32),o,0xaa);x=ą
#define ǋ Matrix
#define ǌ Ĩ ł
#define ǎ ł update=[&](ł number){è ý þ=decltype(number)::Ĵ ĺ(j==0)ǅ Ą accumulate_products<þ,ƅ(č(ƌ),a,b,î);ŝ ǅ Ą accumulate_products<þ,Ś>(č(ƌ),a,b,î);}ĺ(ơ==7)update(Ø integral_constant<ý,7>{});ŝ ħ
#define Ǐ ċ è Ǝ kForward=
#define Ǒ Ɯ Factors{ĭ w,q;};ò ĭ ŭ(ĭ x,Ù Factors&w){ġ Ŧ=ñ(ß(x,w.q),32);ġ odd=ß(ñ(x,32),ñ(w.q,32));ġ q=ĳ(Ŧ,odd,0xAA)Ū ê(_mm256_mullo_epi32(x,w.w),_mm256_mullo_epi32(q,ĕ(kP)));}ò
#define ǒ è ł kInverseScales=[]{Ø array<Ø array<Ö,ì kMaxLog+1>,2>s{}ø(ý lg=3;lg<=ì kMaxLog ė lg){s Ŭ[lg]=ã power(Ö(1)<<(lg-3),kP-2);s ƹ[lg]=ã Đ(s Ŭ[lg],kR);}Ú s;}();ò
#define Ǔ LaneLayout::
#define ǔ inverse_columns
#define Ǖ ø(Õ c Ē c<h;c+=2*width)
#define ǖ StandardLayout
#define Ǘ T::mod()-(T::mod()-1)/
#define ǘ diff
#define Ǚ stack
#define ǚ Ņ Ù
#define Ǜ pairs
#define ǜ Ø array<
#define ǝ ĳ(ñ(Ŧ,32),odd,0xAA)
#define Ǟ derivative
#define ǟ ;while
#define Ǡ work
#define ǡ ĺ(ĩ(nv)%2==0)
#define Ǣ Ĉ ŕ()(Ņ į,Õ þ,Õ ù)Ù
#define ǣ Ù Ī
#define Ǥ cache
#define ǥ [2]
#define Ǧ ą(x,ê(x,p))
#define ǧ ý n
#define Ǩ transformed
#define ǩ Ø abort()
#define Ǫ transform_
#define ǫ Ĩ Ĝ
#define Ǭ Ĩ Č t(ŋ,lg);ł
#define ǭ Ĥ I8 Ĵ=ĳ(ñ(Ŧ,32),odd,0xaa);Ĵ=ą(Ĵ,ê(Ĵ,pp));
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
#define Ǹ ò ĭ
#define ǹ power
#define Ǻ [j+h]
#define ǻ Ø numeric_limits<double>::infinity()
#define Ǽ Õ(1)<<
#define ǽ leaf_product
#define Ǿ blocks
#define ǿ =_mm256_unpackhi_epi64
#define Ȁ =_mm256_unpackhi_epi32
#define ȁ ţ.level
#define Ȃ prepare(ù,Ļ Ŭ)ø(ǈ þ ų,į+=32)
#define ȃ fixed_butterfly8<ć
#define Ȅ .val()
#define ȅ ;Ę ntt::ä
#define Ȇ ;t.ı
#define ȇ odd
#define Ȉ pointwise
#define ȉ Half::kBoth
#define Ȋ Ĩ ý
#define ȋ Tables
#define Ȍ kInverseScales
#define ȍ level
#define Ȏ .release(mark)
#define ȏ &&Ű ě
#define Ȑ count_
#define ȑ _mm256_mullo_epi32
#define Ȓ .ù
#define ȓ T(0)
#define Ȕ ý j Ē j<
#define ȕ Õ k
#define Ȗ odd_lanes
#define ȗ composition_poly
#define Ș InverseProductBottom
#define ș /16;Ɛ=Ĭ(2*n8)â;Ø fill_n(Ɛ,2*n8,0u);ō=Ɛ+n8;ã build_table(Ɛ,entries,ã kRoots Ŭ);ã build_table(ō,entries,ã kRoots ƹ)
#define Ț Ĉ ŕ()(Ņ a,Õ þ,Õ ù)Ù
#define ț õ ĭ*>
#define Ȝ ;i<n Į
#define ȝ Ʒ i
#define Ȟ ė k
#define ȟ leaves
#define Ƞ Ù ý
#define ȡ Õ þ
#define Ȣ ;Ģ
#define ȣ last
#define Ȥ TreeTransform
#define ȥ ProductBottom
#define Ȧ Ż 0 ø
#define ȧ Ū ă;
#define Ȩ Ņ į
#define ȩ __attribute__((target("avx2"),always_inline))
#define Ȫ Ø copy
#define ȫ kVectors
#define Ȭ PolynomialHalfGCD
#define ȭ identity
#define Ȯ O Ư
#define ȯ index
#define Ȱ t.ķ()
#define ȱ ġ ni=ĕ(ì ƥ kNI)
#define Ȳ ø(Õ Ń)
#define ȳ Ć(Õ j=c+width;j<c+2*width ų)v Ƃ=v Ǻ=
#define ȴ sizes
#define ȵ inverse_scratch
#define ȶ Ȣ n1 ǥ ǥ={{lo,mid},{mid,hi}}ø(ý side Ē side<2 ė side)
#define ȷ Ü NTT t(æ);Ű CompositionWords storage(ú ī ý*>(::ŕ new[](8*æ*ƛ(ī ý),Ø align_val_t(64))));ł*p=storage.get()
#define ȸ g Û
#define ȹ ;ć Ř(î);ntt_half(ć)Ĩ Word norm=Ǘ î;Ɓ(1).ŧ(č(ć),î,norm);ł storage=Ü allocate_fps_words((2*þ-1)*k4);Word*
#define Ⱥ public:
#define Ȼ Ď<T>
#define ȼ Value
#define Ƚ kInverse
#define Ⱦ private:
#define ȿ Ȣ m=projection_length(n)Ȋ lg=ĩ(m)Ĩ ȋ Ķ(ĸ Ȓ(ȋ Ư(m)),m);Carve carve(ĸ Ė(à(ȋ Ư(m))))ǫ q=carve ņ(4*m)
#define ɀ allocate_fps_words
#define Ɂ qs[8]ø(Ȕ 8 ų){ý k=i/8+j;ws Ƃ=(k&1)?mod-ķ[k/2]:ķ[k/2];qs Ƃ=(k&1)?~ƣ[k/2]:ƣ[k/2];}inverse_leaf_products8<8
#define ɂ _mm256_loadu_si256
#define Ƀ Ī::mod()
#define Ʉ kComposeBase
#define Ʌ half_residual
#define Ɇ inverse_with
#define ɇ forward_upper
#define Ɉ InverseBottom
#define ɉ repeat_index
#define Ɋ [j+3*h]
#define ɋ [j+2*h]
#define Ɍ if(n>=8&&ě)
#define ɍ ĭ x
#define Ɏ Source
#define ɏ mul8
#define ɐ Ĝ į
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
#define ɞ ǅ trim
#define ɟ ĕ(kP)
#define ɠ Č Ư(lg)+à
#define ɡ kChains
#define ɢ QuotientPlan
#define ɣ log_block(n)
#define ɤ kLanes
#define ɥ Window Ļ ǥ
#define ɦ _mm256_castsi256_si128
#define ɧ t.ô()
#define ɨ a Ĕ
#define ɩ >=8&&ě
#define ɪ ::word
#define ɫ ø(ö 4 Į)
#define ɬ Ø bit_ceil
#define ɭ bump ņ(ǯ)
#define ɮ Õ i
#define ɯ power_projection
#define ɰ step
#define ɱ <=(1<<23)
#define ɲ available
#define ɳ fixed_mul
#define ɴ ;Ŧ=ð(Ŧ,ß(x,y));ȇ=ð(ȇ,ß(ñ(x,32),ñ(y,32)));
#define ɵ Ø align_val_t(64)
#define ɶ log_length
#define ɷ radix4
#define ɸ difference
#define ɹ ß(ñ(x,32),ñ(y,32))
#define ɺ Ù V&w=ć?ô:ķ Ĩ V&
#define ɻ _mm256_unpacklo_epi64
#define ɼ rest
#define ɽ word
#define ɾ add8
#define ɿ è Õ
#define ʀ ã Đ
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
#define ʋ ø(ǈ h ų)
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
#define ʖ =ç
#define ʗ low
#define ʘ Ŋ<Õ>
#define ʙ Ą<Ǝ kNegative>[[gnu::noinline,gnu::flatten]]
#define ʚ ý v Ē v<
#define ʛ node
#define ʜ prefix
#define ʝ from
#define ʞ ŏ(7,6,5,4,3,2,1,0)
#define ʟ storage
#define ʠ Ĺ(ƭ::kForward)
#define ʡ ĥ<ý>
#define ʢ Ć(ý k Ē k<4 Ȟ)
#define ʣ ą(x,ê(x,pp))
#define ʤ ñ(x,32)
#define ʥ â Ƨ
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
#define ʰ a â
#define ʱ .progress
#define ʲ ;i+8<=
#define ʳ +2*
#define ʴ Ȓ(m)
#define ʵ Č::kMinLog
#define ʶ ȕ=1;k<
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
#define ˉ Ƃ=
#define ˊ Ę ā å;ğ ŧ=inverse_scale
#define ˋ ý k Ē k<
#define ˌ Ý Ŵ
#define ˍ montgomery_sums
#define ˎ evaluation_poly
#define ˏ constant
#define ː ni ʖ(998244351)
#define ˑ I8 p ʖ(mod)
#define ˠ dst
#define ˡ long_division
#define ˢ Ğ ǣ>
#define ˣ composition_forward_y
#define ˤ ;Ę ǅ
#define ˬ Ő T>
#define ˮ ĉ í
#define Ͱ prefix_
#define ͱ CompositionBottom
#define Ͳ _mm256_srli_si256
#define ͳ I8 x=
#define Ͷ [&](ł ɷ,ʍ,Õ q,ġ*d,ġ*)
#define ͷ Ȣ mid=lo+(hi-lo)/2;ǚ left=free_;ǚ l8=left Ƨ(mid-lo);free_=l8 Ƨ(hi-mid);
#define ͻ t ƈ _mm256_unpacklo_epi32(r Ĕ,r[i+1])
#define ͼ ;Ę
#define ͽ size_(i6 Û){}Õ mark()Ù{Ú top_;}Ĉ release(Õ mark){top_=mark;}Ņ ś(ƿ)
#define Ϳ Ⱥ explicit
#define Ά Ə(ƛ(Ī)==ƛ(Ö)&&Ɖ Ī>)
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
#define Θ =Ĭ(m)ø(Ů n Į)f ƈ ĥ(ɨ.v,ɨ.v-Ƀ)ø(Ů m Į)g ƈ ĥ(b Ĕ.v,b Ĕ.v-Ƀ)
#define Ι ƽ 4;ʠ forward_top4(in,v,h,Ɛ)Ȳ ő(a ʥ t*h,h,t);Ĺ(ƭ::Ƚ)inverse_top4
#define Κ Ć(Õ Ń)
#define Λ õ Ø uintptr_t>
#define Μ lead
#define Ν DivisionPlan
#define Ξ kSqrtBase
#define Ο source(in,ƍ,ǲ)
#define Π Half ǁ
#define Ρ inverse_partial8
#define Σ small8<
#define Τ Group
#define Υ [i+1]
#define Φ ;scaled_leaves<kNegative>
#define Χ mid=ǉ.split(lo,hi)
#define Ψ _mm256_storeu_si256
#define Ω Ȣ nv=ư/8;ł*v=ț(ʰ)ǡ
#define Ϊ ǀ=ó(ǰ);Ƣ(ǀ>=0)ĺ(ǀ==0)Ú{};Ƣ
#define Ϋ Ű Ą accumulate_wide<þ
#define ά ą(y,ê(y,p))
#define έ Ą<Ǝ
#define ή ý î
#define ί Ù ȋ&Ķ
#define ΰ product_
#define α reduce_sums(Ŧ Ƭ,ȇ Ƭ)
#define β exp_length(n)
#define γ visit(a Ƨ t*h,h,4*k+t);
#define δ capacity
#define ε ==0
#define ζ b Û
#define η .emplace_back
#define θ ::raw
#define ι ;ġ
#define κ Ż 1;i<
#define λ divisor
#define μ undo_montgomery
#define ν row
#define ξ mod
#define ο ƌ(n);a Ř(n);ntt(a);inverse_extend(h,a,Ǥ);Ǥ=h;Ǥ Ř(n);ntt(Ǥ);
#define π ŗ(a+j+i,x0);ŗ(a+j+ď+i,x1);ŗ(a+j ʳ ď+i,x2);ŗ(a+j+3*ď+i,x3);
#define ρ mid
#define ς full=ĥ(n,Ǳ>0?Ǳ-1:0)/8*8;Indices ȯ(1)ø(Ů full ƞ,ȯ.next())ū
#define σ _mm256_cmpgt_epi32
#define τ inverse_h1
#define υ slot
#define φ rows
#define χ second
#define ψ convolution_stage8
#define ω left
#define ϊ stage_pair
#define ϋ a Ƨ
#define ό inverse_cube_roots
#define ύ ;ɓ q=Ǥ+(i-1)*k4;ĵ(č(g)+Ų-ŀ,ŀ,q);ʒ(q,î);Ű scale_normalize
#define ώ ;a.trim();q.trim()Ū{Ɔ(q),Ɔ(a)};
#define Ϗ ǘ(amb,cmd)
#define ϐ ƫ(amb,cmd)
#define ϑ ƫ(ab,cd)
#define ϒ ī ý
#define ϓ ý æ=1 ǟ(æ<ǀ)æ<<=1
#define ϔ d_deg
#define ϕ twice_mod
#define ϖ visit
#define ϗ InverseProductSumBottom
#define Ϙ leaf_weight
#define ϙ table_words
#define Ϛ stage_pair8
#define ϛ Ƨ t
#define ϝ ;I8 z=œ(a Ƭ+Ų+i);Ŧ=ð(Ŧ,ß(z,x));ȇ=ð(ȇ,ß(ñ(z,32),ʤ));
#define Ϟ Ȕ 2 ų
#define ϟ Ȓ(2*m)
#define Ϡ ǖ::î(Ö(m8))
#define ϡ ƅ(a,n/4,n,k);
#define Ϣ Ő ƭ>Ĉ
#define ϣ ĭ f ɛ
#define Ϥ ā ë
#define ϥ ó(ǰ)
#define Ϧ Ù ƭ&b9
#define ϧ Ø has_single_bit
#define Ϩ Õ h
#define ϩ Õ c Ē c<ɡ ė c
#define Ϫ inverse_start_
#define ϫ InterpolationTree
#define Ϭ values_
#define ϭ coefficient_value
#define Ϯ kDivisionTerms
#define ϯ inverse_
#define ϰ from_lower_source
#define ϱ kExpBase
#define ϲ first_touched_
#define ϳ storage_
#define ϴ kModulus
#define ϵ break;
#define Ϸ __m256i
#define ϸ b Ĕ
#define Ϲ %[a]
#define Ϻ inverse_twists
#define ϻ ;case
#define ϼ ;i<
#define Ͻ full=g6/8*8;Indices ȯ(ù+1)ø(Õ c Ē c<full;c+=8,ȯ.next())
#define Ͼ kP-
#define Ͽ sums
#define Ѐ [3]
#define Ё 2*æ
#define Ђ ;ʒ(ƌ);Ȉ(ƌ,ƌ,ć,î);Ʋ(ƌ);
#define Ѓ =[&s](ɍ)Ý{Ú ń(ŭ(x,s),kP);}
#define Є Ĝ ĸ
#define Ѕ ß(ñ(a,32),ñ(b,32))
#define І Õ lo
#define Ї append_division_words
#define Ј k5 Û
#define Љ _mm256_cvtepu32_epi64
#define Њ ;Ĺ
#define Ћ {x1=ą(x1,ê(x1,p));x2=ą(x2,ê(x2,p));x3=ą(x3,ê(x3,p));}
#define Ќ ǘ(ab,cd)
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
#define К Ȧ(ʲ n ƞ)
#define Л Ǚ ņ
#define М Ɣ ķ
#define Н ĺ(ǀ<0)ǀ=
#define О ĝ f
#define П ĕ(2*kP)
#define Р Ć(ǈ h ų)
#define С Ć(ý t Ē t<8 ė t)
#define Т Ć(ö 4 Į)
#define У ø(ʶ 4 Ȟ)
#define Ф Coefficient
#define Х touched_
#define Ц join_windows_strided
#define Ч =ĥ
#define Ш {Ú ĥ(x.v,x.v-Ƀ);}
#define Щ ý Ų=ʁ?n:0 ø(ˋ þ Ȟ)accumulate(ˠ,a Ƭ+Ų,b Ƭ+Ų,c Ƭ+Ų,n,ʁ);
#define Ъ Ɣ ȟ=a+32*g ι f ɛ={Ō(ȟ),Ō(ȟ+8),Ō(ȟ+16),Ō(ȟ+24)};ʿ
#define Ы Ĩ Node ʛ{ƴ ʂ(ω.ũ,l8.ũ),ƴ lead_product(ω.Μ,l8.Μ)}
#define Ь Factors entries(Ɣ n8,ȕ){Ɣ e=n8+υ(k)Ū{Ō(e),Ō(e+8)};}
#define Э ;ý æ=1;Ħ ƻ>ƕ;ł ǉ=Ü Ώ<T>::build(xs,æ,ŀ,Ƈ,&ƕ)
#define Ю ǌ fold=[r](ĭ s){Ú ð(ß(ñ(s,32),r),ĳ(s,đ,0xAA));}
#define Я +à
#define а Ū;
#define б Ģ m8=(m+7)/8*8
#define в mark=Ǚ.mark();
#define г o0 Ѐ={{tc Ѐ,tc Ѐ},{tc ƹ,ƒ Ŭ},{tc ǥ,ƒ ǥ}};t.Ŗ
#define д first_transform
#define е scaled
#define ж powers
#define з r[i+4]=Ł(u Ĕ,u[i+4],0x31);
#define и >=0;--
#define й ;Ě ŋ
#define к =Ĭ(n)
#define л ƶ&į
#define м ŝ if
#define н ĭ Ŧ
#define о ī Ǉ
#define п Õ hi
#define р _mm256_castsi256_ps
#define с order
#define т lanes
#define у ==1
#define ф =Ĭ
#define х ļ(p,č,u);ǚ q=Л(ʎ);t_.ı({p+č-ʎ,ʎ},0,{q,ʎ});
#define ц Ĩ ProjectionBottom b9{&Ķ,p â,q â,p â};ğ f8=ã power(kP/2+1,31);Ö c=1 ø(ý s=2;s+3<lg ė s){Ģ ď=2*(m>>s),rows=Õ(1)<<s;Ģ pad=Õ(ĩ(ď/16))ĺ(s==2){forward_pruned(p,rows*ď,pad,Ķ,ǂ{Ķ.ķ});forward_pruned(q,(rows+1)*ď,pad,Ķ,b9);}if(s+4<lg){Ģ ŀ=ď/16;ğ next=ã Đ(ã Đ(ã Đ(c,c),f8),Ö(ŀ));ğ p8=kInverseScales Ŭ[s+4];Doubling<ForwardBottom>(p,ŀ,Ķ,ForwardBottom{Ķ.ķ}).run(p8,0);Doubling<ProjectionBottom>(q,ŀ,Ķ,b9).run(p8,next+next>=kP?next+next-kP:next+next);c=next;}ŝ{ğ g8=ã power(c,kP-2);ğ ŧ=ã Đ(graeffe_scale(lg+1),ã Đ(g8,g8));inverse_pruned(q.ù(2*m),pad-1,Ķ,InverseBottom{Ķ.ô,q â},ŧ);inverse_pruned(p.ù(2*m),pad-1,Ķ,InverseBottom{Ķ.ô,p â},ŧ);next_level(q,ď/2,2*rows);}}projection_last_levels(t,Ķ,q,p,w,a);
#define ч Ė(m,m)
#define ш ú Ĉ*>
#define щ ø(ǈ nv ų)
#define ъ ö 2 Į
#define ы Ù PairWeights ş(*Ķ,k);ĭ
#define ь Ø swap
#define э middle_lanes
#define ю memory_
#define я Factors
#define ѐ prepare
#define ё fold
#define ђ inv
#define ѓ Ƭ=
#define є į+
#define ѕ short_len
#define і F r(ˈ);r
#define ї ;ŝ
#define ј ν ǥ Ƃ
#define љ ˋ þ Ȟ
#define њ ʖ(ƒ)
#define ћ Ť Ö
#define ќ ĺ(x>=ƒ)x-=ƒ
#define ѝ Ķ.ô
#define ў ĭ*Ù end=a+h;
#define џ Ħ T>
#define Ѡ ý ǀ=-1
#define ѡ ý(ư)
#define Ѣ top_store_
#define ѣ PointSlots
#define Ѥ pointwise_products
#define ѥ mark=ʄ.mark()
#define Ѧ tails
#define ѧ subtrees
#define Ѩ inverse_direct
#define ѩ _mm256_store_si256
#define Ѫ chunk
#define ѫ prepare_sparse
#define Ѭ F ă(ǀ)
#define ѭ :Ú f.Ą ŕ()<
#define Ѯ mul
#define ѯ [&]
#define Ѱ [t]
#define ѱ [[gnu::flatten]]Ĉ
#define Ѳ [2*ʛ+1]
#define ѳ cube_quotients
#define Ѵ ;ő(ʰ,h,0);ő(a ʥ h,h,1)
#define ѵ ;ł
#define Ѷ {x1=Ʀ(x1,r0,q0);x2=Ʀ(x2,r,rq);x3=Ʀ(x3,r3,q3);}
#define ѷ Ĩ ȼ top=ƴ top(ʛ.ũ,ʛ.Μ,î);ƴ ʫ(c,top);ƴ
#define Ѹ q Û
#define ѹ r ƈ Ł(u Ĕ,u[i+4],0x20)
#define Ѻ ϣ={Ō(a),Ō(a+8),Ō(a+16),Ō(a+24)};forward_h1
#define ѻ Ɵ Ȣ
#define Ѽ Ę F=Ȼ
#define ѽ Ć(ö 8 Į)
#define Ѿ ä Stack&Ǚ
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
#define ҙ y=ą(y,ê(y,pp));
#define Қ if(a.ȭ)Ú b ĺ(b.ȭ)Ú a;ǋ ă;ă.ƍ=a.ƍ+b.ƍ
#define қ U*a
#define Ҝ =ŭ
#define ҝ Ƞ m=ó(ķ Û)Ĩ T ɰ=T(3).pow((T::mod()-1)/(2*m))
#define Ҟ ǆ Ŝ(Ɛ,ō,ǂ{Ɛ})Ȣ nv=ǲ/8;ł*v=õ ĭ*>(į â)ĺ(ĩ(nv)%2==0){Ć(ǈ nv ų)v Ƃ=src(j)Ū ő(į â,nv,1);}ƽ 2 Ƥ z(Ɛ ƹ,Ɛ[9])ø(ǈ h ų){ġ a=src(j),zb=ŭ(src(j+h),z);v Ƃ=ƫ(a,zb),v[j+h]=diff(a,zb);}ő(į â,h,2);ő(į ʥ h,h,3);
#define ҟ Ɲ Ù*
#define Ҡ Ɣ b
#define ҡ Ű scale_copy
#define Ң Ņ a
#define ң ý k
#define Ҥ Ú ă;
#define ҥ Ù ƻ&
#define Ҧ Ù ƶ&
#define ҧ inverse_small
#define Ҩ pruned
#define ҩ combine_pairs
#define Ҫ cyclic
#define ҫ power_scratch
#define Ҭ fft ə
#define ҭ _mm256_permute4x64_epi64
#define Ү ==ȓ
#define ү +16
#define Ұ %kP
#define ұ [h3]"r"(96*h)
#define Ҳ T::mod()
#define ҳ ĭ*
#define Ҵ ;ŗ
#define ҵ ν ǥ Ĕ
#define Ҷ ˠ+i
#define ҷ ʚ 4 ė v
#define Ҹ ɚ ø
#define ҹ ǅ ź
#define Һ Ɣ a
#define һ Ī ɪ
#define Ҽ ġ x=
#define ҽ Ĝ a
#define Ҿ ę U*
#define ҿ Ć(ý Ń)
#define Ӏ ā fps_detail
#define Ӂ ý ď
#define ӂ è Ö
#define Ӄ Õ t Ē t
#define ӄ projection_first_levels
#define Ӆ N::œ
#define ӆ typename
#define Ӈ cyclic_ns
#define ӈ continue
#define Ӊ v Ǻ
#define ӊ +i*(n/4)
#define Ӌ .at0
#define ӌ *à
#define Ӎ ;Ø reverse_copy
#define ӎ ;_mm256_maskstore_epi32
#define ӏ =ʗ
#define Ӑ ʗ(ƫ(ŭ(ǘ(f0,f1),w.y),ŭ(ǘ(f2,f3),w.z)))
#define ӑ [v]
#define Ӓ Њ(!kPlaces)keep_(lo,hi,ĝ(į,w*ɖ))Ū ʛ;
#define ӓ Ͼ 2
#define Ӕ ;j+=8
#define ӕ ǆ(Ȱ,ɧ,InverseSquareBottom{Ȱ,ɧ,gt â})
#define Ӗ ǅ ntt
#define ӗ Ř(ǀ)
#define Ә ø(Ů k9 ƞ)
#define ә Õ p Ē p<
#define Ӛ factorial_
#define ӛ gcd_detail
#define Ӝ direct_limit
#define ӝ need
#define Ӟ build_levels
#define ӟ recursion_
#define Ӡ product_leaf
#define ӡ graeffe_leaf
#define Ӣ negate
#define ӣ direct
#define Ӥ inverse_top2
#define ӥ asm volatile
#define Ӧ factorials
#define ӧ polynomial
#define Ө in Ĕ.v
#define ө a ƈ
#define Ӫ [P2]"m"(k2P)
#define ӫ [h]"r"(32*h)
#define Ӭ [2*ʛ]
#define ӭ .insert
#define Ӯ .push_back
#define ӯ .get()
#define Ӱ cube_roots
#define ӱ ø(ɮ=2 ϼ ȸ Į)ђ ƈ Đ(Ͼ kP/Ö(i),ђ[kP%i]);g
#define Ӳ ;k8=Ѯ(k8,in(2*İ));U d4=ƫ(k8,k8)ø(ö 2*İ Į)
#define ӳ r ʖ(ļ Ƭ Ĕ);e ѓ ð(e Ƭ,ß(z,r));o ѓ ð
#define Ӵ free_=ю;build(1,lo,hi);descend(1,lo,hi,w);
#define ӵ nv=ǲ/8 ѵ*v=ț(į â)
#define Ӷ 2*k
#define ӷ >(a,b,k)а
#define Ӹ <Ü Ī>
#define ӹ ;ƀ Ú ǅ ƾ
#define Ӻ ;ū(є c,ń(ê(ƫ(d,ɟ),Ž(є æ+c)),kP));
#define ӻ =ś(ǯ)
#define Ӽ T(1)
#define ӽ ǭ Ĺ(þ==7)Ĵ=ą(Ĵ,ê(Ĵ,pp))Њ(!ù)Ĵ=
#define Ӿ Š(į ϛ,f Ѱ);
#define ӿ ĸ Ė
#define Ԁ į Ʒ
#define ԁ Ğ Ù
#define Ԃ ý þ
#define ԃ ò Ö
#define Ԅ Õ nv
#define ԅ evaluate_direct
#define Ԇ xs Û
#define ԇ projection_work
#define Ԉ Carve
#define ԉ forward_columns
#define Ԋ below
#define ԋ cur
#define Ԍ *ď
#define ԍ <ƅ
#define Ԏ Ў 1
#define ԏ Ć(Ů 8 Į)x ƈ Ō(q Ʒ 16*(r+i));ʈ(x);
#define Ԑ Ć(Õ r Ē r<e1;r+=8)
#define ԑ ø(ý s Ē s<ɤ ė s)
#define Ԓ fact
#define ԓ if(ǰ<ȸ)Ƕ resize(ȸ)ø(Ů ȸ Į)(ˈ)Ĕ
#define Ԕ n-1
#define ԕ ;ĵ
#define Ԗ Ć(ʍ=0,g=k*(nv/(4*h));j<nv;j+=4*h,++g)
#define ԗ ;ķ Ř(2*m)ø(ö m Į)ķ[m+i]=ķ Ĕ*ɰ;
#define Ã "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc"
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
Ӏ{Ɯ NTT{Ę T=Ī;typedef ϒ U __attribute__((may_alias))ͼ V=Ħ ϒ>;ċ è U ξ=998244353,ƒ=2*ξ,one=301989884;V ķ,ô,twists,Ϻ;V ƣ,ſ;mutable V Ӱ,ό,ѳ,ˇ;Ĉ prepare_cubes(ǧ)Ù{ý begin=Ӱ Û ĺ(begin>=n)Ú;Ӱ Ř(n);ό Ř(n);ѳ Ř(n);ˇ Ř(n)ø(ң=begin;k<n Ȟ){U x=(о)ķ Ƭ*ķ[Ӷ]%ξ;U y=(о)ô Ƭ*ô[Ӷ]%ξ;Ӱ ѓ x;ό ѓ y;ѳ ѓ ť(x);ˇ ѓ ť(y);}}ċ U ń(о x){Ú(x+(о)(U(x)*998244351u)*ξ)>>32;}ċ U Ѯ(U a,U b){Ú ń((о)a*b);}ċ U ƫ(U a,U b){U c=a+b Ū c<ƒ?c:c-ƒ;}ċ U sub(U a,U b){U c=a-b Ū c<ƒ?c:c+ƒ;}ċ U in(U a){Ú Ѯ(a,932051910);}ċ U į(U a){a=ń(a)Ū a<ξ?a:a-ξ;}ċ U ť(U r){Ú((о)r<<32)/ξ;}ċ U ɳ(U a,U r,U q){Ú a*r-U(((о)a*q)>>32)*ξ;}NTT(ý æ,Ǝ c8=Ƈ):ķ(æ,one),ô(æ,one),twists(c8?æ:0,one),Ϻ(c8?æ:0,one),ƣ(æ),ſ(æ){Ć(ý h=1;h<æ;h<<=1){U r=in(T(3).pow((ξ-1)/(4*h))Ȅ);U ir=in(T(3).pow(ξ-1-(ξ-1)/(4*h))Ȅ);U t=in(T(3).pow((ξ-1)/(2*h))Ȅ);U it=in(T(3).pow(ξ-1-(ξ-1)/(2*h))Ȅ);U a=in(ξ-(ξ-1)/h),b=a ø(ö h Į){ķ[h+i]=Ѯ(ķ Ĕ,r);ô[h+i]=Ѯ(ô Ĕ,ir)ĺ(c8)twists[h+i]=a,Ϻ[h+i]=b;a=Ѯ(a,t);b=Ѯ(b,it);}}Ć(ö æ Į){ƣ ƈ(ķ Ĕ<ξ?ķ Ĕ:ķ Ĕ-ξ)*998244351u;ſ ƈ(ô Ĕ<ξ?ô Ĕ:ô Ĕ-ξ)*998244351u;ķ ƈ į(ķ Ĕ);ô ƈ
 į(ô Ĕ)ĺ(c8)twists ƈ į(twists Ĕ),Ϻ ƈ į(Ϻ Ĕ);}}
#if defined(__x86_64__) || defined(__i386__)
ċ Ǝ ě{
#if defined(__AVX2__)
Ú Ƈ;
#else
ċ Ù Ǝ ɲ=__builtin_cpu_supports("avx2")Ū ɲ;
#endif
}Ę I8=Ϸ;Ý ċ I8 œ(Ɲ a){Ú ɂ(õ Ù I8*>(a));}ˌ ŗ(қ,I8 x){Ψ(õ I8*>(a),x);}Ý ċ I8 ɏ(I8 a,I8 b){ͳ ß(a,b)В y=Ѕ В r ʖ(998244351),p ʖ(ξ);x=ð(x,ß(ß(x,r),p));y=ð(y,ß(ß(y,r),p))Ū ĳ(ʤ,y,0xaa);}Ý ċ I8 Ʀ(I8 a,I8 r,I8 q){ͳ ñ(ß(a,q),32)В y=ß(ñ(a,32),q)В t=ĳ(x,y,0xaa)Ū ê(ȑ(a,r),ȑ(t,ç(ξ)));}Ý ċ I8 fixed_even8(I8 a,I8 r,I8 q){I8 t=ñ(ß(a,q),32)Ū _mm256_sub_epi64(ß(a,r),ß(t,ç(ξ)));}Ý ċ I8 ɾ(I8 a,I8 b){ͳ ň(a,b)Ū ą(x,ê(x,ç(ƒ)));}Ý ċ I8 sub8(I8 a,I8 b){ͳ ê(a,b)Ū ą(x,ň(x,ç(ƒ)));}Ą<Ӂ>Ý ċ I8 rates8(Ù V&Ƴ,ң){Ĺ(ď у)Ú Љ(_mm_loadu_si128(õ Ù __m128i*>(Ƴ Ʒ 4*k)))Њ(ď==2)Ú ҭ(Љ(_mm_loadl_epi64(õ Ù __m128i*>(Ƴ Ʒ Ӷ))),0x50)Њ(ď==4)Ú ç(Ƴ Ƭ);}Ą<Ӂ,Ǝ ć>Ý I8 small8(I8 z,ң)Ù{Ĺ(ď==4){if(k ε){ͳ Ł(z,z,0x00),y=Ł(z,z,0x11)Ū ĳ(ɾ(x,y),sub8(x,y),0xf0);}}ɺ quot=ć?ſ:ƣ В r=rates8<ď>(w,k),q=rates8<ď>(quot,k)Њ(ć){Ĺ(ď==2)z=Ƒ(z,0xd8)Њ(ď==4)z=Ľ(z,ŏ(0,4,1,5,2,6,3,7))В y=ñ(z,32),x=ɾ(z,y)В d=fixed_even8(sub8(z,y),r,q);z=ĳ(x,_mm256_slli_epi64(d,32),0xaa);}ŝ{Ĺ(ď у)z=Ƒ(z,0xb1)Њ(ď==2)z=Ƒ(z,0x72)Њ(ď==4)z=Ľ(z,ŏ(4,0,5,1,6,2,7,3))В y=fixed_even8(z,r,q);ͳ ĳ(ñ(
z,32),z,0xaa);y=ĳ(y,ê(ç(ƒ),_mm256_slli_epi64(y,32)),0xaa);z=ɾ(x,y);}Ĺ(ď==2)z=Ƒ(z,0xd8)Њ(ď==4)z=Ľ(z,ŏ(0,2,4,6,1,3,5,7))Ū z;}έ ć>Ý Ĉ fft8(қ,ή,ý i2=0)Ù{if(!i2)i2=î Њ(!ć){ý h=i2/2 ø(;h>=16;h>>=2)radix4_8 ə(a,h/2,î)ĺ(h==8){if(i2>=32)Ć(ö î ƞ){ͳ Д,p њ Ҵ(a+i,Ǧ);}stage8 ə(a,h,î);}}Ć(Ȕ î Ӕ){I8 z=œ(a+j)Њ(!ć)if((i2>=32)&&(__builtin_ctz((ϒ)i2)&1)){I8 p њ;z=ą(z,ê(z,p));}Ĺ(ć)z=Σ 4,ƅ(Σ 2,ƅ(Σ 1,ƅ(z,j/8),j/8),j/8)ї z=Σ 1,Ś>(Σ 2,Ś>(Σ 4,Ś>(z,j/8),j/8),j/8)Ҵ(a+j,z);}Ĺ(ć){ý h=8 ø(;4*h<=i2;h<<=2)radix4_8 ԍ(a,h,î)ĺ(h<i2)stage8 ԍ(a,h,î);}}έ ć>Ý Ĉ radix4_8(қ,Ӂ,ή)Ù{ɺ q=ć?ſ:ƣ;prepare_cubes(î/(4 Ԍ))Ĩ V&w3=ć?ό:Ӱ Ĩ V&q3s=ć?ˇ:ѳ В p њ В im ʖ(w ƹ),iq ʖ(q ƹ)ø(ý j=0,k Ē j<î;j+=4 Ԍ,++k){I8 r ʖ(w Ƭ),rq ʖ(q Ƭ)В r0 ʖ(w[Ӷ]),q0 ʖ(q[Ӷ])В r3 ʖ(w3 Ƭ),q3 ʖ(q3s Ƭ)ø(ö ď ƞ){I8 x0=œ(a+j+i),x1=œ(a+j+ď+i)В x2=œ(a+j ʳ ď+i),x3=œ(a+j+3 Ԍ+i)Њ(ć){I8 u=ɾ(x0,x1),v=sub8(x0,x1)В s=ɾ(x2,x3)В t=Ʀ(ê(ň(x2,p),x3),im,iq);x0=ɾ(u,s);x1=ň(v,t);x2=ê(ň(u,p),s);x3=ê(ň(v,p),t)ĺ(k)Ѷ ŝ Ћ}ŝ{x0=ą(x0,ê(x0,p))ĺ(!k)Ћ if(k)Ѷ I8 u=ɾ(x0,x2),v=sub8(x0,x2),s=ɾ(x1,x3)В t
=Ʀ(ê(ň(x1,p),x3),im,iq);x0=ň(u,s);x1=ê(ň(u,p),s);x2=ň(v,t);x3=ê(ň(v,p),t);}π}}}έ ć>Ý Ĉ stage8(қ,Ӂ,ή,ý ù=0)Ù{ɺ quot=ć?ſ:ƣ ĺ(ď<8){Ć(Ȕ î Ӕ){I8 z=œ(a+j)ĺ(ď у)z=Σ 1,ć>(z,j/8);м(ď==2)z=Σ 2,ć>(z,j/8)ї z=Σ 4,ć>(z,j/8)Ҵ(a+j,z);}}ŝ{Ć(ý j=0,k=ù;j<î;j+=2 Ԍ,++k){I8 r ʖ(w Ƭ),q ʖ(quot Ƭ)ø(ö ď ƞ){ͳ œ(a+j+i),y=œ(a+j+ď+i)Њ(!ć)if(k)y=Ʀ(y,r,q)В z=sub8(x,y);x=ɾ(x,y)Њ(ć)if(k)z=Ʀ(z,r,q)Ҵ(a+j+i,x)Ҵ(a+j+ď+i,z);}}}}έ ć,Ǝ lazy=Ś>ȩ Ŵ fixed_butterfly8(I8&x,I8&y,I8 r,I8 q,Ǝ unit=Ś){Ĺ(lazy&&!ć){I8 p њ;x=Ǧ;y=unit?ά:Ʀ(y,r,q)В z=ê(ň(x,p),y);x=ň(x,y);y=z;}ŝ{Ĺ(!ć)if(!unit)y=Ʀ(y,r,q)В z=sub8(x,y);x=ɾ(x,y);y=ć&&!unit?Ʀ(z,r,q):z;}}έ ć,Ǝ lazy=Ś>Ý Ĉ Ϛ(қ,Ӂ,ή,ý ù=0)Ù{ɺ q=ć?ſ:ƣ ø(ý j=0,k=ù;j<î;j+=4 Ԍ,++k){I8 r ʖ(w Ƭ),rq ʖ(q Ƭ)В r0 ʖ(w[Ӷ]),q0 ʖ(q[Ӷ])В r1 ʖ(w[Ӷ+1]),q1 ʖ(q[Ӷ+1])ø(ö ď ƞ){I8 x0=œ(a+j+i),x1=œ(a+j+ď+i)В x2=œ(a+j ʳ ď+i),x3=œ(a+j+3 Ԍ+i)Њ(ć){ȃ,lazy>(x0,x1,r0,q0,k ε);ȃ,lazy>(x2,x3,r1,q1,Ś);ȃ,lazy>(x0,x2,r,rq,k ε);ȃ,lazy>(x1,x3,r,rq,k ε);}ŝ{ȃ,lazy>(x0,x2,r,rq,k ε);ȃ,lazy>(x1,x3,r,rq,k ε);ȃ,lazy>(x0,x1,r0,q0,k ε);ȃ,lazy>(x2,
x3,r1,q1,Ś);}π}}}ˌ load_pairs(Ɲ a,I8&x,I8&y){I8 ȯ=ŏ(0,2,4,6,1,3,5,7)В u=Ľ(œ(a),ȯ),v=Ľ(œ(a+8),ȯ);x=Ł(u,v,0x20);y=Ł(u,v,0x31);}ˌ store_pairs(қ,I8 x,I8 y){I8 ȯ=ŏ(0,4,1,5,2,6,3,7)Ҵ(a,Ľ(Ł(x,y,0x20),ȯ))Ҵ(a+8,Ľ(Ł(x,y,0x31),ȯ));}ˌ scale_copy8(U*ˠ,Ɲ src,ή,U r){U l1=ť(r)В v ʖ(r),q ʖ(l1);Ȧ(ʲ î ƞ)ŗ(Ҷ,Ʀ(œ(src+i),v,q))ø(ϼ î Į)ˠ ƈ ɳ(src Ĕ,r,l1);}ˌ scale_normalize8(қ,ǧ,U r){U qr=ť(r)В v ʖ(r),q ʖ(qr),p ʖ(ξ);К{ͳ Ʀ(Д,v,q)Ҵ(a+i,Ǧ);}Ć(Ȝ){U x=ɳ(ɨ,r,qr);ө x<ξ?x:x-ξ;}}ˌ square_fold8(қ,Ɲ q,ǧ,Ǝ j1){Ć(ö n ƞ){ͳ Д,y=œ(a+n+i)В u=ɏ(œ(q+i),œ(q+i))В v=ɏ(œ(q+n+i),œ(q+n+i));x=ɾ(x,x)Ҵ(a+i,j1?sub8(x,u):ɾ(x,u))Ҵ(a+n+i,ɾ(ɾ(y,y),v));}}ˌ scale8(қ,ή,U r){I8 v ʖ(r),q ʖ(ť(r));Ȧ(ʲ î ƞ)ŗ(a+i,Ʀ(Д,v,q))ø(ϼ î Į)ө ɳ(ɨ,r,ť(r));}ˌ pair_product8(қ,Ɲ b,ή){I8 ː,i8 ʖ(ξ),p њ В ȯ=ŏ(0,2,4,6,1,3,5,7)ø(ö î ƞ){ͳ œ(b ʳ i),y=œ(b ʳ i+8);x=Ǧ;y=ά;x=ß(x,ʤ);y=ß(y,ñ(y,32));x=ð(x,ß(ß(x,ni),i8));y=ð(y,ß(ß(y,ni),i8));x=ĳ(ʤ,y,0xaa)Ҵ(a+i,Ľ(x,ȯ));}}ˌ pair_multiply8(қ,Ɲ b,ή){I8 f9=ŏ(0,0,1,1,2,2,3,3),g3=ŏ(4,4,5,5,6,6,7,7)В p њ ø(ö î ƞ){I8 r=œ(b+i),x=œ(a ʳ i),y=œ
(a ʳ i+8);x=Ǧ;y=ά;x=Ƒ(x,0xb1);y=Ƒ(y,0xb1)Ҵ(a ʳ i,ɏ(x,Ľ(r,f9)))Ҵ(a ʳ i+8,ɏ(y,Ľ(r,g3)));}}ˌ difference8(қ,Ɲ b,Ɲ c,ή){Ć(ö î ƞ)ŗ(a+i,sub8(œ(b+i),œ(c+i)));}
#endif
έ ć>Ĉ stage(қ,Ӂ,ή,ý ù=0)Ù{ɺ quot=ć?ſ:ƣ;
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){stage8<ć>(a,ď,î,ù)а}
#endif
Ć(ý j=0,k=ù;j<î;j+=2 Ԍ,++k){U r=w Ƭ,q=quot Ƭ;Ȧ(ϼ ď Į){U x=a[j+i],y=a[j+ď+i]Њ(!ć)if(k)y=ɳ(y,r,q);a[j+i]=ƫ(x,y);a[j+ď+i]=ć&&k?ɳ(sub(x,y),r,q):sub(x,y);}}}έ ć>Ĉ ϊ(қ,Ӂ,ή,ý ù=0)Ù{
#if defined(__x86_64__) || defined(__i386__)
if(ď ɩ){Ϛ<ć>(a,ď,î,ù)а}
#endif
Ĺ(ć)stage ԍ(a,ď,î,2*ù),stage ԍ(a,2 Ԍ,î,ù)ї stage ə(a,2 Ԍ,î,ù),stage ə(a,ď,î,2*ù);}έ ć>Ĉ fft(қ,ή,ý i2=0)Ù{if(!i2)i2=î;
#if defined(__x86_64__) || defined(__i386__)
if(i2 ɩ){fft8<ć>(a,î,i2)а}
#endif
Ĺ(ć){Ć(ý h=1;h<i2;h<<=1)stage ԍ(a,h,î);}ŝ{Ć(ý h=i2/2;h;h>>=1)stage ə(a,h,î);}}
#if defined(__x86_64__) || defined(__i386__)
ˌ product8(U*ˠ,Ɲ a,Ɲ b,ǧ){К{ͳ ɏ(Д,œ(b+i))Ҵ(Ҷ,x);}Ć(Ȝ)ˠ ƈ Ѯ(ɨ,ϸ);}ˌ weighted8(U*ˠ,Ɲ src,ǧ,ý ù){Ť U e2 Ҹ(Ȕ 8 ų)e2 ˉ in(Ў j)В r=œ(e2),ɰ ʖ(in(8));К{ŗ(Ҷ,ɏ(œ(src+i),r));r=ɾ(r,ɰ);}Ć(Ȝ)ˠ ƈ Ѯ(src Ĕ,in(Ў i));}ȩ ċ I8 inverse8(I8 a){I8 a2=ɏ(a,a),a3=ɏ(a2,a)В a7=ɏ(ɏ(a3,a3),a),a31=a7 ø(ý ɉ Ē ɉ<2 ė ɉ)a31=ɏ(a31,a31);a31=ɏ(a31,a3)В r=a7 ø(ý ɉ Ē ɉ<3 ė ɉ)r=ɏ(r,r);r=ɏ(r,a3);r=ɏ(r,r)ø(Ȕ 4 ų){Ć(ý ɉ Ē ɉ<5 ė ɉ)r=ɏ(r,r);r=ɏ(r,a31);}Ć(ý ɉ Ē ɉ<3 ė ɉ)r=ɏ(r,r)Ū ɏ(r,a7);}ˌ batch_inverse8(U*ˠ,Ɲ src,ǧ){if(n<16){Ć(Е)ˠ ƈ T ɪ(src Ĕ)ˆ.v а}Ԃ=n/16*16 В p=œ(src),q=œ(src+8)Ҵ(ˠ,p)Ҵ(ˠ+8,q)ø(Ż 16 ϼ þ Ƽ 16){p=ɏ(p,œ(src+i));q=ɏ(q,œ(src+i+8))Ҵ(Ҷ,p)Ҵ(Ҷ+8,q);}Ť U т[16]Ҵ(т,p)Ҵ(т+8,q)ø([[maybe_unused]]ł x:т)Ƣ(į(x)!=0)В ć=inverse8(p),j5=inverse8(q)ø(Ż þ-16;i>=16;i-=16){ŗ(Ҷ,ɏ(ć,œ(Ҷ-16)))Ҵ(Ҷ+8,ɏ(j5,œ(Ҷ-8)));ć=ɏ(ć,œ(src+i));j5=ɏ(j5,œ(src+i+8));}ŗ(ˠ,ć)Ҵ(ˠ+8,j5)ø(Ż þ Ȝ)ˠ ƈ T ɪ(src Ĕ)ˆ.v;}ˌ evaluate_leaf8(U*ˠ,Ɲ ԋ,Ɲ ǉ,ý ũ,Ɲ k5,Ԃ){ˑ,pp њ;ͳ œ(ԋ);x=ʣ;x=Ǧ;Ť U Ļ[16]={},ļ ɚ={},ӧ ɚ,Η ɚ={}Ҵ(Ļ+8,x)ԕ(ǉ,ĥ(ũ,8),ļ)В y=œ(ļ);ҙ ŗ(ļ,ά)В e=đ,o Ǌ(x,ê(x,pp))Ҵ(ӧ,Ǧ
)ĺ(þ==8)x=œ(k5)ї{ĵ(k5,þ,Η);x=œ(Η);}I8 Ĵ=đ ø(ö 8 Į)Ĵ=ɾ(ɏ(Ĵ,x),ç(ӧ Ĕ))ĺ(þ==8)ŗ(ˠ,Ĵ)ї{ŗ(Η,Ĵ)ԕ(Η,þ,ˠ);}}ȩ ċ U sum_product8(I8 a,I8 b){ˑ;a=ą(a,ê(a,p));b=ą(b,ê(b,p))В ʂ=ð(ß(a,b),Ѕ);ʂ=ð(ʂ,Ł(ʂ,ʂ,1));ʂ=ð(ʂ,Ƒ(ʂ,0x4e));U ă=ń((о)_mm_cvtsi128_si64(ɦ(ʂ)))Ū ă<ƒ?ă:ă-ƒ;}ˌ interpolate_leaf8(U*ˠ,Ɲ q,Ɲ k5,Ɲ ş){ͳ œ(k5),w=œ(ş),c ʖ(one)В ʂ=ɾ(w,Ł(w,w,1));ʂ=ɾ(ʂ,Ƒ(ʂ,0x4e));ʂ=ɾ(ʂ,Ƒ(ʂ,0xb1));ˠ[7]=Ҙ(ɦ(ʂ))ø(ң=7-1;k и k){c=ɾ(ɏ(c,x),ç(q[k+1]));ˠ ѓ sum_product8(c,w);}}ˌ evaluate_middle8(U*ω,U*l8,Ɲ ԋ,Ɲ ql,ý dl,Ɲ qr,ý dr){ˑ,pp њ,ː;Ť U Ļ[16],ļ ǥ[9]={}ø(ö 16 ƞ){ͳ œ(ԋ+i);x=ʣ Ҵ(Ļ+i,Ǧ);}Ć(ö ĥ(dl,9)Į){U x=ql Ĕ ќ ĺ(x>=ξ)x-=ξ;ļ Ŭ ƈ x;}Ć(ö ĥ(dr,9)Į){U x=qr Ĕ ќ ĺ(x>=ξ)x-=ξ;ļ ƹ ƈ x;}I8 e ǥ={},o ǥ={}ø(ö 9 Į){I8 z=œ(Ļ+8-i),high=ñ(z,32)ø(ˋ 2 Ȟ){I8 ӳ(o Ƭ,ß(high,r));}}U*ˠ ǥ={ω,l8}ø(ˋ 2 Ȟ){e ѓ ð(e Ƭ,ß(ß(e Ƭ,ni),p));o ѓ ð(o Ƭ,ß(ß(o Ƭ,ni),p));ͳ ĳ(ñ(e Ƭ,32),o Ƭ,0xaa)Ҵ(ˠ Ƭ,ʣ);}}ˌ inverse_sequence8(U*ˠ,ý ù,ǧ){Ť U g4 Ҹ(Ȕ 8 ų)g4 ˉ in(Ў j)В Ÿ=œ(g4),ɰ ʖ(in(8))В p=Ÿ Ҵ(ˠ,p)ø(Ż 8 ϼ n ƞ){Ÿ=ɾ(Ÿ,ɰ);p=ɏ(p,Ÿ)Ҵ(Ҷ,p);}I8 ć ʖ(one)ø(U e=ξ-2;e;e>>=1,p=ɏ(p,p)
)if(e&1)ć=ɏ(ć,p)ø(Ż n-8;i>=8;i-=8){ŗ(Ҷ,ɏ(ć,œ(Ҷ-8)));ć=ɏ(ć,Ÿ);Ÿ=sub8(Ÿ,ɰ);}ŗ(ˠ,ć);}ˌ normalize8(қ,ǧ){ˑ;К{ͳ Д Ҵ(a+i,Ǧ);}Ć(Ȝ)if(ɨ>=ξ)ɨ-=ξ;}
#endif
Ŵ weighted(U*ˠ,Ɲ src,ǧ,ý ù){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{weighted8(ˠ,src,n,ù)а}
#endif
Ć(Е)ˠ ƈ Ѯ(src Ĕ,in(Ў i));}Ŵ scale_normalize(қ,ǧ,U r){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{scale_normalize8(a,n,r)а}
#endif
U qr=ť(r);Ȧ(Ȝ){U x=ɳ(ɨ,r,qr);ө x<ξ?x:x-ξ;}}Ŵ square_fold(қ,Ɲ q,ǧ,Ǝ j1){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{square_fold8(a,q,n,j1)а}
#endif
Ȧ(Ȝ){U u=Ѯ(q Ĕ,q Ĕ),v=Ѯ(q[n+i],q[n+i]);U x=ƫ(ɨ,ɨ);ө j1?sub(x,u):ƫ(x,u);a[n+i]=ƫ(ƫ(a[n+i],a[n+i]),v);}}Ŵ normalize(қ,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{normalize8(a,n)а}
#endif
Ȧ(Ȝ)if(ɨ>=ξ)ɨ-=ξ;}Ŵ Ŏ(U*ˠ,Ɲ a,Ɲ b,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{product8(ˠ,a,b,n)а}
#endif
Ȧ(Ȝ)ˠ ƈ Ѯ(ɨ,ϸ);}
#if defined(__x86_64__) || defined(__i386__)
έ ć>ȩ Ĉ ψ(қ,Ӂ,ң)Ù{ɺ q=ć?ſ:ƣ В r ʖ(w Ƭ),rq ʖ(q Ƭ)ø(ö ď ƞ){ͳ Д,y=œ(a+ď+i);ȃ,ƅ(x,y,r,rq,k ε)Ҵ(a+i,x)Ҵ(a+ď+i,y);}}ȩ Ĉ leaf8(қ,Ɲ b,ң)Ù{U w=ķ[k/2],q=ƣ[k/2]ĺ(k&1)w=ξ-w,q=~q;ˑ,pp њ;ͳ œ(a),y=œ(b);x=ʣ;ҙ x=Ǧ;y=ά;Ť U Ļ[16],ļ ɚ В ʗ=Ʀ(x,ç(w),ç(q))Ҵ(Ļ,ą(ʗ,ê(ʗ,p)))Ҵ(Ļ+8,x)Ҵ(ļ,y)В e=đ,o Ǌ(x,ê(x,pp))Ҵ(a,Ǧ);}Ą<Ԃ,Ǝ χ=Ś>ȩ Ĉ inverse_leaf_products8(қ,U*b,Ɲ ws,Ɲ qs)Ù{ˑ,pp њ;Ť U Ļ[þ][16],ļ[þ]ɚ В e[þ],o[þ]ø(љ){ͳ œ(ϋ k),y=œ(b Ƨ k);x=ʣ Њ(!χ)ҙ x=Ǧ Њ(!χ)y=ά В ʗ=Ʀ(x,ç(ws Ƭ),ç(qs Ƭ))Ҵ(Ļ Ƭ,ą(ʗ,ê(ʗ,p)))Ҵ(Ļ Ƭ+8,x)Ҵ(ļ Ƭ,y)Њ(!χ)ŗ(b Ƨ k,y);e ѓ o ѓ đ;}ѽ Ć(љ){I8 z=œ(Ļ Ƭ+8-i),ӳ(o Ƭ,ß(ñ(z,32),r));}I8 ː ø(љ){ͳ e Ƭ,y=o Ƭ;x=ð(x,ß(ß(x,ni),p));y=ð(y,ß(ß(y,ni),p));x=ĳ(ʤ,y,0xaa)Ҵ(ϋ k,ʣ);}}Ý ċ U reverse_dot8(Ɲ a,Ɲ b,ǧ){ˑ,pp њ,ː В reversed=ʞ,o4=đ;Ȧ(ʲ n;){I8 Ŧ=đ,ȇ=Ŧ;ý end Ч(n/8*8,i+64)ø(ϼ end ƞ){ͳ Д,y=Ľ(œ(b-i-7),reversed);x=ʣ;ҙ x=Ǧ;y=ά ɴ}ǭ o4=ɾ(o4,Ĵ);}o4=ɾ(o4,Ł(o4,o4,1));o4=ɾ(o4,Ƒ(o4,0x4e));o4=ɾ(o4,Ƒ(o4,0xb1));U ă=Ҙ(ɦ(o4))ø(Ȝ){U x=ɨ,y=b[-i]ќ ĺ(y>=ƒ)y-=ƒ ĺ(x>=ξ)x-=ξ ĺ(y>=ξ)y-=ξ;ă=ƫ(ă,Ѯ(x,y));}Ҥ}έ ć>Ý Ĉ Ρ(қ,ǧ,ý i2=0)Ù{if(!i2)i2=n Њ(!ć){Ӂ=
i2/2 ø(;ď>=16;ď>>=2)radix4_8 ə(a,ď/2,n)ĺ(ď==8)Ć(ö n Ƽ 16)ψ ə(a+i,8,i/16);}ŝ{Ӂ=8 ø(;4 Ԍ<=n;ď<<=2)radix4_8 ԍ(a,ď,n)ĺ(ď<n)stage8 ԍ(a,ď,n);}}Ý Ĉ inverse_newton8(қ,U*b,ǧ)Ù{Ƣ(n>=64&&!(n&(Ԕ))&&n ɱ&&ó(ķ Û)>=n/16)ԕ(b,n/2,b+n/2);Ρ ə(b,n,n/2);Ρ ə(a,n)ø(ö n Ƽ 64){U ws ɚ,Ɂ>(a+i,b+i,ws,qs);}Ρ ԍ(a,n);ʇ(a,n/2,U(0));Ρ ə(a,n)ø(ö n Ƽ 64){U ws ɚ,Ɂ,ƅ(a+i,b+i,ws,qs);}Ρ ԍ(a,n);U norm=ξ-(ξ-1)/(n/8);scale8(a+n/2,n/2,ξ-(о)norm*norm%ξ);}Ą<ǧ>ȩ Ĉ ʕ(қ,U*b,ң)Ù{Ĺ(n==8)leaf8(a,b,k)ї Ĺ(n==16){ψ ə(a,n/2,k);ψ ə(b,n/2,k);ʕ<n/2>(a,b,Ӷ);ʕ<n/2>(a+n/2,b+n/2,Ӷ+1);ψ ԍ(a,n/2,k);}ŝ{Ϛ<Ś,ϡ Ϛ<Ś,ƅ(b,n/4,n,k)ɫ ʕ<n/4>(a ӊ,b ӊ,4*k+i);Ϛ<ϡ}}Ý Ĉ convolution8(қ,U*b,ǧ,ң=0)Ù{if(n==256){ʕ<256 ӷ}if(n==128){ʕ<128 ӷ}if(n==64){ʕ<64 ӷ}Ϛ<Ś,ϡ Ϛ<Ś,ƅ(b,n/4,n,k)ɫ convolution8(a ӊ,b ӊ,n/4,4*k+i);Ϛ<ϡ}
#endif
#if defined(__x86_64__) || defined(__i386__)
έ ȇ>ˌ reduce_pairs8(қ,U*b,Ɲ twist,ǧ){Ć(ö n ƞ){I8 x,y,u,v;load_pairs(a ʳ i,x,y);load_pairs(b ʳ i,u,v);x=ɏ(x,v);y=ɏ(y,u)Њ(ȇ)x=ɏ(sub8(x,y),œ(twist+i))ї x=ɾ(x,y)Ҵ(a+i,x)Ҵ(b+i,ɏ(u,v));}}ˌ combine8(қ,Ɲ b,Ɲ x,Ɲ y,ǧ){Ć(ö n ƞ)ŗ(a+i,ɾ(ɏ(Д,œ(y+i)),ɏ(œ(b+i),œ(x+i))));}
#endif
έ ȇ>Ŵ reduce_pairs(қ,U*b,Ɲ twist,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{reduce_pairs8<ȇ>(a,b,twist,n)а}
#endif
Ȧ(Ȝ){U u=Ѯ(a ɔ,b Ʊ),v=Ѯ(a Ʊ,b ɔ);ө ȇ?Ѯ(sub(u,v),twist Ĕ):ƫ(u,v);b ƈ Ѯ(b ɔ,b Ʊ);}}Ŵ җ(қ,Ɲ b,Ɲ x,Ɲ y,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{combine8(a,b,x,y,n)а}
#endif
Ȧ(Ȝ)ө ƫ(Ѯ(ɨ,y Ĕ),Ѯ(ϸ,x Ĕ));}
#if defined(__x86_64__) || defined(__i386__)
ˌ geometric8(қ,ǧ,U ù,U ɰ){Ť U e2 ɚ;U ǹ=one ø(ö 8 Į){e2 ƈ Ѯ(ù,ǹ);ǹ=Ѯ(ǹ,ɰ);}I8 r=œ(e2),l2 ʖ(ǹ);К{ŗ(a+i,ɏ(Д,r));r=ɏ(r,l2);}ŗ(e2,r)ø(ý j Ē i<n Į,++j)ө Ѯ(ɨ,e2 Ƃ);}
#endif
Ŵ geometric(қ,ǧ,U ù,U ɰ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{geometric8(a,n,ù,ɰ)а}
#endif
Ȧ(Ȝ){ө Ѯ(ɨ,ù);ù=Ѯ(ù,ɰ);}}
#if defined(__x86_64__) || defined(__i386__)
ˌ add_scaled8(қ,Ɲ b,ǧ,U r){U q=ť(r)В v ʖ(r),vq ʖ(q);К ŗ(a+i,ɾ(Д,Ʀ(œ(b+i),v,vq)))ø(Ȝ)ө ƫ(ɨ,ɳ(ϸ,r,q));}
#endif
Ŵ add_scaled(қ,Ɲ b,ǧ,U r){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{add_scaled8(a,b,n,r)а}
#endif
ý i Ē U q=ť(r)ø(Ȝ)ө ƫ(ɨ,ɳ(ϸ,r,q));}Ĉ leaf(қ,Ɲ b,ң)Ù{U w=ķ[k/2],q=ƣ[k/2]ĺ(k&1)w=ξ-w,q=~q;U Ļ[16],ļ Ҹ(ö 8 Į){U x=ɨ<ξ?ɨ:ɨ-ξ;U z=ɳ(x,w,q);Ļ ƈ z<ξ?z:z-ξ;Ļ[8+i]=x;ļ ƈ ϸ<ξ?ϸ:ϸ-ξ;}Ć(Ȕ 8 ų){о ʂ Ɠ(ö 8 Į)ʂ+=(о)Ļ[8-i+j]*ļ Ĕ;U x=ń(ʂ)ќ;a ˉ x<ξ?x:x-ξ;}}Ą<ǧ>Ĉ ʯ(қ,U*b,ң)Ù{Ĺ(n==8)leaf(a,b,k)ї Ĺ(n==16){stage ə(a,n/2,n,k);stage ə(b,n/2,n,k);ʯ<n/2>(a,b,Ӷ);ʯ<n/2>(a+n/2,b+n/2,Ӷ+1);stage ԍ(a,n/2,n,k);}ŝ{ϊ ə(a,n/4,n,k);ϊ ə(b,n/4,n,k)ɫ ʯ<n/4>(a ӊ,b ӊ,4*k+i);ϊ<ϡ}}Ĉ convolution(қ,U*b,ǧ,ң=0)Ù{
#if defined(__x86_64__) || defined(__i386__)
if(ě){convolution8(a,b,n,k)а}
#endif
if(n==256){ʯ<256 ӷ}if(n==128){ʯ<128 ӷ}if(n==64){ʯ<64 ӷ}ϊ ə(a,n/4,n,k);ϊ ə(b,n/4,n,k)ɫ convolution(a ӊ,b ӊ,n/4,4*k+i);ϊ<ϡ}
#if defined(__x86_64__) || defined(__i386__)
ˌ accumulate8(U*ˠ,Ɲ a,Ɲ b,Ɲ c,ǧ,Ǝ ʁ){Ć(ö n ƞ){ͳ œ(b+i),y=œ(c+i);x=ʁ?ê(ň(x,ç(ƒ)),y):ň(x,y)Ҵ(Ҷ,ɾ(œ(Ҷ),ɏ(Д,x)));}}
#endif
#if defined(__x86_64__) || defined(__i386__)
Ą<Ԃ>ˌ accumulate_batch8(U*ˠ,ҟ a,ҟ b,ҟ c,ǧ,Ǝ ʁ){Ə(þ>=1&&þ<=3);ˑ,pp њ,ː;ý Ų=ʁ?n:0 ø(ö n ƞ){I8 Ŧ=đ,ȇ=Ŧ;
#pragma GCC unroll 3
Ć(љ){ͳ œ(b Ƭ+Ų+i),y=œ(c Ƭ+Ų+i);x=ʁ?ê(ň(x,pp),y):ň(x,y)ϝ}ǭ ŗ(Ҷ,ɾ(œ(Ҷ),Ĵ));}}
#endif
#if defined(__x86_64__) || defined(__i386__)
Ą<Ԃ,Ǝ ù,Ǝ ů>ˌ accumulate_wide8(U*ˠ,ҟ a,ҟ b,ҟ c,ǧ,Ǝ ʁ){Ə(þ>=1&&þ<=7);ˑ,pp њ,ː;ý Ų=ʁ?n:0 ø(ö n ƞ){I8 Ŧ=đ,ȇ=Ŧ;
#pragma GCC unroll 7
Ć(љ){ͳ œ(b Ƭ+Ų+i),y=œ(c Ƭ+Ų+i);x=ʁ?ê(ň(x,ů?p:pp),y):ň(x,y)Њ(!ů)x=ʣ ϝ}ӽ ɾ(œ(Ҷ),Ĵ)Ҵ(Ҷ,Ĵ);}}
#endif
Ą<Ԃ,Ǝ ù,Ǝ ů>Ŵ accumulate_wide(U*ˠ,ҟ a,ҟ b,ҟ c,ǧ,Ǝ ʁ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{accumulate_wide8<þ,ù,ů>(ˠ,a,b,c,n,ʁ)а}
#endif
Ĺ(ù)ʇ(ˠ,n,0u);Щ}Ą<Ԃ>Ŵ accumulate_batch(U*ˠ,ҟ a,ҟ b,ҟ c,ǧ,Ǝ ʁ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{accumulate_batch8<þ>(ˠ,a,b,c,n,ʁ)а}
#endif
Щ}Ŵ accumulate(U*ˠ,Ɲ a,Ɲ b,Ɲ c,ǧ,Ǝ ʁ){
#if defined(__x86_64__) || defined(__i386__)
Ɍ{accumulate8(ˠ,a,b,c,n,ʁ)а}
#endif
Ȧ(Ȝ)ˠ ƈ ƫ(ˠ Ĕ,Ѯ(ɨ,ʁ?sub(ϸ,c Ĕ):ƫ(ϸ,c Ĕ)));}Ŵ scale_copy(U*ˠ,Ɲ src,ή,U r){
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){scale_copy8(ˠ,src,î,r)а}
#endif
U q=ť(r);Ȧ(ϼ î Į)ˠ ƈ ɳ(src Ĕ,r,q);}Ĉ ŧ(қ,ή,U r)Ù{
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){scale8(a,î,r)а}
#endif
ý i Ē U q=ť(r)ø(ϼ î Į)ө ɳ(ɨ,r,q);}Ŵ pair_product(қ,Ɲ b,ή){
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){pair_product8(a,b,î)а}
#endif
Ȧ(ϼ î Į)ө Ѯ(b ɔ,b Ʊ);}Ŵ pair_multiply(қ,Ɲ b,ή){
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){pair_multiply8(a,b,î)а}
#endif
Ȧ(ϼ î Į){U x=a ɔ;a ɔ=Ѯ(ϸ,a Ʊ);a Ʊ=Ѯ(ϸ,x);}}Ŵ ɸ(қ,Ɲ b,Ɲ c,ή){
#if defined(__x86_64__) || defined(__i386__)
if(î ɩ){difference8(a,b,c,î)а}
#endif
Ȧ(ϼ î Į)ө sub(ϸ,c Ĕ);}Ŵ ё(қ,ý İ){Ȧ(ϼ İ Į)ө a[İ+i]=ƫ(ɨ,a[İ+i]);}Ĉ project(қ,ý φ,ý İ)Ù{if(İ у){ý j Ɠ(;j<φ ų)a[2*j]=a[2*j+1]=ƫ(a[2*j],a[2*j+1])а}if(İ==2){ý j Ɠ(;j<φ ų){U*r=a+4*j;U x=ƫ(ƫ(r Ŭ,r ƹ),ƫ(r ǥ,r Ѐ));U y=ƫ(sub(r Ŭ,r ƹ),ɳ(sub(r ǥ,r Ѐ),ô ƹ,ſ ƹ));r Г ƫ(x,y);r ƹ=sub(x,y);y=ɳ(y,ķ ƹ,ƣ ƹ);r ǥ=ƫ(x,y);r Ѐ=sub(x,y);}Ú;}Ć(Ȕ φ ų){U*ν=a ʳ İ*j;fft ԍ(ν,2*İ,İ);ё(ν,İ);Ҭ(ν,2*İ,İ);}}Ĉ composition_difference(қ,Ɲ b,Ɲ c,ǧ)Ù{ɸ(a,b,c,n);}Ĉ composition_finish(U*p,Ɲ v,ý İ,U Ÿ)Ù{Ć(ö İ Į)p ƈ Ѯ(sub(ƫ(v Ĕ,v[İ+i]),ƫ(v[2*İ+i],v[3*İ+i])),Ÿ);}Ɯ CompositionDelete{Ĉ ŕ()(ϒ*p)Ù{::ŕ delete[](p,ɵ);}}ͼ CompositionWords=Ø unique_ptr<ϒ[],CompositionDelete>;Ĉ ˣ(қ,ý æ,ý İ)Ù{ý h=æ ø(;h/2>İ;h>>=2)ϊ ə(a,h/2,Ё)ĺ(h>İ)stage ə(a,h,Ё);}Ĉ double_y(қ,ý φ,ý İ,Ǝ ć)Ù{Ƞ æ=φ*İ;ý h=2*İ ø(;h<=æ/2;h<<=2){
#if defined(__x86_64__) || defined(__i386__)
if(h ɩ)radix4_8 ԍ(a,h,Ё)ї
#endif
ϊ ԍ(a,h,Ё);}if(h<=æ)stage ԍ(a,h,Ё)ø(Ȕ φ ų)ŧ(a ʳ İ*j,2*İ,(ć?Ϻ:twists)[φ+j]);}Ŵ projection_dot(U*p,Ɲ q,ǧ){Ć(Е){U x=p ɔ,y=p Ʊ,a=q ɔ,b=q Ʊ ќ ĺ(y>=ƒ)y-=ƒ ĺ(a>=ƒ)a-=ƒ ĺ(b>=ƒ)b-=ƒ;U r=ń((о)x*b+(о)y*a);p ƈ r<ƒ?r:r-ƒ;}}Ĉ ʔ{ķ.swap(ô);ƣ.swap(ſ);Ӱ.swap(ό);ѳ.swap(ˇ);}U reversion_projection(U*p,U*q,ý æ,Ɲ ş=Ƿ){ý ţ=__builtin_ctz((ϒ)æ);V saved(ţ,one);U k8=one,j9=one ø(Ż 1,İ=æ/2;İ>1 Į,İ>>=1){k8=Ѯ(Ѯ(k8,k8),in(2*İ));saved ƈ k8;}Ć(Ż ţ-1,İ=2;i и i,İ<<=1){j9=Ѯ(j9,saved Ĕ)ĺ(i)j9=Ѯ(j9,in(4*İ));}U Ÿ=T ɪ(Ѯ(j9,in(4*æ)))ˆ.v;Ҭ(q,Ё)ø(ö Ё Į){U x=q Ĕ;q ƈ ƫ(x,one);q[Ё+i]=sub(x,one);}if(ş){Ć(ö æ Į){p ƈ p[æ+i]=ş Ĕ;}ʔ;Ҭ(p,Ё,æ);ʔ ø(ö Ё Į)p[Ё+i]=sub(0,p Ĕ);projection_dot(p,q,Ё);}ŝ{Ć(ö æ Į){U x=ɳ(sub(q Ʊ,q ɔ),ķ Ĕ,ƣ Ĕ);p ƈ i<æ/2?x:sub(0,x);p[æ+i]=sub(0,p Ĕ);}}k8=one ø(ý İ=æ/2,φ=2;İ>1;İ>>=1,φ<<=1){pair_product(q,q,Ё);k8=Ѯ(k8,k8);project(q,φ,İ)ԕ(q,Ё,q ʳ æ);double_y(q ʳ æ,φ,İ,Ś)Ӳ q[Ё+i]=sub(q[Ё+i],d4);ˣ(q ʳ æ,æ,İ);ʔ;project(p,φ,İ)ø(ö Ё Į)p[Ё+i]=sub(0,p Ĕ);double_y(p ʳ æ,φ,İ,Ƈ);ˣ(p ʳ æ,æ,İ);ʔ;projection_dot(p,q,Ё);}Ć(ö æ Į)p ƈ ƫ(
p ɔ,p Ʊ);ʔ;fft ԍ(p,æ);ʔ Ū Ÿ;}Ĉ compose(U*p,Ɲ q,ý φ,ý İ,U&j9,U&k8,U*p6=Ƿ,ý ȍ=0)Ù{Ƞ æ=φ*İ ĺ(İ у){Ҭ(p,φ)ø(Ż φ-1;i и i)p ɔ=p Ʊ=p Ĕ а}CompositionWords ʟ ĺ(!p6){ý ţ=__builtin_ctz((ϒ)İ);ʟ.reset(ú ϒ*>(::ŕ new[]((4*æ ү)*ţ*ƛ(ϒ),ɵ)));p6=ʟ ӯ;}U*v=p6+(4*æ ү)*ȍ ĺ(φ у){ʇ(v+İ,İ,U(0))ø(ö İ Į)v ƈ sub(0,q Ĕ);Ҭ(v,2*İ)ø(ö 2*İ Į){U x=v Ĕ;v ƈ ƫ(x,one);v[i ʳ İ]=sub(x,one);}}ŝ{pair_product(v,q,Ё);k8=Ѯ(k8,k8);project(v,φ,İ)ԕ(v,Ё,v ʳ æ);U*ʓ=v ʳ æ;double_y(ʓ,φ,İ,Ś)Ӳ ʓ ƈ sub(ʓ Ĕ,d4);ˣ(ʓ,æ,İ);}U saved=k8;compose(p,v,2*φ,İ/2,j9,k8,p6,ȍ+1);pair_multiply(v,p,Ё);j9=Ѯ(j9,saved)ĺ(φ у){fft ԍ(v,2*İ,İ);fft ԍ(v ʳ İ,2*İ,İ);U Ÿ=in(T(į(Ѯ(j9,in(4*æ))))ˆ Ȅ);composition_finish(p,v,İ,Ÿ);}ŝ{U*ʓ=v ʳ æ;double_y(ʓ,φ,İ,Ƈ);ˣ(ʓ,æ,İ);j9=ƫ(j9,j9);composition_difference(p,v,ʓ,Ё);project(p,φ,İ);j9=Ѯ(j9,in(2*İ));}}};ò NTT&ntt_plan(ý æ){ċ NTT t(1,Ś)ĺ(ó(t.ķ Û)<æ)t=NTT(æ,Ś)Ū t;}}í ntt(Ħ fps_mint>&a,Ǝ ć=Ś){Ƣ(!a ű&&!(ư&(ư-1))&&ư<=(1U<<23));ǧ=ó(ư)ѵ&t=Ü ntt_plan(Ŋ(1,n/2))ѵ reorder=ѯ{Ć(Ż 1,j Ē i<n Į){ý bit=n>>1 ø(;j&bit;bit>>=1)j^=bit;j^=bit ĺ(i<j)ь(ɨ
,a Ƃ);}};Ə(ƛ(fps_mint)==ƛ(Ű U)&&Ɖ fps_mint>)ѵ*č=õ Ű U*>(ʰ)ĺ(ć){reorder();t.fft ԍ(č,n);t.ŧ(č,n,fps_mint(n)ˆ Ȅ);}ŝ{t.Ҭ(č,n);reorder();}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ɯ Ď;Ӏ{Ő T,Ũ Work=Ī>Ɯ Operations{Ѽ ͼ W=Ď<Work>ͼ ƻ=џ ͼ Word=ę U;ċ W ź(ҥ a){W r(ư)ø(ö ó(ư)Į)r ƈ Ü Ī θ(ɨ Ȅ)Έ}ċ F ƾ(Ù W&a){F r(ư)ø(ö ó(ư)Į)r ƈ T θ(ɨ Ȅ)Έ}ċ Ü NTT&Ɓ(ǧ){Ú Ü ntt_plan(n);}ċ ɓ č(ƻ&a){Ə(ƛ(T)==ƛ(Word)&&Ɖ T>)Ū õ ɓ>(ʰ);}ċ Ù ɓ č(ҥ a){Ú õ Ù ɓ>(ʰ);}έ ć>Ŵ Җ(ƻ&a){ǧ=ư;Ƣ(n&&!(n&(Ԕ))&&n ɱ)ѵ&t=Ɓ(Ŋ(1,n/2));t.Ą fft<ć>(č(a),n);ƀ t.normalize(č(a),n);}Ŵ ntt(ƻ&a){Җ ə(a);}Ŵ ʒ(ɓ a,ǧ){ĵ(a,n/2,a+n/2);Ɓ(Ŋ(1,n/2)).Ą Ҭ(a,n,n/2);}Ŵ ʒ(ƻ&a){ǧ=ư ԕ(a ü,n/2,a ʷ n/2);Ɓ(Ŋ(1,n/2)).Ą Ҭ(č(a),n,n/2);}Ŵ Ʋ(ƻ&a){Җ ԍ(a);}Ŵ Ζ(ƻ&a,T Ÿ){if(a ű)Ú;Ġ{ł&t=Ɓ(1);t.ŧ(č(a),ư,Ÿ Ȅ);}ŝ Ć(T&x:a)x*=Ÿ;}Ŵ ɘ(ƻ&a,T ù,T ɰ){Ġ Ű geometric(č(a),ư,ù.v,ɰ.v)ї Ć(T&x:a){x*=ù;ù*=ɰ;}}Ŵ batch_inverse(ƻ&ˠ,ҥ src){ǧ=src Û;
#if defined(__x86_64__) || defined(__i386__)
Ġ{if(n>=128 ȏ){Ű batch_inverse8(č(ˠ),č(src),n)а}}
#endif
ƻ ʜ(n);T p=1 ø(Е)ʜ ƈ p,p*=src Ĕ;Ƣ(p!=ȓ);p=p ˆ ø(Ż Ԕ;i и i){ˠ ƈ ʜ Ĕ*p;p*=src Ĕ;}}Ŵ Ȉ(ƻ&r,ҥ a,ҥ b,ǧ){Ġ Ű Ŏ(č(r),č(a),č(b),n)ї Ć(Е)r ƈ ɨ*ϸ;}
#if defined(__x86_64__) || defined(__i386__)
Ą<Ԃ,Ǝ ù>ˌ accumulate_products8(ɓ ˠ,Ù ɓ Ù*a,Ù ɓ Ù*b,ǧ){Ę I8=Ű I8 В p ʖ(Ҳ),pp ʖ(2*Ҳ),ː ø(ö n ƞ){I8 Ŧ=đ,ȇ=Ŧ;
#pragma GCC unroll 7
Ć(љ){ͳ Ű œ(a Ƭ+i),y=Ű œ(b Ƭ+i)ɴ}ӽ Ű ɾ(Ű œ(Ҷ),Ĵ);Ű ŗ(Ҷ,Ĵ);}}
#endif
Ą<Ԃ,Ǝ ù>Ŵ accumulate_products(ɓ ˠ,Ù ɓ Ù*a,Ù ɓ Ù*b,ǧ){Ə(þ>=1&&þ<=7);
#if defined(__x86_64__) || defined(__i386__)
if(n>=8 ȏ){accumulate_products8<þ,ù>(ˠ,a,b,n)а}
#endif
Ć(Е){Word Ĵ Ɠ(љ)Ĵ=Ű ƫ(Ĵ,Ű Ѯ(a Ƭ Ĕ,b Ƭ Ĕ))Њ(!ù)Ĵ=Ű ƫ(Ĵ,ˠ Ĕ);ˠ ƈ Ĵ;}}Ŵ join_windows(ɓ Ǿ,ή,Ԃ){Ц(Ǿ,î,þ,î);}Ŵ Ц(ɓ Ǿ,ή,Ԃ,ý k4){Ć(Ż þ-1;i>0;--i){ɓ a=Ǿ+i*k4 Ĩ ɓ b=a-k4 ø(Ȕ î/2 ų)a ˉ Ű ƫ(a Ƃ,b Ƃ)ø(ý j=î/2;j<î ų)a ˉ Ű sub(a Ƃ,b Ƃ);}}Ŵ inverse_extend(F&h,ҥ a,ҥ Ǥ){Ƞ m=ó(Ǥ Û)Ĩ T im=T θ(Ǘ m);ƻ b(m);Ȉ(b,a,Ǥ,m);Ʋ(b);ż(b ü,b ʷ m/2,ȓ);ntt(b);Ȉ(b,b,Ǥ,m);Ʋ(b);h Ř(m)ø(Ż m/2 ϼ m Į)h ƈ-ϸ*im*im;}ċ T dot(Ù T*a,Ù T*b,ǧ){
#if defined(__x86_64__) || defined(__i386__)
Ġ{if(n>=64 ȏ)Ú T ɪ(Ű reverse_dot8(õ Ù ɓ>(a),õ Ù ɓ>(b),n));}
#endif
ϒ ă Ɠ(ö n Ƽ 16){о ʂ Ɠ(ý j=i;j<ĥ(n,i ү)ų)ʂ+=(о)a Ƃ Ȅ*b[-j]Ȅ;ă+=ʂ%Ҳ ĺ(ă>=Ҳ)ă-=Ҳ;}Ú T θ(ă);}ċ ý ѫ(Ħ Ø pair<ý,T>>&ơ){Ӂ Ɠ(ł&[j,x]:ơ){ď=Ø gcd(ď,j);Ġ if(x.v>=Ҳ)x.v-=Ҳ;}Ú ď;}ċ T sparse_dot(Ù Ħ Ø pair<ý,T>>&ơ,ʧ g,ý i){Ġ{Ə(7ULL*(Ҳ-1)*(2ULL*Ҳ-1)+0xffffffffULL*Ҳ<~0ULL);о ʂ Ē Word ă Ē Ԃ Ɠ(ł[j,x]:ơ){if(j>i)ϵ ʂ+=(о)x.v*g[i-j].v ĺ(++þ==7){ă=Ű ƫ(ă,Ű ń(ʂ)%Ҳ);ʂ Ē þ Ē}}Ú T ɪ(Ű ƫ(ă,Ű ń(ʂ)%Ҳ));}ŝ{T ă Ɠ(ł[j,x]:ơ){if(j>i)ϵ ă+=x*g[i-j];}Ҥ}}Ŵ intt(ƻ&a){Ʋ(a)Ĩ T ђ=T θ(Ǘ ó(ư));Ζ(a,ђ);}ċ T mod_sqrt(T a){if(a Ү||a.pow((Ҳ-1)/2)!=Ӽ)Ú 0;T r=a.pow(60),t=a.pow(119),c=T(3).pow(119);ý m=23 ǟ(t!=Ӽ){Ȧ(T u=t;u!=Ӽ;u*=u)++i;T b=c.pow(1<<(m-i-1));r*=b;c=b*b;t*=c;m=i;}Ú r;}ċ ҥ inverse_ntt_roots(ǧ){ċ ƻ ķ{1}ǟ(ó(ķ Û)<n){ҝ ˆ ԗ}Ú ķ;}ċ ҥ ntt_roots(ǧ){ċ ƻ ķ{1}ǟ(ó(ķ Û)<n){ҝ ԗ}Ú ķ;}ċ Ù џ&inverses(ǧ){Ƣ(ī(n)<ī(Ҳ));ċ џ ђ{0,1};
#if defined(__x86_64__) || defined(__i386__)
Ġ{ý ù=ђ Û,þ=(n+1-ù)/8*8 ĺ(þ>=256 ȏ){ђ.reserve(n+1);ђ Ř(Ў þ);Ű inverse_sequence8(õ ɓ>(ђ â)+ù,ù,þ);}}
#endif
while(ó(ђ Û)<=n){Ù Ż ó(ђ Û);ђ Ӯ(-ђ[Ҳ%i]*(Ҳ/i));}Ú ђ;}Ŵ trim(F&a){while(!a ű&&a.back()Ү)a.pop_back();}};}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
Ӏ{ā horner_detail{Ą<ý K,Ũ T>ò T scalar(Ù T*a,ƿ,T x){Ə(K>0);T ɰ=1 ø(ö K Į)ɰ*=x;ǜ T,K>ʂ{};Õ f5=n/K,Η=n%K ø(Ů Η Į)ʂ ƈ a[f5*K+i]ǟ(f5){--f5 ø(ö K Į)ʂ ƈ ʂ Ĕ*ɰ+a[f5*K+i];}T ă Ɠ(Ż K;i-->0;)ă=ă*x+ʂ Ĕ ȧ}
#if defined(__x86_64__) || defined(__i386__)
Ą<ý K>Ý ò Ī vector(ǣ*a,ƿ,Ī x){Ə(K у||K==2||K==4)ͼ V=ę I8;Ə(Ɖ Ī>&&ƛ(Ī)==ƛ(ī));Ī ɰ=x ø(ö 3 Į)ɰ*=ɰ Ĩ ī x8=ɰ Ȅ ø(κ K;i*=2)ɰ*=ɰ Ĩ ī Ĵ=ɰ Ȅ Ĩ V Ÿ ʖ(Ĵ),ť ʖ(ę ť(Ĵ));V ʂ[K]ø(ł&v:ʂ)v=đ;ɿ ҕ=8*K;Õ f5=n/ҕ,Η=n%ҕ ĺ(Η){Ť Ī ŀ[ҕ]{};Ǵ(ш(ŀ),a+f5*ҕ,Η*ƛ(Ī))ø(ö K Į)Ǵ(&ʂ Ĕ,ŀ Ƨ i,ƛ(V));}м(f5){--f5 ø(ö K Į)Ǵ(&ʂ Ĕ,a+f5*ҕ Ƨ i,ƛ(V));}while(f5){--f5 ø(ö K Į){V ʑ;Ǵ(&ʑ,a+f5*ҕ Ƨ i,ƛ(V));ʂ ƈ ę ɾ(Ĵ у?ʂ Ĕ:ę Ʀ(ʂ Ĕ,Ÿ,ť),ʑ);}}Ù V n5 ʖ(x8),l0 ʖ(ę ť(x8));V h1=ʂ[K-1]ø(Ż K-1;i-->0;)h1=ę ɾ(ę Ʀ(h1,n5,l0),ʂ Ĕ);Ť Ī ɽ ɚ;Ǵ(ш(ɽ),&h1,ƛ(h1));Ī ă Ɠ(Ż 8;i-->0;)ă=ă*x+ɽ Ĕ ȧ}
#endif
}ò Ī horner(ǣ*a,ƿ,Ī x){if(!n)Ú Ī(0)ĺ(x==Ī(0))Ú a Ŭ;
#if defined(__x86_64__) || defined(__i386__)
if(n>=32&&ę ě){if(n<64)Ú horner_detail::vector<1>(a,n,x)Ū horner_detail::vector<4>(a,n,x);}
#endif
if(n>=32)Ú horner_detail::scalar<8>(a,n,x);Ī ă Ē while(n)ă=ă*x+a[--n]ȧ}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
Ӏ{
#if defined(__x86_64__) || defined(__i386__)
Ý í factorial_transpose(ę I8(&x)ɚ){ę I8 a ɚ,b Ҹ(ö 8 Ƽ 2){ө _mm256_unpacklo_epi32(x Ĕ,x Υ);a Υ Ȁ(x Ĕ,x Υ);}Ć(ö 8 Ƽ 4){b ƈ ɻ(ɨ,a[i+2]);b Υ ǿ(ɨ,a[i+2]);b[i+2]=ɻ(a Υ,a[i+3]);b[i+3]ǿ(a Υ,a[i+3]);}Т{x ƈ Ł(ϸ,b[i+4],0x20);x[i+4]=Ł(ϸ,b[i+4],0x31);}}έ ˁ>Ý í factorial_chain(Ҿ į,ý Ѫ,ę I8 Ĵ,ę I8 Ÿ){Ę I=ę I8;I ɰ ʖ(ę one);I с=ʞ ø(Ȕ Ѫ Ӕ){I ŀ Ҹ(ˋ 8 Ȟ){ŀ ѓ Ĵ;Ĵ=ę ɏ(Ĵ,Ÿ)Њ(ˁ)Ÿ=ę sub8(Ÿ,ɰ)ї Ÿ=ę ɾ(Ÿ,ɰ);}factorial_transpose(ŀ)ø(ˋ 8 Ȟ){Ĺ(ˁ)ę ŗ(є(k+1)*Ѫ-j-8,Ľ(ŀ Ƭ,с))ї ę ŗ(є k*Ѫ+j,ŀ Ƭ);}}}Ý í factorials8(Ҿ Ԓ,Ҿ g9,ǧ){Ę I=ę I8 ͼ U=ę U Ȋ Ѫ=n/256*8,d5=Ѫ*32;Ť U Ÿ[32],Ɩ[32],n4[33],ć[33];I p ɛ,x ɛ,ɰ ʖ(ę one)ø(ö 32 Į)Ÿ ƈ ę in(i*Ѫ+1)ø(ˋ 4 Ȟ)p Ƭ ʖ(ę one),x ѓ ę œ(Ÿ Ƨ k)ø(ö Ѫ Į){ʢ p ѓ ę ɏ(p Ƭ,x Ƭ),x ѓ ę ɾ(x Ƭ,ɰ);}ʢ ę ŗ(Ɩ Ƨ k,p Ƭ);n4 Г ę one ø(ö 32 Į)n4 Υ=ę Ѯ(n4 Ĕ,Ɩ Ĕ);U ȣ=n4[32]ø(Ż d5 Ȝ)Ԓ ƈ ȣ,ȣ=ę Ѯ(ȣ,ę in(i+1));ȣ=һ(ȣ)ˆ.v ø(Ż Ԕ;i>=d5;--i)ȣ=ę Ѯ(ȣ,ę in(i+1)),g9 ƈ ȣ;ć[32]=ȣ ø(Ż 31;i и i)ć ƈ ę Ѯ(ć Υ,Ɩ Ĕ)ø(ˋ 4 Ȟ){factorial_chain ə(Ԓ Ƨ k*Ѫ,Ѫ,ę œ(n4 Ƨ k),ę œ(Ÿ Ƨ k));Ť U Ƴ ɚ,e2 Ҹ(Ȕ 8 ų){Ƞ end=(8*k+j+1)*Ѫ;Ƴ ˉ ę Ѯ(ć[8*k+j+1],ę in(end)
);e2 ˉ ę in(end-1);}factorial_chain ԍ(g9 Ƨ k*Ѫ,Ѫ,ę œ(Ƴ),ę œ(e2));}}
#endif
í Ӧ(Ҿ Ԓ,Ҿ g9,ǧ){if(!n)Ú;
#if defined(__x86_64__) || defined(__i386__)
if(n>=256&&ę ě){factorials8(Ԓ,g9,n)а}
#endif
ę U Ĵ=ę one ø(Е)Ԓ ƈ Ĵ,Ĵ=ę Ѯ(Ĵ,ę in(i+1));Ĵ=һ(Ԓ[Ԕ])ˆ.v ø(Ż Ԕ;i и i)g9 ƈ Ĵ,Ĵ=ę Ѯ(Ĵ,ę in(i));}ˬ requires ģ T,Ī>í Ӧ(T*Ԓ,T*g9,ǧ){Ƣ(0<=n&&ī(n)<=Ƀ);Ə(ƛ(Ī)==ƛ(ę U)&&Ɖ Ī>)ĺ(ī(n)==Ƀ){Ӧ(Ԓ,g9,Ԕ);Ԓ[Ԕ]=-Ԓ[n-2];g9[Ԕ]=-g9[n-2]а}Ӧ(õ Ҿ>(Ԓ),õ Ҿ>(g9),n);}}
#if defined(__linux__)
#include <sys/mman.h>
#endif
Ӏ{Ɯ FpsWordsDelete{Ĉ*region=Ƿ;Õ bytes Ē Ĉ ŕ()(ϒ*p)Ù{
#if defined(__linux__)
if(region){munmap(region,bytes)а}
#endif
::ŕ delete[](p,ɵ);}}ͼ FpsWords=Ø unique_ptr<ϒ[],FpsWordsDelete>;ò FpsWords ɀ(ȡ){
#if defined(__linux__) && defined(MADV_HUGEPAGE)
ɿ page=1<<21 ĺ(þ>=page/ƛ(ϒ)&&þ<=(Ø numeric_limits<Õ>::max()-2*page)/ƛ(ϒ)){Ģ bytes=(þ*ƛ(ϒ)+page-1)/page*page;Ĉ*region=mmap(Ƿ,bytes+page,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)ĺ(region!=MAP_FAILED){ʦ a6=(Λ(region)+page-1)&~(page-1)ѵ*p=::new(õ Ĉ*>(a6))ϒ[þ];madvise(p,bytes,MADV_HUGEPAGE)Ū FpsWords(p,FpsWordsDelete{region,bytes+page});}}
#endif
Ú FpsWords(new(ɵ)ϒ[þ]);}}
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
Ӏ{ā fps_output_detail{ò Ϸ Ŕ(Ϸ a){ʦ b ʖ(932051910u),ni ʖ(998244351u),p ʖ(Ƀ)ѵ Ŧ=ß(a,b),ȇ=ß(ñ(a,32),b);Ĥ Ú ǝ;}}í ƚ(л,ƿ){if(n<=į.δ())Ú;į.reserve(n)ĺ(n-ǲ<(1u<<17))Ú ǌ ù=Λ(Ԁ ǲ)ǌ end=Λ(Ԁ n);[[maybe_unused]]ʦ advise=[ù,end](Ø uintptr_t Б,ý a8){ʦ lo=(Ў Б-1)&~(Б-1),hi=end&~(Б-1)ĺ(lo<hi)ú Ĉ>(madvise(õ Ĉ*>(lo),hi-lo,a8));};
#if defined(MADV_HUGEPAGE)
advise(2u*1024*1024,MADV_HUGEPAGE);
#endif
#if defined(MADV_POPULATE_WRITE)
advise(4096,MADV_POPULATE_WRITE);
#endif
}έ Ŕ>í append_fps_words(л,Ù ī*in,ƿ){Ə(ƛ(Ī)==ƛ(ī)&&Ɖ Ī>)ĺ(n ε)Ú ĺ(n>į.max_size()-ǲ)throw Ø length_error("fps output");ƚ(į,ǲ+n);ǜ Ī,64>batch;ɮ Ɠ(;i+64<=n Ƽ 64){Ĺ(Ŕ)Ǵ(ш(batch â),in+i,ƛ(batch))ї{
#pragma GCC unroll 8
Ć(Ȕ 64 Ӕ){ʦ x=fps_output_detail::Ŕ(ɂ(õ Ù Ϸ*>(in+i+j)));Ǵ(ш(batch Ʒ j),&x,ƛ(x));}}į ӭ(į ƃ,batch ü,batch ƃ);}Ć(Ȝ){Ĺ(Ŕ)į η(һ(in Ĕ))ї į η(Ī θ(in Ĕ));}}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
Ӏ{ā ì kernels{Ę ĭ=Ϸ;ò ӂ kP=998244353,k2P=2*kP,kNI=998244351;í ı(ҳ a,Ϩ,Ɣ x,Ɣ y){Ť ĭ w Ѐ;ў
	#define Â(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) R(%[P],z) R(%[P2],r) D((%[x]),d) m(d,G)D(32(%[x]),w)D((%[y]),d)m(d,L)D(32(%[y]),E)D(4(%[y]),d)m(d,M)D(36(%[y]),F)".p2align 5\n\t" "1:\n\t" m(H,d)m(J,c)m(N,k)m(Q,g)q(H,w,u)q(J,w,s)q(4(%[a],%[h3]),w,e)q(36(%[a],%[h3]),w,i)m(I,l)v(t,u,u)m(K,n)v(t,s,s)B(A,e,u,e)B(A,i,s,i)m((%[a]),s)p(G,d,d)p(z,e,e)m(O,u)p(G,c,c)p(z,i,i)j(r,k,f)C(f,k,f)q(I,w,k)j(e,d,e)j(r,g,d)C(d,g,d)j(i,c,i)q(K,w,c)q(4(%[a],%[h],2),w,g)v(t,k,k)p(G,l,l)p(G,n,n)v(t,c,c)B(A,g,k,g)q(36(%[a],%[h],2),w,k)p(z,g,g)B(A,k,c,k)o(r,f,c)o(e,f,f)j(e,c,e)p(z,k,k)j(g,l,g)o(r,d,l)o(i,d,d)j(i,l,i)v(t,f,l)q(E,f,c)q(E,l,l)p(L,f,f)j(k,n,k)v(t,e,n)q(F,n,n)v(t,c,c)B(A,l,c,l)q(F,e,c)p(M,e,e)p(z,l,l)v(t,c,c)B(A,n,c,n)j(l,f,l)v(t,d,f)q(E,d,c)q(E,f,f)p(z,n,n)p(L,d,d)v(t,c,c)B(A,f,c,f)j(n,e,n)v(t,i,e)q(F,i,c)q(F,e,e)p(z,f,f)p(M,i,i)v(t,c,c)B(A,e,c,e)j(f,d,f)j(r,s,d)j(r,u,c)C(d,s,d)C(c,u,c)p(z,e,e)o(g,d,u)j(g,d,g)o(k,c,d)j(k,c,k)j(r,u,c)o(r,g,s)C(c,u,c)C(s,g,s)j(e,i,e)j(r,d,i)o(r,k,g)C(i,d,i)o(r,c,d)o(r,s,u)C(g,k,g)o(l,c,c)j(l,d,l)o(n,s,s)o(r,i,d)o(r,g,k)j(n,u,n)o(f,i,i)m(c,(%[a]))j(f,d,f)o(e,g,g)j(e,k,e)m(l,N)m(s,I)m(n,H)m(i,O)m(f,Q)m(g,K)m(e,J)S($64,%[a])T(%[end],%[a])U(1b)
ӥ(Â(%%ymm1,%%ymm0,%%ymm6,%%ymm10,%%ymm3,%%ymm7,Ô,%%ymm2,%%ymm8,Ê,%%ymm9,Ì,Ñ,Ò,%%ymm14,%%ymm5,$32,%%ymm4,Ó,%%ymm11,%%ymm15,$170,Î,Ð,É,%%ymm12,%%ymm13,(%[w]),(Ϲ,%[h3]),(Ϲ,%[h],2),32(Ϲ,%[h3]),32(Ϲ,%[h],2),32(%[w]),64(%[w]),(Ϲ,%[h]),32(Ϲ),32(Ϲ,%[h]),Ï,À,Ä,Æ)
#undef Â
ʆ,ӫ,ұ,[y]"r"(y),[x]"r"(x),[w]"r"(w),[P]"m"(kP),Ӫ
		: Ã);
}í ć(ҳ a,Ϩ,Ɣ x,Ɣ y){Ť ĭ w Ѐ;ў
	#define Â(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T) Q(%[P],w) Q(%[P2],k) C((%[x]),c) r(c,F)C(32(%[x]),v)C((%[y]),c)r(c,K)C(32(%[y]),D)C(4(%[y]),c)r(c,L)C(36(%[y]),E)".p2align 5\n\t" "1:\n\t" r((%[a]),c)r(N,f)r(M,s)r(O,t)n(k,c,i)n(H,c,c)n(k,f,j)h(H,i,i)n(J,f,f)h(k,c,u)n(k,s,e)h(J,j,j)q(l,i,g)p(D,i,d)n(k,t,m)h(G,e,e)p(D,g,g)o(K,i,i)h(I,m,m)n(G,s,s)B(u,c,u)q(l,j,c)p(D,c,c)q(l,d,d)n(I,t,t)A(z,g,d,g)p(D,j,d)o(K,j,j)o(w,g,g)q(l,d,d)A(z,c,d,c)h(g,i,g)q(l,e,i)p(E,e,d)p(E,i,i)o(w,c,c)o(L,e,e)q(l,d,d)A(z,i,d,i)h(c,j,c)q(l,m,j)p(E,m,d)p(E,j,j)o(w,i,i)o(L,m,m)q(l,d,d)A(z,j,d,j)h(i,e,i)h(k,f,e)h(k,s,d)o(w,j,j)B(e,f,e)B(d,s,d)h(k,t,s)n(k,u,f)B(s,t,s)n(k,e,t)n(d,u,u)h(d,f,d)h(s,t,t)n(s,e,s)n(k,g,e)h(j,m,j)n(k,c,m)h(i,e,e)q(l,d,f)h(j,m,m)p(v,f,f)n(i,g,i)n(j,c,j)p(v,d,c)q(l,e,g)o(F,d,d)p(v,g,g)q(l,c,c)A(z,f,c,f)p(v,e,c)o(F,e,e)o(w,f,f)q(l,c,c)A(z,g,c,g)h(f,d,f)q(l,t,d)p(v,t,c)p(v,d,d)o(w,g,g)r(f,M)q(l,m,f)p(v,f,f)o(F,t,t)q(l,c,c)A(z,d,c,d)h(g,e,g)p(v,m,e)o(F,m,m)o(w,d,d)h(k,u,c)r(g,G)h(k,i,g)B(c,u,c)q(l,e,e)h(k,s,u)B(g,i,g)r(c,(%[a]))A(z,f,e,f)h(k,j,e)B(u,s,u)h(d,t,d)r(g,H)o(w,f,f)B(e,j,e)r(u,N)r(e,J)r(d,O)h(f,m,f)r(f,I)R($64,%[a])S(%[end],%[a])T(1b)
ӥ(Â(%%ymm0,%%ymm9,%%ymm7,%%ymm1,%%ymm8,Ô,%%ymm4,%%ymm5,%%ymm14,$32,%%ymm10,Ì,Ñ,Ò,Ó,Ê,%%ymm2,%%ymm3,%%ymm6,%%ymm11,%%ymm15,$170,Î,Ð,É,%%ymm12,%%ymm13,(%[w]),(Ϲ,%[h3]),(Ϲ,%[h]),32(Ϲ,%[h3]),32(Ϲ,%[h]),32(%[w]),64(%[w]),(Ϲ,%[h],2),32(Ϲ),32(Ϲ,%[h],2),Ï,À,Ä,Æ)
#undef Â
ʆ,ӫ,ұ,[y]"r"(y),[x]"r"(x),[w]"r"(w),[P]"m"(kP),Ӫ
		: Ã);
}í forward_identity(ҳ a,Ϩ,Ɣ y){ў
	#define Â(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N) K(%[P],A) K(%[P2],d) I(4(%[y]),z) I(36(%[y]),t)".p2align 5\n\t" "1:\n\t" j(C,o)j(G,g)j(B,p)j(F,l)j((%[a]),i)j(D,m)j(E,q)j(H,r)c(d,o,h)c(d,g,f)n(h,o,h)c(d,p,o)n(f,g,f)k(d,h,g)c(d,l,s)n(o,p,o)k(d,f,p)n(s,l,s)c(o,g,g)c(d,i,l)k(o,h,o)c(s,p,p)x(u,g,h)w(t,g,e)n(l,i,l)x(u,p,i)w(t,h,h)v(z,g,g)k(s,f,s)w(t,p,f)w(t,i,i)x(u,e,e)v(z,p,p)J($170,h,e,h)c(d,m,e)x(u,f,f)n(e,m,e)v(A,h,h)c(d,q,m)n(m,q,m)J($170,i,f,i)c(d,r,f)k(e,l,q)n(f,r,f)c(e,l,e)v(A,i,i)c(d,q,l)k(f,m,r)c(f,m,f)k(d,e,m)n(l,q,l)c(d,r,q)n(m,e,m)c(d,o,e)c(h,g,h)k(d,f,g)n(q,r,q)c(d,s,r)n(e,o,e)n(g,f,g)k(d,l,f)k(d,m,o)n(r,s,r)c(i,p,i)k(d,q,p)k(d,g,s)k(e,l,l)c(e,f,e)k(h,m,m)c(h,o,h)k(r,q,q)j(l,(%[a]))c(r,p,r)k(i,g,g)c(i,s,i)j(e,C)j(m,D)j(h,B)j(q,E)j(r,G)j(g,H)j(i,F)L($64,%[a])M(%[end],%[a])N(1b)
ӥ(Â(Ô,%%ymm14,%%ymm11,%%ymm9,%%ymm1,%%ymm8,%%ymm4,Ê,Ì,%%ymm3,%%ymm5,Ð,%%ymm0,%%ymm2,%%ymm6,%%ymm7,%%ymm10,%%ymm13,$32,Ñ,Ò,Ó,%%ymm12,%%ymm15,(Ϲ,%[h3]),(Ϲ,%[h]),(Ϲ,%[h],2),32(Ϲ),32(Ϲ,%[h3]),32(Ϲ,%[h]),32(Ϲ,%[h],2),É,Î,Ï,À,Ä,Æ)
#undef Â
ʆ,ӫ,ұ,[y]"r"(y),[P]"m"(kP),Ӫ
		: Ã);
}í inverse_identity(ҳ a,Ϩ,Ɣ y){ў
	#define Â(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N) K(%[P],A) K(%[P2],f) I(4(%[y]),z) I(36(%[y]),t)".p2align 5\n\t" "1:\n\t" p(F,c)p(H,r)p((%[a]),n)p(G,j)h(f,c,o)h(B,c,c)h(f,r,i)e(B,o,o)h(D,r,r)e(f,c,s)e(D,i,i)x(u,o,d)w(t,o,g)v(z,o,o)x(u,i,k)w(t,i,m)w(t,d,d)h(C,n,q)w(t,k,k)v(z,i,i)e(C,n,n)l(s,c,s)x(u,g,g)h(E,j,c)e(E,j,j)x(u,m,m)J($170,d,g,d)e(f,q,g)J($170,k,m,k)v(A,d,d)l(g,q,g)h(f,n,q)v(A,k,k)e(f,c,m)l(q,n,q)h(s,g,n)l(m,c,m)e(s,g,s)e(f,r,g)h(f,j,c)e(d,o,d)l(g,r,g)l(c,j,c)e(f,n,j)e(k,i,k)h(d,q,i)e(d,q,d)h(g,m,q)h(k,c,r)e(g,m,g)e(k,c,k)e(f,i,c)h(f,s,m)h(f,d,o)l(j,n,j)l(c,i,c)e(f,q,i)e(f,r,n)l(m,s,m)l(o,d,o)p(j,(%[a]))h(f,g,j)h(f,k,d)l(i,q,i)l(n,r,n)p(c,C)l(j,g,j)l(d,k,d)p(m,F)p(o,B)p(i,G)p(n,E)p(j,H)p(d,D)L($64,%[a])M(%[end],%[a])N(1b)
ӥ(Â(%%ymm0,%%ymm7,Ô,%%ymm14,%%ymm8,Ì,%%ymm5,%%ymm3,%%ymm9,Ð,%%ymm10,%%ymm2,%%ymm4,Ê,%%ymm11,%%ymm1,%%ymm6,%%ymm13,$32,Ñ,Ò,Ó,%%ymm12,%%ymm15,(Ϲ,%[h3]),(Ϲ,%[h]),32(Ϲ,%[h3]),32(Ϲ,%[h]),(Ϲ,%[h],2),32(Ϲ),32(Ϲ,%[h],2),É,Î,Ï,À,Ä,Æ)
#undef Â
ʆ,ӫ,ұ,[y]"r"(y),[P]"m"(kP),Ӫ
		: Ã);
}í scale_radix2(ҳ a,Ϩ,Ɣ y){ў
	#define Â(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K) H(%[P],m) H(%[P2],C) G(4(%[y]),s) G(36(%[y]),l)".p2align 5\n\t" "1:\n\t" r((%[a]),c)r(F,d)z(C,c,e)z(D,c,c)z(C,d,f)k(D,e,e)z(E,d,d)q(n,c,h)k(E,f,f)p(l,c,v)q(n,e,i)p(l,e,w)q(n,d,j)p(l,d,x)q(n,f,g)p(l,f,u)p(l,h,h)p(l,i,i)p(l,j,j)p(l,g,g)q(n,v,v)q(n,w,w)q(n,x,x)q(n,u,u)o(s,c,c)o(s,e,e)A(t,h,v,h)A(t,i,w,i)o(s,d,d)o(s,f,f)A(t,j,x,j)A(t,g,u,g)o(m,h,h)o(m,i,i)o(m,j,j)o(m,g,g)k(h,c,h)k(i,e,i)k(j,d,j)k(g,f,g)k(m,h,f)k(m,i,d)k(m,j,e)k(m,g,c)B(f,h,f)B(d,i,d)B(e,j,e)B(c,g,c)r(f,(%[a]))r(d,D)r(e,F)r(c,E)I($64,%[a])J(%[end],%[a])K(1b)
ӥ(Â(%%ymm0,%%ymm1,%%ymm2,%%ymm3,%%ymm10,%%ymm4,%%ymm6,%%ymm8,Ô,%%ymm13,%%ymm15,$32,Ñ,Ò,Ó,Ê,%%ymm12,$170,%%ymm11,%%ymm5,%%ymm7,%%ymm9,Ì,Î,Ð,%%ymm14,(Ϲ,%[h]),32(Ϲ,%[h]),32(Ϲ),É,Ï,À,Ä,Æ)
#undef Â
ʆ,ӫ,ұ,[y]"r"(y),[P]"m"(kP),Ӫ
		: Ã);
}}ā blocked_ntt{ò ӂ ϴ=ƥ kP;ò è ý Ҕ=25;Ϥ{Ę ƥ ĭ ͼ ƥ kP;ӂ Đ(Ö x,Ö y){Ú Ö(ŉ(x)*y Ұ);}ӂ ǹ(Ö x,Ö e){Ö r=1 ø(;e;e>>=1,x=Đ(x,x))if(e&1)r=Đ(r,x)Έ}ӂ ť(Ö w){Ú Ö((ŉ(w)<<32)/kP);}ò ӂ kR=Ö((ŉ(1)<<32)Ұ);ò è ý kRootCount=Ҕ-4;ò è ł kRoots=[]{ǜ ǜ Ö,kRootCount>,2>t{}ø(Ȕ kRootCount ų){t Ŭ ˉ ǹ(3,(Ͼ 1)>>(j+2));t ƹ ˉ ǹ(t Ŭ Ƃ,ӓ);}Ú t;}();Ǹ ĕ(Ö x){Ú ç(ý(x));}Ǹ ƫ(ɍ,ĭ y){Ú ň(x,y);}Ǹ ǘ(ɍ,ĭ y){Ú ê(ň(x,П),y);}Ǹ ń(ɍ,Ö c0){Ú ą(x,ê(x,ĕ(c0)));}Ɯ Ʈ{ĭ w,q;explicit Ʈ(Ö Ĵ):w(ĕ(Ĵ)),q(ĕ(ť(Ĵ))){}Ʈ(Ö Ĵ,Ö o7):w(ĕ(Ĵ)),q(ĕ(o7)){}};Ǹ Ź(ɍ,Œ&f){ġ Ŧ=ñ(ß(x,f.q),32)ι ȇ=ß(ʤ,f.q)ι q=ĳ(Ŧ,ȇ,0xAA)Ū ê(ȑ(x,f.w),ȑ(q,ɟ));}Ί υ(ȕ){Ú(k>>3<<4)|(k&7);}ò ɿ ϙ(ý lg){Ú(Ǽ lg)/8 ү;}í build_table(Ņ t,ȡ,Ù ǜ Ö,kRootCount>&ķ){t[υ(0)]=1;t[υ(0)+8]=ť(1)ø(ý ȍ Ē ȍ<kRootCount ė ȍ){Ģ h=Ǽ ȍ ĺ(h>=þ)ϵ ğ ˀ=ķ[ȍ]ĺ(h<8){Р{ğ v=Đ(t[υ(j)],ˀ);t[υ(h+j)]=v;t[υ(h+j)+8]=ť(v);}ӈ;}Œ by(ˀ),to_r(kR)ø(ǈ h Ӕ){ġ v=ń(Ź(_mm256_load_si256(õ ġ*>(t+υ(j))),by),kP)ι vr=ń(Ź(v,to_r),kP);ѩ(ț(t+υ(h+j)),v);ѩ(ț(t+υ(h+j)+8),ȑ(vr,ĕ(ƥ kNI)));}}}í inverse_radix4(ҳ f,Ϩ,Ɣ ô,Œ&s){Œ z(ô ƹ,ô[9])ǌ ŧ=[&s](ɍ)Ý{Ú
 ń(Ź(x,s),kP);}ʋ{ġ p0=f Ƃ,p1=f Ǻ,p2=f ɋ,p3=f Ɋ ι ab=ń(ƫ(p0,p1),2*kP),cd=ń(ƫ(p2,p3),2*kP)ι amb=ń(ǘ(p0,p1),2*kP),cmd=Ź(ǘ(p2,p3),z);f ˉ ŧ(ϑ),f Ǻ=ŧ(ϐ);f ɋ=ŧ(Ќ),f Ɋ=ŧ(Ϗ);}}}}ā ʃ kernels{Ę ĭ=Ϸ;ò ӂ kP=998244353,k2P=2*kP;Ɨ í forward_h4(ҳ a,ȡ,Ɣ n8,ȕ){Ć(ǈ þ ų)ì ƥ ı(a ү*j,4,n8+ã υ(k+j),n8+ã υ(2*(k+j)));}Ɨ í inverse_h4(ҳ a,ȡ,Ɣ n8,ȕ){Ć(ǈ þ ų)ì ƥ ć(a ү*j,4,n8+ã υ(k+j),n8+ã υ(2*(k+j)));}}ā ē{ò ӂ ϴ=ì ϴ;enum Ũ Half{kBoth,kLower,kUpper};Ũ Ě{Ϳ Ě(Õ č):ϳ(ɀ(č)),next_(ϳ ӯ),end_(next_+č){}Ě(Ù Ě&)=delete;Ě&ŕ=(Ù Ě&)=delete;ċ ɿ footprint(ƿ){Ú(n+7)/8*8 ү;}Ĝ ś(ƿ){if(Õ(end_-next_)<footprint(n))throw Ø bad_alloc()ѵ*start=next_;next_+=footprint(n)Ū{start,n};}Ǝ zero_initialized()Ù{Ú ϳ.get_deleter().region!=Ƿ;}Ⱦ FpsWords ϳ;Ņ next_,*end_;};Ϥ{Ę ã ƫ ͼ ã ĕ ͼ ã ǘ ͼ ã Ʈ ͼ ã kP ͼ ã kR ͼ ã ń ͼ ã υ ͼ ã ĭ;ò ǒ ĭ Ō(Ɣ p){Ú _mm256_load_si256(õ ġ*>(p));}í Š(Ņ p,ɍ){ѩ(ț(p),x);}Ǹ ʗ(ɍ){Ú ń(x,2*kP);}Ǹ ů(ɍ){Ú ń(ń(x,2*kP),kP);}Ǹ low_difference(ɍ,ĭ y){ġ t=ê(x,y)Ū ą(t,ň(t,П));}Ǹ ɸ(ɍ,ĭ y){Ú ń(ê(ƫ(x,ɟ),y),kP);}Ǹ ŭ(ɍ,Œ&w){Ú ã Ź(x,w);}ò Ʈ ʮ(Ɣ n8,
ȕ){Ɣ e=n8+υ(k)Ū Ʈ(e Ŭ,e ɚ);}ò Ʈ Ϙ(М,Õ p){Ɣ e=ķ+υ(p>>1)Ū p&1?Ʈ(Ͼ e Ŭ,~e ɚ):Ʈ(e Ŭ,e ɚ);}Ɯ Τ{Ʈ x,y,z;ĉ Τ(Ɣ n8,Õ g):x(ʮ(n8,g)),y(ʮ(n8,2*g)),z(ʮ(n8,2*g+1)){}};ˮ forward_h1(ĭ(&f)ɛ,Ù Τ&w){ġ a ӏ(f Ŭ),b ӏ(f ƹ),c Ҝ(f ǥ,w.x),d Ҝ(f Ѐ,w.x)ι ac ӏ(ƫ(a,c)),amc=low_difference(a,c)ι bd Ҝ(ƫ(b,d),w.y),bmd Ҝ(ǘ(b,d),w.z);f Г ƫ(ac,bd),f ƹ=ǘ(ac,bd),f ǥ=ƫ(amc,bmd),f Ѐ=ǘ(amc,bmd);}ˮ τ(ĭ(&f)ɛ,Ù Τ&w){ġ ab ӏ(ƫ(f Ŭ,f ƹ)),cd ӏ(ƫ(f ǥ,f Ѐ))ι amb Ҝ(ǘ(f Ŭ,f ƹ),w.y),cmd Ҝ(ǘ(f ǥ,f Ѐ),w.z);f Г ʗ(ϑ),f ƹ ӏ(ϐ);f ǥ Ҝ(Ќ,w.x),f Ѐ Ҝ(Ϗ,w.x);}Ɯ alignas(64)ʅ{Ö ɽ[17];};í Ε(ʅ&Ļ,ĭ a,Œ&w){Ļ.ɽ[16]Ē Š(Ļ.ɽ+8,a);Š(Ļ.ɽ,ń(ŭ(a,w),kP));}ˮ ʿ(ʅ(&Ļ)ɛ,ġ(&a)ɛ,М,Õ g){Œ y=ʮ(ķ,2*g),z=ʮ(ķ,2*g+1);
#pragma GCC unroll 4
ҿ{Ļ Ѱ.ɽ[16]Ē Š(Ļ Ѱ.ɽ+8,a Ѱ)ι wa Ҝ(a Ѱ,t<2?y:z);Š(Ļ Ѱ.ɽ,ń(t%2?ê(П,wa):wa,kP));}}ĉ Ǹ ǽ(Ù ʅ&Ļ,Ҡ){ȱ,p=ɟ,p2=П;н,ȇ,bi,t0,t1,t2,t3;
	#define Â(a,c,d,e,f,g,h,i,j,k,l,m,n,o,q,r,s,t,u,v) i((%[b]),a) d(36(%[w]),a,e) d(r,a,c) i(4(%[b]),a)d(r,a,g)d(q,a,h)i(8(%[b]),a)d(q,a,j)d(o,a,k)f(g,e,e)f(h,c,c)i(12(%[b]),a)d(o,a,g)d(n,a,h)f(j,e,e)f(k,c,c)i(16(%[b]),a)d(n,a,j)d(m,a,k)f(g,e,e)f(h,c,c)i(20(%[b]),a)d(m,a,g)d(l,a,h)f(j,e,e)f(k,c,c)i(24(%[b]),a)d(l,a,j)d(s,a,k)f(g,e,e)f(h,c,c)i(28(%[b]),a)d(s,a,g)d(4(%[w]),a,h)f(j,e,e)f(k,c,c)f(g,e,e)f(h,c,c)d(%[ni],c,g)d(%[ni],e,h)d(%[p],g,g)d(%[p],h,h)f(g,c,c)f(h,e,e)u($32,c,c)t($0xaa,e,c,c)v(%[p2],c,g)"vpminud %[t0], %[even], %[even]"
asm(Â(%[bi],%[Ŧ],Ò,%[ȇ],Í,%[t0],%[t1],Ï,%[t2],%[t3],12(%[w]),16(%[w]),20(%[w]),24(%[w]),28(%[w]),32(%[w]),8(%[w]),Î,Ó,Ô)
#undef Â
:[Ŧ]"=&x"(Ŧ),[ȇ]"=&x"(ȇ),[bi]"=&x"(bi),[t0]"=&x"(t0),[t1]"=&x"(t1),[t2]"=&x"(t2),[t3]"=&x"(t3):[w]"r"(Ļ.ɽ),[b]"r"(b),[ni]"x"(ni),[p]"x"(p),[p2]"x"(p2),"m"(*õ ğ(*)[17]>(Ļ.ɽ)),"m"(*õ ğ(*)ɚ>(b)))Ū Ŧ;}Ɯ ǂ{Ǐ Ƈ,Ƚ=Ś;М;Ț{Ć(ǈ þ ų,a+=32){Ù Τ w(ķ,Ў j);Ѻ(f,w);
#pragma GCC unroll 4
ҿ Š(ϋ t,ů(f Ѱ));}}};Ɯ Ɉ{Ǐ Ś,Ƚ=Ƈ;Ɣ ô;Ɣ in;Ț{Ɣ ʝ=in+32*ù ø(ǈ þ ų,a+=32,ʝ+=32){Ù Τ w(ô,Ў j);ϣ={Ō(ʝ),Ō(ʝ+8),Ō(ʝ ү),Ō(ʝ+24)};τ(f,w)ø(ý Ń)Š(ϋ t,f Ѱ);}}};Ɯ ȥ{Ǐ Ƈ,Ƚ=Ƈ;М;Ɣ ô;Ҡ;Õ origin Ē ĉ Ĉ ѐ(Һ,Õ g,ʅ(&Ļ)ɛ)Ù{Ѻ(f,Τ(ķ,g));
#pragma GCC unroll 4
ҿ f Ѱ=ů(f Ѱ);ʿ(Ļ,f,ķ,g);}ĉ Ĉ finish(Ң,Õ g,Ù ʅ(&Ļ)ɛ)Ù{ϣ;
#pragma GCC unroll 4
ҿ f Ѱ=ǽ(Ļ Ѱ,b Ƨ(4*g+t-origin));τ(f,Τ(ô,g))ø(ý Ń)Š(ϋ t,f Ѱ);}Ț{ɥ ɛ;ѐ(a,ù,Ļ Ŭ)ø(ǈ þ ų,a+=32){if(j+1<þ)ѐ(a+32,Ў j+1,Ļ[(j+1)&1]);finish(a,Ў j,Ļ[j&1]);}}};Ɯ Ș{Ǐ Ś,Ƚ=Ƈ;ȥ Ŏ;Һ;ĉ Ĉ ѐ(Õ g,ʅ(&Ļ)ɛ)Ù{Ъ(Ļ,f,Ŏ.ķ,g);}Ǣ{ɥ ɛ;Ȃ{ɕ;Ŏ.finish(į,Ў j,Ļ[j&1]);}}};Ą<Õ K>Ɯ ϗ{Ǐ Ś,Ƚ=Ƈ;ǜ Ș,K>ơ;Ĉ ѐ(Õ g,ʅ(&Ļ)[K]ɛ)Ù{Ć(ѿ K Ȟ)ơ Ƭ.ѐ(g,Ļ Ƭ);}Ǣ{ɥ[K]ɛ;Ȃ{ɕ Ȣ g=Ў j;ϣ;
#pragma GCC unroll 4
Κ{f Ѱ=ǽ(Ļ[j&1]Ŭ Ѱ,ơ Ŭ.Ŏ.b Ƨ(4*g+t-ơ Ŭ.Ŏ.origin))ø(ʶ K Ȟ)f Ѱ ӏ(ƫ(f Ѱ,ǽ(Ļ[j&1]Ƭ Ѱ,ơ Ƭ.Ŏ.b Ƨ(4*g+t-ơ Ƭ.Ŏ.origin))));}τ(f,Τ(ơ Ŭ.Ŏ.ô,g));
#pragma GCC unroll 4
Κ Ӿ}}};Ɯ ForwardProductBottom{Ǐ Ƈ,Ƚ=Ś;ȥ Ŏ;Ț{Œ μ(kR);ɥ ɛ;Ŏ.ѐ(a,ù,Ļ Ŭ)ø(ǈ þ ų,a+=32){if(j+1<þ)Ŏ.ѐ(a+32,Ў j+1,Ļ[(j+1)&1]);Ҡ=Ŏ.b Ƨ(4*(Ў j)-Ŏ.origin);
#pragma GCC unroll 4
ҿ Š(ϋ t,ń(ŭ(ǽ(Ļ[j&1]Ѱ,b ϛ),μ),kP));}}};Ő ƭ>Ũ ǆ{Ⱥ ǆ(М,Ɣ ô,Ϧ):r_(ķ),ir_(ô),bottom_(b9){}Ĉ ϖ(Ң,Ԅ,ȕ)Ù{if(nv<=kTile)Ú tile(a,nv,k);ƽ 4;ʠ ı(a,h,k)Ȳ γ Ĺ(ƭ::Ƚ)ć(a,h,k);}ċ è Ǝ radix8(Ԅ){Ú nv/2>kTile;}Ĉ Ԋ(Ң,Ԅ,ȕ)Ù{ƽ 4 Ȳ γ Ĺ(ƭ::Ƚ)ć(a,h,k);}Ⱦ ċ ɿ kTile=256;Ĉ tile(Ң,Ԅ,ȕ)Ù{ʠ{Ć(Ϩ=nv/4;h>=16;h/=4)Ԗ ı(ϋ j,h,g)ĺ(nv>=16)ƥ forward_h4(И(a),nv/16,r_,k*(nv/16));}bottom_(a,nv/4,k*(nv/4))Њ(ƭ::Ƚ){if(nv>=16)ƥ inverse_h4(И(a),nv/16,ir_,k*(nv/16))ø(Ϩ=16;h<nv;h*=4)Ԗ ć(ϋ j,h,g);}}ċ ҳ И(Ң){Ú ț(a);}Ĉ ı(Ң,Ϩ,ȕ)Ù{if(k ε)Ú ì ƥ forward_identity(И(a),h,r_);ì ƥ ı(И(a),h,r_+υ(k),r_+υ(Ӷ));}Ĉ ć(Ң,Ϩ,ȕ)Ù{if(k ε)Ú ì ƥ inverse_identity(И(a),h,ir_);ì ƥ ć(И(a),h,ir_+υ(k),ir_+υ(Ӷ));}Ɣ r_,*ir_;ƭ bottom_;};Ũ Ɏ{Ⱥ Ɏ(Ɣ in,Õ æ,Õ ƍ):in_(in),shift_(ƍ),end_(ƍ+æ),first_((ƍ+7)/8),ϲ(ƍ/8){Ȑ=end_/8>first_?end_/8-first_:0;Х=(end_+7)/8-ϲ;}ĭ ŕ()(Õ v)Ù{if(v-first_<Ȑ)Ú ɂ(õ ġ*>(in_+(8*v-shift_)))ĺ(v-ϲ>=Х)Ú đ Ū edge(in_,shift_,end_,v);}Ǝ in_place(Ɣ į)Ù{Ú in_==є shift_;}Ĉ zero_outside(Ȩ,І,п)Ù{Ģ begin=Ø clamp(shift_,lo,hi),end=Ø clamp(end_,begin,hi);ż(є lo,є
 begin,0);ż(є end,є hi,0);}Ǝ within(І,п)Ù{Ú Х ε||(ϲ>=lo&&ϲ+Х<=hi);}Ⱦ Ɨ ċ ĭ edge(Ɣ in,Õ ƍ,Õ end,Õ v){ћ x ɚ={}ø(ɮ=8*v ϼ 8*v+8 Į)if(i>=ƍ&&i<end)x[i-8*v]=in[i-ƍ]Ū Ō(x);}Ɣ in_;Õ shift_,end_;Õ first_,Ȑ;Õ ϲ,Х;};Ő Input>ˮ forward_top4_body(Input in,ҳ į,Ϩ,М){Œ z(ķ ƹ,ķ[9])ι p=ɟ ĺ(in.within(0,2*h)){Р{ġ a=in(j),b=in(j+h),zb Ҝ(b,z);į ˉ ƫ(a,b),į Ǻ=ǘ(a,b),į ɋ=ƫ(a,zb),į Ɋ=ǘ(a,zb);}Ú;}if(in.within(2*h,4*h)){Р{ġ c=in(j ʳ h),d=in(j+3*h)ι mc=ê(p,c),zmd Ҝ(ê(p,d),z);į ˉ ƫ(c,d),į Ǻ=ǘ(c,d),į ɋ=ƫ(mc,zmd),į Ɋ=ǘ(mc,zmd);}Ú;}Р{ġ a=in(j),b=in(j+h),c=in(j ʳ h),d=in(j+3*h)ι ac=ƫ(a,c),amc=ê(ƫ(a,p),c)ι bd=ƫ(b,d),zbmd Ҝ(ê(ƫ(b,p),d),z);į ˉ ƫ(ac,bd),į Ǻ=ǘ(ac,bd);į ɋ=ƫ(amc,zbmd),į Ɋ=ǘ(amc,zbmd);}}Ő Input>ˮ forward_top2_body(Input in,ҳ į,Ϩ){ġ p=ɟ ĺ(in.within(0,h)){Р į ˉ į Ǻ=in(j)а}if(in.within(h,2*h)){Р{ġ v=in(j+h);į ˉ v,į Ǻ=ê(p,v);}Ú;}Р{ġ u=in(j),v=in(j+h);į ˉ ƫ(u,v),į Ǻ=ê(ƫ(u,p),v);}}Ɨ í forward_top4(Ɏ in,ҳ į,Ϩ,М){forward_top4_body(in,į,h,ķ);}Ɨ í forward_top2(Ɏ in,ҳ į,Ϩ){forward_top2_body(in,į,h);}ò Ǝ forward_top8(Ù Ɏ&,Ņ
,Õ,Ɣ){Ú Ś;}Ϣ radix2_halves(Ù ǆ<ƭ>&Ŝ,Ù Ɏ&in,Ң,Ԅ,М){ƽ 2;ʠ{if(Ŝ.radix8(nv)&&forward_top8(in,a,nv,ķ)){Ŝ.Ԋ(a,h,0);Ŝ.Ԋ(ϋ h,h,1)а}forward_top2(in,ț(a),h);}ő(a,h,0);ő(ϋ h,h,1);}í inverse_top4(ҳ f,Ϩ,Π,Ɣ ô,Œ&s){if(ǁ==ȉ)Ú ã inverse_radix4(f,h,ô,s)Ƥ z(ô ƹ,ô[9])ǌ ŧ Ѓ ʋ{ġ p0=f Ƃ,p1=f Ǻ,p2=f ɋ,p3=f Ɋ ι ab ӏ(ƫ(p0,p1)),cd ӏ(ƫ(p2,p3))ι amb ӏ(ǘ(p0,p1)),cmd Ҝ(ǘ(p2,p3),z)ĺ(ǁ==Ʃ){f ˉ ŧ(ϑ),f Ǻ=ŧ(ϐ);}ŝ{f ɋ=ŧ(Ќ),f Ɋ=ŧ(Ϗ);}}}í Ӥ(ҳ f,Ϩ,Π,Ö ŧ){if(ǁ==ȉ){ћ s[16]={};s ƹ=ŧ;s[9]=ã ť(ŧ)Ū ì ƥ scale_radix2(f,h,s);}Œ s(ŧ)ʋ{if(ǁ==Ʃ)f ˉ ń(ŭ(ƫ(f Ƃ,f Ǻ),s),kP)ї f Ǻ=ń(ŭ(ǘ(f Ƃ,f Ǻ),s),kP);}}}Ũ Č{Ⱥ ċ è ý kMinLog=6,Ҕ=ì Ҕ;ċ ɿ č(ý Δ){Ú à(2*ã ϙ(Δ));}Č(Ě&ŋ,ý Δ):lg_max_(Δ){if(Δ<kMinLog||Δ>Ҕ)ǩ Ȣ n8=ã ϙ(Δ),entries=(Ǽ Δ)ș;}ý Δ()Ù{Ú lg_max_;}М()Ù{Ú Ɛ;}Ɣ ô()Ù{Ú ō;}Ĉ ı(ĝ in,Õ ƍ,ɐ)Ù{run(į,Ο,ä ǂ{Ɛ},0,ȉ);}Ĉ ı(ҽ)Ù{ı(a,0,a);}Ĉ ɇ(ĝ in,Õ ƍ,ɐ)Ù{Ɵ;ʐ(2*ǲ)Ĩ Ɏ src=Ο Ĩ ä Ҟ}Ĉ ć(ĝ in,ɐ,Π=ȉ)Ù{ˊ(ǲ,Ś);run(į,ä Ɏ(Ƿ,0,0),ä Ɉ{ō,in â},ŧ,ǁ);}Ĉ ć(ҽ,Π=ȉ)Ù{ć(a,a,ǁ);}Ĉ Ŷ(ĝ in,Õ ƍ,ɐ,ĝ b,Π=ȉ,Ö c=1)Ù{Ę ā å;ğ ŧ=Đ(inverse_scale(ǲ,Ƈ),c);run(į,Ο,ä ȥ{Ɛ,ō,b â},ŧ,ǁ);}Ĉ Ŷ(ҽ,ĝ b)
Ù{Ŷ(a,0,a,b);}Ĉ inverse_product(ĝ a,ĝ b,ɐ,Π=ȉ)Ù{ˊ(ǲ,Ƈ)Ĩ ä ȥ Ŏ{Ɛ,ō,b â};run(į,ä Ɏ(Ƿ,0,0),ä Ș{Ŏ,ʰ},ŧ,ǁ);}Ɯ Pair{ĝ a,b;};Ĉ Ŗ(ԁ Pair>Ǜ,ɐ,Π=ȉ)Ù{switch(Ǜ Û){case 1:Ú inverse_product(Ǜ Ŭ.a,Ǜ Ŭ.b,į,ǁ)ϻ 2:Ú inverse_products<2>(Ǜ,į,ǁ)ϻ 3:Ú inverse_products<3>(Ǜ,į,ǁ);default:ǩ;}}Ϣ Ɇ(Ϧ,ɐ,Π=ȉ)Ù{ˊ(ǲ,Ƈ);run(į,ä Ɏ(Ƿ,0,0),b9,ŧ,ǁ);}Ĉ forward_product(ĝ in,Õ ƍ,ɐ,ĝ b)Ù{Ù ä ȥ Ŏ{Ɛ,ō,b â};run(į,Ο,ä ForwardProductBottom{Ŏ},0,ȉ);}Ĉ Ź(ҽ,ĝ b)Ù{Ɩ(ʰ,ʰ,b â,ư,ѯ(ä ĭ,ä ĭ p)Ý{Ú p;});}Ĉ multiply_add(Ĝ c,ĝ a,ĝ b)Ù{Ɩ(c â,ʰ,b â,ư,ѯ(ä ĭ c,ä ĭ p)Ý{Ú ä ƫ(c,p);});}Ⱦ Ö inverse_scale(ƿ,Ǝ Ŕ)Ù{ʐ(n)Ū ä Ȍ[Ŕ][ĩ(n)];}Ĉ ʐ(ƿ)Ù{Ƞ lg=ĩ(n)ĺ(!ϧ(n)||lg<kMinLog||lg>lg_max_)ǩ;}Ą<Õ K>Ĉ inverse_products(ԁ Pair>Ǜ,ɐ,Π)Ù{ˊ(ǲ,Ƈ);ä ϗ<K>b9{}ø(ѿ K Ȟ)b9.ơ ѓ{{Ɛ,ō,Ǜ Ƭ.b â},Ǜ Ƭ.ʰ};run(į,ä Ɏ(Ƿ,0,0),b9,ŧ,ǁ);}ċ ä Ɏ source(ĝ in,Õ ƍ,ƿ){if(ƍ>n||in Û>n-ƍ)ǩ Ū ä Ɏ(in â,in Û,ƍ);}Ϣ run(ҽ,Ù ä Ɏ&in,Ϧ,Ö ŧ,Π)Ù{Ɵ;ʐ(ư)Ĩ ä ǆ Ŝ(Ɛ,ō,b9)Ω{Ι(v,h,ǁ,ō,Ʈ(ŧ));}ŝ{ƽ 2;radix2_halves(Ŝ,in,ʰ,nv,Ɛ)Њ(ƭ::Ƚ)Ӥ(v,h,ǁ,ŧ);}}Ő Combine>Ĉ Ɩ(Ȩ,Һ,Ҡ,ƿ,Combine җ)Ù{Ɵ;ʐ(n)Ƥ μ(kR);ɥ;Ε(Ļ Ŭ,Ō(a),Ϙ(Ɛ,0))ø(ә n
/8 ė p){if(p+1<n/8)Ε(Ļ[(p+1)&1],Ō(ϋ p+8),Ϙ(Ɛ,p+1))ι Ŏ=ń(ŭ(ǽ(Ļ[p&1],b Ƨ p),μ),kP);Š(į Ƨ p,ů(җ(Ō(į Ƨ p),Ŏ)));}}ý lg_max_;Ņ Ɛ,*ō;};}ā ē{Ϥ{Ǹ Ž(Ɣ p){Ú ɂ(õ ġ*>(p));}í ū(Ņ p,ɍ){Ψ(ț(p),x);}Ǹ Ŕ(ĭ a,ĭ b){ȱ,p=ɟ;н=ß(a,b);ĭ ȇ=Ѕ;Ĥ Ú ǝ;}Ũ Indices{Ϳ Indices(ȕ){ћ x Ҹ(Ö i=0;i<8 Į)x ƈ Ö((ŉ(k+i)<<32)Ұ);value_=Ō(x);}ĭ Ĵ()Ù{Ú value_;}Ĉ next(){value_=ń(ƫ(value_,ĕ(kStep)),kP);}Ⱦ ċ ӂ kStep=Ö((ŉ(8)<<32)Ұ);ĭ value_;};ԃ scalar_inverse(Ö x){Ú ã ǹ(x,ӓ);}Ő A>Ĉ ɗ(Õ ù,ɐ,Ù A&a,Ö Ÿ=1){ɿ ɤ=32 Ȣ n=ǲ ĺ(n ε)Ú Ȣ Ǿ=(n+ɤ-1)/ɤ,ȣ=ɤ*(Ǿ-1)ι one=ĕ(1),ɰ=ĕ(ɤ);ɍ ɛ ø(ҷ)x ӑ=ň(ĕ(Ö(ù Ƨ v)),ɜ)ǌ valid=[n](Õ at)Ý{ġ т=ň(ĕ(Ö(at)),ɜ)Ū σ(ĕ(Ö(n)),т);}ǌ masked=ѯ(ĭ Ĵ,Õ at)Ý{Ú at+8<=n?Ĵ:_mm256_blendv_epi8(one,Ĵ,valid(at));};ĭ ʜ ɛ ø(ҷ)ʜ ӑ=masked(x ӑ,8*v)ø(Õ b=ɤ;b<n;b+=ɤ){Ć(ҷ){ū(Ԁ b-ɤ Ƨ v,ʜ ӑ);x ӑ=ň(x ӑ,ɰ);ʜ ӑ=Ŕ(ʜ ӑ,masked(x ӑ,b Ƨ v));}}ћ o4[ɤ],ć[ɤ]ø(ҷ)Š(o4 Ƨ v,ů(ʜ ӑ));Ö m3=1 ø(Õ l Ē l<ɤ ė l){ć[l]=m3;m3=ʀ(m3,o4[l]);}m3=ʀ(scalar_inverse(m3),ʀ(kR,Ÿ))ø(Õ l=ɤ;l-->0;){ć[l]=ʀ(ć[l],m3);m3=ʀ(m3,o4[l]);}ĭ ђ ɛ ø(ҷ)ђ ӑ=Ō(ć Ƨ v)ǌ put=ѯ(Õ at,ĭ l4)Ý{ġ q=ů(Ŕ(l4,a(at
)))ĺ(at+8<=n)Ú ū(Ԁ at,q)ӎ(õ ý*>(Ԁ at),valid(at),q);}ø(Õ b=ȣ;b>0;b-=ɤ){Ć(ҷ){Ģ at=b Ƨ v ĺ(at<n)put(at,Ŕ(Ž(Ԁ at-ɤ),ђ ӑ));ђ ӑ=Ŕ(ђ ӑ,masked(x ӑ,at));x ӑ=ê(x ӑ,ɰ);}}Ć(ҷ)if(8*Õ(v)<n)put(8*v,ђ ӑ);}}í Ǟ(О,Ĝ d){ѻ n=d Û,ς(d ȝ,ů(Ŕ(Ž(f ȝ+1),ȯ.Ĵ())))ø(ɮ=full Ȝ)d ƈ i+1<Ǳ?ʀ(Ö(i+1),f Υ):0;}í ɗ(ĝ a,Õ ù,Ĝ q){if(ư!=Ѹ||ù ε||Ў ư>ϴ)ǩ Ȣ n=ư;ä ɗ(ù,q,[a,n](Õ at)Ý{if(at+8<=n)Ú ä Ž(a Ʒ at);ћ ɼ ɚ={};Ȫ(a ʷ at,a ƃ,ɼ)Ū ä Ō(ɼ);});}}ā ē{Ϥ{ò ɿ kInverseBase=32;í Ѩ(О,Ĝ g){Ę ʀ;ğ inv0=ã ǹ(f Ŭ,ӓ);g Г inv0 ø(ɮ=1 ϼ ȸ Į){ŉ ʂ Ɠ(ʍ=1;j<=ĥ(i,Ǳ-1)ų){ʂ+=ŉ(f Ƃ)*g[i-j]Ұ;}g ƈ Đ(Ö(ʂ Ұ?Ͼ ʂ Ұ:0),inv0);}}}ò ý inverse_log(ƿ){Ú Ŋ(ʵ,ý(ʌ(ʘ(n,2)-1)));}Ί ȵ(ƿ){Ú 2 ӌ(Ǽ inverse_log(n));}í inverse_step(Ƶ,О,ĝ g,Ĝ to,Ĝ gt,Ĝ Ǡ){Ģ k=ȸ Ȇ(g,0,gt);t.Ŷ(f Ȓ(ĥ(Ӷ,Ǳ)),0,Ǡ,gt,ƨ);t.Ŷ(Ǡ Ė(k),k,Ǡ,gt,ƨ,ä Ͼ 1)ĺ(to â!=Ǡ Ʒ k)ĵ(Ǡ ʊ(k),to Û,to ü);}í ć(Ƶ,О,Ĝ g,Є){Ģ n=ȸ;ȕ Ч(n,ä kInverseBase);ä Ѩ(f,g Ȓ(k))ø(;k<n;k*=2){Ģ ǯ=Ӷ;inverse_step(t,f,g Ȓ(k),g Ė(k,ĥ(k,n-k)),ĸ Ȓ(ǯ),ӿ(à(ǯ),ǯ));}}}ā ē{Ϥ{ò ɿ kLogBase=64;ò ɿ kLogBlocks=4;í log_direct(О,Ĝ g){Ę ʀ;Ö a[kLogBase]={},ђ[kLogBase]=
{0,1},kg[kLogBase]={}ԕ(f ü,ĥ(Ǳ,ȸ),a)ӱ Ŭ Ɠ(ɮ=1 ϼ ȸ Į){ŉ ʂ=Đ(Ö(i),ɨ)ø(ʶ i Ȟ)ʂ+=Ͼ ŉ(kg Ƭ)*a[i-k]Ұ;kg ƈ Ö(ʂ Ұ);g ƈ Đ(kg Ĕ,ђ Ĕ);}}Ί log_block(ƿ){Ú ʘ(32,ɬ((Ԕ+kLogBlocks-1)/kLogBlocks));}Ί log_blocks(ƿ){Ú(Ԕ+ɣ-1)/ɣ;}í subtract_from_derivative(О,Õ ù,ȡ,Ȩ,Õ æ){Ģ g6=Ǳ>Ԏ?ĥ(þ,Ǳ-ù-1):0,Ͻ{ġ d=ů(Ŕ(Ž(f Ʒ Ԏ+c),ȯ.Ĵ()))Ӻ}Ć(Õ c=full;c<þ ė c){ğ d=c<g6?ʀ(Ö(Ԏ+c),f[Ԏ+c]):0;į[c]=(d+Ͼ į[æ+c])Ұ;}}Ɯ ToeplitzBottom{Ǐ Ś,Ƚ=Ƈ;М,*ô,*w1,*w2,*q0,*q1;Ņ w3;ĉ Ĉ ѐ(Õ g,ʅ(&Ļ)Ѐ ɛ)Ù{ĭ ʂ ɛ,d1 ɛ,d3 ɛ;
#pragma GCC unroll 4
Κ{Ģ leaf=8*(4*g+t)ι v2=Ō(w2+leaf);ʂ Ѱ=ń(ƫ(Ō(q0+leaf),Ō(q1+leaf)),kP);d1 Ѱ=ɸ(Ō(w1+leaf),v2);d3 Ѱ=ɸ(Ō(w3+leaf),v2);}ʿ(Ļ Ŭ,ʂ,ķ,g);ʿ(Ļ ƹ,d1,ķ,g);ʿ(Ļ ǥ,d3,ķ,g);}Ǣ{ɥ Ѐ ɛ;Ȃ{ɕ Ȣ g=Ў j ǌ&w=Ļ[j&1];ϣ;
#pragma GCC unroll 4
Κ{Ģ leaf=8*(4*g+t)ι m1=ǽ(w Ŭ Ѱ,w2+leaf);f Ѱ ӏ(ƫ(m1,ǽ(w ƹ Ѱ,q1+leaf)));Š(w3+leaf,ʗ(ƫ(m1,ǽ(w ǥ Ѱ,q0+leaf))));}τ(f,Τ(ô,g));
#pragma GCC unroll 4
Κ Ӿ}}};Ɯ SideProductBottom{Ǐ Ś,Ƚ=Ƈ;Ș term;Ɣ side;Ǣ{ɥ ɛ;term.Ȃ{if(j+1<þ)term.ѐ(Ў j+1,Ļ[(j+1)&1])Ȣ g=Ў j;ϣ;
#pragma GCC unroll 4
Κ{Ģ leaf=8*(4*g+t);f Ѱ ӏ(ƫ(ǽ(Ļ[j&1]Ѱ,term.Ŏ.b+(leaf-8*term.Ŏ.origin)),Ō(side+leaf)));}τ(f,Τ(term.Ŏ.ô,g));
#pragma GCC unroll 4
Κ Ӿ}}};Ί log_buffers(ƿ){Ú ʘ(log_blocks(n)+2,3);}ò ī mint_scalar(Ī x)Ш Ǹ mint_load(ˢ f,Õ at){if(at+8<=Ǳ){ĭ Ĵ;Ǵ(&Ĵ,f Ʒ at,ƛ(Ĵ))Ū ń(Ĵ,kP);}Ť ī Ƴ ɚ={}ø(Ů 8&&at+i<Ǳ Į)Ƴ ƈ mint_scalar(f[at+i])Ū Ō(Ƴ);}Ũ MintSource{Ϳ MintSource(ˢ f):f_(f),full_(Ǳ/8),Х((Ǳ+7)/8){}ĭ ŕ()(Õ v)Ù{if(v<full_){ĭ Ĵ;Ǵ(&Ĵ,f_ ʥ v,ƛ(Ĵ))Ū ń(Ĵ,kP);}if(v>=Х)Ú đ Ū edge(f_,8*v);}Ǝ within(І,п)Ù{Ú Х ε||(lo ε&&Х<=hi);}Ⱦ Ɨ ċ ĭ edge(ˢ f,Õ at){Ú mint_load(f,at);}ˢ f_;Õ full_,Х;};Ɨ í mint_forward_top4(MintSource in,ҳ į,Ϩ,М){forward_top4_body(in,į,h,ķ);}Ɨ í mint_forward_top2(MintSource in,ҳ į,Ϩ){forward_top2_body(in,į,h);}í mint_forward(Ƶ,ˢ f,ɐ){Ù MintSource in(f)Ĩ ä ǆ Ŝ(Ȱ,ɧ,ǂ{Ȱ})Ȣ ӵ ǡ{ƽ 4;mint_forward_top4(in,v,h,Ȱ)ø(Õ q Ē q<4 ė q)ő(į ʥ q*h,h,q);}ŝ{ƽ 2;mint_forward_top2(in,v,h);ő(į â,h,0);ő(į ʥ h,h,1);}}í mint_derivative(ˢ f,Ĝ d){ѻ n=d Û,ς(d ȝ,ů(Ŕ(mint_load(f,i+1),ȯ.Ĵ())))ø(ɮ=full Ȝ)d ƈ i+1<Ǳ?ʀ(Ö(i+1),mint_scalar(f Υ)):0;}í mint_subtract_from_derivative(ˢ f,Õ ù,ȡ,Ȩ,Õ æ){Ģ g6=Ǳ>Ԏ?ĥ(þ,Ǳ-ù-1):0,Ͻ{ġ d=ů(Ŕ(mint_load(f,Ԏ+c),ȯ.Ĵ()))Ӻ}Ć(Õ c=full
;c<þ ė c){ğ d=c<g6?ʀ(Ö(Ԏ+c),mint_scalar(f[Ԏ+c])):0;į[c]=(d+Ͼ į[æ+c])Ұ;}}Ɯ NoLogTransform{Ĉ ŕ()(Õ,ĝ)Ù{}};Ő Input,Ũ Sink,Ũ ғ>ˮ log_derivative_impl(Ƶ,Input f,ƿ,Є,ĝ ʜ,Ù Sink&sink,Ù ғ&Ǩ={}){Ģ k=ɣ,ǯ=Ӷ,Ǿ=log_blocks(n)ǌ buffer=[&ĸ,ǯ](ɮ)Ý{Ú ӿ(i ӌ(ǯ),ǯ);}ǫ ht=buffer(0),q0=buffer(Ǿ)ǌ Ļ=[&buffer](Õ t)Ý{Ú buffer(t);};ć(t,ʜ,ht Ȓ(k),ӿ(à(ǯ),ȵ(k)))Ȇ(ht Ȓ(k),0,ht)ø(Õ s=1;s<Ǿ ė s){Ģ ʝ Ч(Ǳ,(s-1)*k)Њ(ģ Input,Ğ ǣ>>)mint_forward(t,f Ė(ʝ,ĥ(Ǳ-ʝ,ǯ)),Ļ(s))ї t.ı(f Ė(ʝ,ĥ(Ǳ-ʝ,ǯ)),0,Ļ(s));}Ć(ǈ Ǿ ų){Ģ ù=j*k,þ Ч(k,Ԕ-ù)ǫ Ǡ=j ε?q0:buffer(Ǿ+1)ĺ(j ε){Ĺ(ģ Input,Ğ ǣ>>)mint_derivative(f Ȓ(ĥ(Ǳ,þ+1)),Ǡ Ȓ(þ))ї Ǟ(f Ȓ(ĥ(Ǳ,þ+1)),Ǡ Ȓ(þ));}ŝ{if(j у){Ɗ(Ļ(1),q0,Ǡ,ƨ);}м(Ǿ==3){Ù ɝ Ǜ ǥ={{Ļ(2),q0},{Ļ(1),Ǡ}};t.Ŗ(Ǜ,Ǡ,ƨ);}м(j==2){t.Ɇ(ToeplitzBottom{Ȱ,ɧ,Ļ(1)â,Ļ(2)â,q0 â,Ǡ â,Ļ(3)â},Ǡ,ƨ);}ŝ{Ù Ș Ŏ{{Ȱ,ɧ,Ǡ â},Ļ(1)â};t.Ɇ(SideProductBottom{Ŏ,Ļ(3)â},Ǡ,ƨ);}Ĺ(ģ Input,Ğ ǣ>>)mint_subtract_from_derivative(f,ù,þ,Ǡ â,k)ї subtract_from_derivative(f,ù,þ,Ǡ â,k);}t.Ŷ(Ǡ Ȓ(þ),0,Ǡ,ht,Ʃ);sink(ù,ĝ(Ǡ Ȓ(þ)))ĺ(j+1<Ǿ){t.ı(Ǡ Ȓ(k),0,Ǡ);Ǩ(j,ĝ(Ǡ));}}}Ő Sink,Ũ ғ=NoLogTransform
>ˮ Γ(Ƶ,О,ƿ,Є,Ù Sink&sink,Ù ғ&Ǩ={}){log_derivative_impl(t,f,n,ĸ,f,sink,Ǩ);}Ő Sink,Ũ ғ=NoLogTransform>ˮ mint_log_derivative(Ƶ,ˢ f,ƿ,Є,ĝ ʜ,Ù Sink&sink,Ù ғ&Ǩ={}){log_derivative_impl(t,f,n,ĸ,ʜ,sink,Ǩ);}ò ý log_derivative_log(ƿ){Ú ĩ(2*ɣ);}Ί log_derivative_scratch(ƿ){Ú log_buffers(n)ӌ(2*ɣ);}}ò ý log_log(ƿ){Ú n<=ä kLogBase?ʵ:ĩ(2*ä ɣ);}Ί log_scratch(ƿ){Ú n<=ä kLogBase?0:ä log_buffers(n)ӌ(2*ä ɣ);}í log(Ƶ,О,Ĝ g,Є){ѻ n=ȸ ĺ(n<=kLogBase)Ú log_direct(f,g);Γ(t,f,n,ĸ,[g](Õ ù,ĝ q)Ý{ä ɗ(Ԏ,g Ė(Ԏ,Ѹ),[q](ɮ)Ý{Ú Ō(q ȝ);});});g Ŭ Ē}}ā ē{í native_log(ǣ*in,Õ ʭ,л,ƿ,Ǝ g7){ƚ(į,n);į η(0)Ȣ c Ч(ʭ,n)Ȋ lg=ä log_derivative_log(n)Ĩ Ǝ ӣ=n>=262144 ǌ k7=ӣ?ĥ(c,ä ɣ):c й(ɠ(k7)Я(log_scratch(n)))Ǭ f ф(k7),ĸ ф(log_scratch(n))ø(Ů k7 Į){Ù ī v=Ө;f ƈ ĥ(v,v-Ƀ);}ʦ sink=[&į,g7,n](Õ ù,ĝ q)Ý{if(g7){ł*Ǡ=const_cast<Ņ>(q â)+ä ɣ;ä ɗ(Ԏ,{Ǡ,Ѹ},[q](ɮ)Ý{Ú ä Ō(q ȝ);},ã kR);}ʦ*Ƴ=g7?q Ʒ ä ɣ:q â ĺ(g7)ž(į,Ƴ,Ѹ)ї append_fps_words ə(į,Ƴ,Ѹ);}ĺ(ӣ)ä mint_log_derivative(t,{in,c},n,ĸ,f,sink)ї ä Γ(t,f,n,ĸ,sink);}}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
Ӏ{ā ē{Ϥ{Ǒ Ь}}ā ē{Ϥ{ò ɿ ϱ=64;í exp_direct(ĝ q,Ĝ g){Ę ʀ;Ö ђ[ϱ]={0,1}ӱ Г 1 ø(ɮ=1 ϼ ȸ Į){ŉ ʂ Ɠ(ȕ=1;k<=i Ȟ)ʂ+=ŉ(q Ƭ)*g[i-k]Ұ;g ƈ Đ(Ö(ʂ Ұ),ђ Ĕ);}}í multiply_by_index(ĝ a,Õ ù,ɐ){Ģ n=ǲ,full=n/8*8;Indices ȯ(ù)ø(Ů full ƞ,ȯ.next())ū(į ȝ,ů(Ŕ(Ž(a ȝ),ȯ.Ĵ())))ø(ɮ=full Ȝ)į ƈ ʀ(Ö((Ў i)Ұ),ɨ);}Ί exp_length(ƿ){Ú Ǽ Ŋ(ʵ+1,ý(ʌ(ʘ(n,2)-1)));}}ò ý exp_log(ƿ){Ú ĩ(ä β)-1;}Ϥ{Ɯ QTransforms{ĝ ʗ,block2;};Ɯ NoQTransforms{};Ϣ product_to(Ƶ,ɐ,Ù Ɏ&in,Ϧ,Ö ŧ,Π,Ĝ to){Ģ ӵ Ĩ ǆ Ŝ(Ȱ,ɧ,b9)Ƥ s(ŧ)ǌ put=[to,&s](ʍ,ɍ)Ý{ġ y=ń(ŭ(x,s),kP)ĺ(8*j+8<=to Û)Ú ū(to ʥ j,y)ĺ(8*j>=to Û)Ú ι т=ɜ ι mask=σ(ĕ(Ö(to Û-8*j)),т)ӎ(õ ý*>(to ʥ j),mask,y);}Ĩ Ǝ lower=ǁ==Ʃ ǡ{ƽ 4;ʠ forward_top4(in,v,h,Ȱ)ø(Õ q Ē q<4 ė q)ő(į ʥ q*h,h,q)Ƥ z(ɧ ƹ,ɧ[9])ʋ{ġ ab ӏ(ƫ(v Ƃ,Ӊ)),cd ӏ(ƫ(v ɋ,v Ɋ))ι amb ӏ(ǘ(v Ƃ,Ӊ)),cmd Ҝ(ǘ(v ɋ,v Ɋ),z);put(j,lower?ϑ:Ќ);put(j+h,lower?ϐ:Ϗ);}}ŝ{ƽ 2;radix2_halves(Ŝ,in,į â,nv,Ȱ)ʋ put(j,lower?ƫ(v Ƃ,Ӊ):ǘ(v Ƃ,Ӊ));}}Ő Given>ˮ exp_last_step(Ƶ,Ĝ lo,Ĝ g,Ό,Ǝ ù,Ĝ gt,ĝ ht,Ĝ w,Ĝ Ǡ,Ù Given&e7){Ģ n=ȸ,Ơ=m/2,ɼ=n-m,þ Ч(ɼ,Ơ)Ĩ Ǝ ʓ=ɼ>Ơ ǫ g0t=gt ʴ,glt=gt ч,own=w ʴ,r=w ч;ĝ qt
=own,q2t=lo;Ǝ e9=Ś,e8=Ś Њ(ģ Given,QTransforms>){e9=!e7.ʗ ű,e8=!e7.block2 ű ĺ(e9)qt=e7.ʗ ĺ(e8)q2t=e7.block2;}ʦ q=[g,n](ɮ)Ý{if(i+8<=n)Ú Ž(g ȝ);ћ x ɚ={};Ȫ(g ʊ(i),g ƃ,x)Ū Ō(x);}ĺ(ù)t.ı(g Ȓ(Ơ),0,g0t)Ȇ(g ʴ,0,glt)ĺ(e9){Ɗ(qt,glt,r,ʓ?ȉ:Ʃ);}м(ʓ){t.ı(lo,0,own);Ɗ(own,glt,r);}ŝ{t.Ŷ(lo,0,r,glt,Ʃ);}t.ı(r Ȓ(Ơ),0,Ǡ);Ɗ(ht,Ǡ,Ǡ,Ʃ);ä ɗ(m,r Ȓ(þ),ѯ(ɮ)Ý{Ҽ ń(ƫ(Ō(Ǡ ȝ),q(m+i)),kP)Ū ƫ(x,ê(ɟ,Ō(lo ȝ)));})ĺ(ʓ&&!e8)t.ı(g Ė(m,Ơ),0,lo)Ȇ(r Ȓ(þ),0,Ǡ);ğ ŧ=Ȍ ƹ[ĩ(m)]Ĩ Ɏ none(Ƿ,0,0);product_to(t,Ǡ,none,Ș{{Ȱ,ɧ,Ǡ â},g0t â},ŧ,Ʃ,g Ė(m,þ))ĺ(!ʓ)Ú Ȇ(g Ė(m,Ơ),0,Ǡ)Ĩ ɝ Ǜ[]={{Ǡ,qt},{q2t,glt}};t.Ŗ(Ǜ,Ǡ,ƨ);Indices ȯ(Ơ)ø(ɮ=Ơ ϼ m ƞ,ȯ.next()){ġ xg=Ŕ(Ž(g ȝ),ȯ.Ĵ())ι ʂ=ƫ(Ō(r ȝ),Ō(Ǡ ȝ));Š(r ȝ,ů(ê(ƫ(ʂ,П),xg)));}t.ı(r Ė(Ơ,Ơ),0,own);Ɗ(ht,own,own,Ʃ);ä ɗ(m+Ơ,r Ȓ(ɼ-Ơ),ѯ(ɮ)Ý{Ú ƫ(Ō(own ȝ),q(m+Ơ+i));})Ȇ(r Ȓ(ɼ-Ơ),0,own);product_to(t,own,none,Ș{{Ȱ,ɧ,own â},g0t â},ŧ,Ʃ,g Ė(m+Ơ,ɼ-Ơ));}Ő Given=NoQTransforms>ˮ exp_newton(Ƶ,Ĝ lo,Ĝ g,Є,Ù Given&e7={}){Ģ n=ȸ,ǯ=2*lo Û ǌ ś=[&ĸ](Õ č)Ý{Ù Ĝ s=ĸ Ȓ(č);ĸ=ӿ(à(č))Ū s;}ǫ h=ś(ǯ/2),gt ӻ,ht=ś(ǯ/2),w ӻ;Ό=ϱ;Ѩ(g,h Ȓ(m/2))Ȇ(h Ȓ(m/2)
,0,ht ʴ)ø(;2*m<n;m*=2){Ģ Ơ=m/2 ǫ e5=gt ʴ,f6=ht ʴ,o8=w ʴ Ȇ(g ʴ,0,e5);Ɗ(e5,f6,o8,ƨ);ğ j0=ʀ(Ȍ ƹ[ĩ(m)],Ͼ 1);product_to(t,o8,Ɏ(o8 Ʒ Ơ,Ơ,Ơ),ȥ{Ȱ,ɧ,f6 â},j0,ƨ,h Ė(Ơ,Ơ));t.forward_product(lo ʴ,0,o8,e5);t.ć(o8,gt ч)Ȇ(h ʴ,0,ht ϟ);t.ɇ(gt ч,0,w ч);Ɗ(w ϟ,ht ϟ,w ϟ,Ʃ);ä ɗ(m,w ч,ѯ(ɮ)Ý{Ҽ ń(ƫ(Ō(w ȝ),Ō(lo Ʒ m+i)),kP)Ū ƫ(x,ê(ɟ,Ō(lo ȝ)));});t.ɇ(g ʴ,0,gt ч);product_to(t,w ϟ,Ɏ(w Ʒ m,m,m),ȥ{Ȱ,ɧ,gt â},Ȍ ƹ[ĩ(2*m)],ƨ,g ч);}exp_last_step(t,lo,g,m,m==ϱ,gt,ht,w,h,e7);}Ί exp_newton_scratch(ƿ){Ģ ǯ=β Ū 2 ӌ(ǯ)ʳ à(ǯ/2);}}Ί exp_scratch(ƿ){Ú à(ä β/2)+ä exp_newton_scratch(n);}í exp(Ƶ,О,Ĝ g,Є){ѻ n=ȸ,m=β/2,æ Ч(Ǳ,n),Ԋ Ч(æ,m)ǫ lo=ĸ ʴ;multiply_by_index(f Ȓ(Ԋ),0,lo Ȓ(Ԋ));ż(lo ʊ(Ԋ),lo ƃ,0)ĺ(æ>m)multiply_by_index(f Ė(m,æ-m),m,g Ė(m,æ-m))ĺ(n>Ŋ(æ,m))ż(g ʊ(Ŋ(æ,m)),g ƃ,0);exp_direct(lo,g Ȓ(ĥ(n,ϱ)))ĺ(n>ϱ)exp_newton(t,lo,g,ӿ(à(m)));}}ā ē{Ϥ{έ kSum>Ĉ times_x(М,Һ,Ҡ,Ö e,Ö a9,Ö beta,ɐ){Ģ ȟ=ǲ/8 Ƥ ŧ(e)ι m0=ŏ(7,0,1,2,3,4,5,6),ȇ=ŏ(0,-1,0,-1,0,-1,0,-1);ğ c9 ǥ={(a9+beta)Ұ,(a9+Ͼ beta)Ұ}ø(ә ȟ;p+=8){Ù Ǝ ʓ=p>=ȟ/2 Ĩ я r=entries(ķ,p/16*8)ι pick=p%16?ŏ(4,
4,5,5,6,6,7,7):ŏ(0,0,1,1,2,2,3,3)ι rw=Ľ(r.w,pick),rq=Ľ(r.q,pick)Ĩ я w{ĳ(rw,ê(ɟ,rw),0xAA),_mm256_xor_si256(rq,ȇ)};ĭ y ɚ;
#pragma GCC unroll 8
Ć(Ů 8 Į){ĭ c=Ō(ϋ(p+i))Њ(kSum)c=ʓ?ɸ(c,Ō(b Ƨ(p+i))):ń(ƫ(c,Ō(b Ƨ(p+i))),kP);y ƈ ń(ŭ(c,ŧ),kP);}ġ t01 Ȁ(y Ŭ,y ƹ),t23 Ȁ(y ǥ,y Ѐ)ι t45 Ȁ(y ɛ,y[5]),t67 Ȁ(y[6],y[7])ι u03 ǿ(t01,t23),u47 ǿ(t45,t67)ι z=ů(ƫ(ŭ(Ł(u03,u47,0x31),w),ĕ(c9[ʓ])))ι zh=Ł(z,z,0x11)ι h2 ɚ={z,Ͳ(z,4),Ͳ(z,8),Ͳ(z,12),zh,Ͳ(zh,4),Ͳ(zh,8),Ͳ(zh,12)};
#pragma GCC unroll 8
Ć(Ů 8 Į)Š(į ʥ(p+i),ĳ(Ľ(y Ĕ,m0),h2 Ĕ,1));}}}ò ý power_log(ƿ){Ú Ŋ(ä log_derivative_log(n),exp_log(n));}Ί ҫ(ƿ){Ú 3 ӌ(ä β/2)+Ŋ(ä log_derivative_scratch(n),ä exp_newton_scratch(n));}í ǹ(Ƶ,О,Ö e,Ö c,Ĝ g,Є){ѻ n=ȸ,m=β/2,k=m/2,ď=à(m)ĺ(f ű||f Ŭ ε)ǩ ǫ lo=ĸ ʴ,ɼ=ӿ(3 Ԍ)ǌ q=ѯ(ɮ)Ý{Ú i<m?lo ȝ:g ȝ;}Ƥ Ÿ(e)Ĩ Ǝ l7=m>=128&&ɣ==k&&log_blocks(n)>=3 Ĩ Ǝ block2=l7&&log_blocks(n)==4&&n-m>k ǫ qt=ӿ(ď,l7?m:0)ǫ q2t=ӿ(2 Ԍ,block2?m:0);Ɣ d0t=Ƿ;lo Ŭ Ē if(n>=2)Γ(t,f,n,ɼ,ѯ(Õ ù,ĝ d)Ý{ɮ Ɠ(ʲ d Û ƞ){Ģ at=Ԏ+i ι x=ń(ŭ(Ō(d ȝ),Ÿ),kP)ĺ(at+8<=m||at>=m){ū(q(at),x);}ŝ{ћ č ɚ;Š(č,x)ø(ǈ 8 ų)*q(at+j)=č Ƃ;}}Ć(ϼ d Û Į)*q(Ԏ+i)=ʀ(d Ĕ,e);},ѯ(ʍ,ĝ dt)Ý{if(j ε)d0t=dt â ĺ(j у&&l7)times_x ԍ(Ȱ,d0t,dt â,e,Ͼ*q(m),0,qt)ĺ(j==2&&block2)times_x ə(Ȱ,dt â,Ƿ,e,*q(m),Ͼ*q(m+k),q2t);})ǫ start=g Ȓ(ĥ(n,ϱ));exp_direct(lo,start)ø(Ö&x:start)x=ʀ(x,c)ĺ(n>ϱ)exp_newton(t,lo,g,ɼ,QTransforms{qt,q2t});}}ā ē{Ϥ{έ ŵ>Ɨ Ī exp_tail_coefficient(ǣ*in,Õ ʭ,ǣ*g,ƿ){Ģ þ Ч(n,ʭ?ʭ-1:0),full=þ/8*8 ι reversed=ʞ,ni=ĕ(ì ƥ kNI),p=ɟ;ĭ o4=đ;Indices ȯ(1);ʍ Ɠ(;j<full;){н=đ,ȇ=Ŧ Ȣ end Ч(full,j+64)ø(
;j<end Ӕ,ȯ.next()){ĭ a,b;Ǵ(&a,in+j+1,ƛ(a));Ǵ(&b,g+n-j-8,ƛ(b));a=ń(a,kP)Њ(!ŵ)a=ů(Ŕ(a,ȯ.Ĵ()));b=Ľ(ń(b,kP),reversed);Ŧ=ð(Ŧ,ß(a,b));ȇ=ð(ȇ,Ѕ);}Ĥ ġ Ĵ=ů(ǝ);o4 ӏ(ƫ(o4,Ĵ));}o4 ӏ(ƫ(o4,Ł(o4,o4,1)));o4 ӏ(ƫ(o4,Ƒ(o4,0x4E)));o4 ӏ(ƫ(o4,Ƒ(o4,0xB1)));Ī ă=һ(Ö(Ҙ(ɦ(o4))))ø(;j<þ ų){Ī Ĵ=in[j+1]*g[n-j-1]Њ(!ŵ)Ĵ*=Ī(j+1);ă+=Ĵ;}Ú ă/Ī(n);}έ ŵ>Ɨ Ĉ import_exp_input(ǣ*in,Õ ù,ȡ,Ņ to){Ά;ɮ Ē Œ undo(ã ǹ(kR,ӓ))ѵ ȯ=ƫ(ĕ(Ö(ù)),ɜ)ø(ʲ þ ƞ,ȯ=ƫ(ȯ,ĕ(8))){ĭ Ĵ;Ǵ(&Ĵ,in+Ў i,ƛ(Ĵ))Њ(ŵ)ū(to+i,ń(ŭ(Ĵ,undo),kP))ї ū(to+i,ń(Ŕ(Ĵ,ȯ),kP));}Ć(ϼ þ Į){Ĺ(ŵ)to ƈ in[Ў i]Ȅ ї to ƈ ʀ(in[Ў i]Ȅ,Ў i);}}}í native_exp_regular(ǣ*in,Õ ʭ,л,ƿ,Ǝ ŵ){Ģ m=ä β/2,æ Ч(ʭ,n),Ԋ Ч(æ,m)Ȋ lg=exp_log(n)й(ɠ(n)Я(exp_scratch(n)))Ǭ g к,ĸ ф(exp_scratch(n))ѵ lo=ĸ ʴ ǌ import=ѯ(Õ ù,ȡ,Ņ to){if(ŵ)ä import_exp_input ԍ(in,ù,þ,to)ї ä import_exp_input ə(in,ù,þ,to);};import(0,Ԋ,lo â);ż(lo ʷ Ԋ,lo ƃ,0u)ĺ(æ>m)import(m,æ-m,g Ʒ m)ĺ(n>Ŋ(æ,m))ż(g ʷ Ŋ(æ,m),g ƃ,0u);ä exp_direct(lo,g Ȓ(ĥ(n,ä ϱ)))ø(ł&x:g Ȓ(ĥ(n,ä ϱ)))x=ʀ(x,ä kR)ĺ(n>ä ϱ)ä exp_newton(t,lo,g,ӿ(à(m)));ž(į,g â,n);}í native_exp(ǣ*in,Õ ʭ,л,ƿ,
Ǝ ŵ=Ś){Ģ ʜ=Ø bit_floor(n),Η=n-ʜ ĺ(ʜ>=4096&&Η&&Η<=4){Ģ Ų=ǲ;ƚ(į,Ų+n);native_exp_regular(in,ʭ,į,ʜ,ŵ)ø(Õ at=ʜ;at<n ė at){ǣ Ĵ=ŵ?ä exp_tail_coefficient ԍ(in,ʭ,Ԁ Ų,at):ä exp_tail_coefficient ə(in,ʭ,Ԁ Ų,at);į Ӯ(Ĵ);}Ú;}native_exp_regular(in,ʭ,į,n,ŵ);}í native_power(ǣ*in,Õ ʭ,л,ƿ,Ö e,Ī c){Ƞ lg=power_log(n)Ȣ æ Ч(ʭ,n)й(ɠ(n)Я(ҫ(n)))Ǭ g к,ĸ ф(ҫ(n))ø(Ů æ Į){ʦ x=Ө;g ƈ ĥ(x,x-Ƀ);}ǹ(t,g Ȓ(æ),e,ĥ(c.v,c.v-Ƀ),g,ĸ);ž(į,g â,n);}}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā Ü ē{í append_scaled(ҽ,л,Ö ŧ){Ù ä Ʈ Ÿ(ŧ)ĺ(ư>į.max_size()-ǲ)throw Ø length_error("fps output");ƚ(į,ǲ+ư);ћ Ƴ[64]ø(Ů ư Ƽ 64){ʦ þ Ч<Õ>(64,ư-i);ʍ Ɠ(;j+8<=þ Ӕ)ä Š(Ƴ+j,ä ń(ä ŭ(ä Ž(a ȝ+j),Ÿ),Ƀ))ø(;j<þ ų)Ƴ ˉ ʀ(a[i+j],ŧ);ž(į,Ƴ,þ);}}í native_inverse(ǣ*in,Õ ʭ,л,ƿ){Ƞ lg=inverse_log(n)Ȣ ǯ=Ǽ lg,k=ǯ/2 Ȣ c Ч(ʭ,n),Ƅ=Ŋ(ǯ,ȵ(k))й(ɠ(ǯ)Я(Ƅ)Я(k))Ǭ a ф(ǯ),ĸ ф(Ƅ),g ф(k)ø(Ů c Į){Ù ī v=Ө;ө ĥ(v,v-Ƀ);}Ù О=a Ȓ(c)ǌ extend=ѯ(ĝ d6,Ĝ to,Ĝ Ǩ,Ĝ Ǡ){Ģ þ=d6 Û Ȇ(d6,0,Ǩ);t.Ŷ(f Ȓ(ĥ(2*þ,Ǳ)),0,Ǡ,Ǩ,ƨ);t.Ŷ(Ǡ Ė(þ),þ,Ǡ,Ǩ,ƨ,53925088)ĺ(to â!=Ǡ Ʒ þ)ĵ(Ǡ ʊ(þ),to Û,to ü);}Ȣ ƪ Ч(n,k),ƻ Ч(ƪ,ä kInverseBase);ä Ѩ(f,g Ȓ(ƻ))ø(Ů ƻ Į)g ƈ ʀ(g Ĕ,932051910)ø(ȡ=ƻ;þ<ƪ;þ*=2){Ģ æ=2*þ;extend(g Ȓ(þ),g Ė(þ,ĥ(þ,ƪ-þ)),ĸ Ȓ(æ),ӿ(à(æ),æ));}if(n<=k)Ú ž(į,g â,n);extend(g,a Ė(k,n-k),ĸ Ȓ(ǯ),a);ƚ(į,n);ž(į,g â,k);ž(į,a Ʒ k,n-k);}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā Ü ē{Ϥ{ò ɿ Ξ=64;í sqrt_direct(О,Ö c,Ĝ g){Ę ʀ;ğ ć=ã ǹ(Đ(2,c),ӓ);g Г c ø(ɮ=1 ϼ ȸ Į){ŉ ʂ=i<Ǳ?f Ĕ:0 ø(ʍ=1;j<i ų)ʂ+=Ͼ ŉ(g Ƃ)*g[i-j]Ұ;g ƈ Đ(Ö(ʂ Ұ),ć);}}Ǹ load_padded(О,Õ at){if(at+8<=Ǳ)Ú Ž(f Ʒ at);ћ x ɚ={}ĺ(at<Ǳ)Ȫ(f ʊ(at),f ƃ,x)Ū Ō(x);}Ǹ first_lanes(ȡ){Ú σ(ĕ(Ö(ĥ<Õ>(þ,8))),ɜ);}í store_first(Ņ p,ɍ,ȡ){if(þ>=8)Ú ū(p,x)ӎ(õ ý*>(p),first_lanes(þ),x);}Ǹ load_first(Ɣ p,ȡ){if(þ>=8)Ú Ž(p)Ū _mm256_maskload_epi32(õ Ƞ*>(p),first_lanes(þ));}ĉ Ǹ leaf_square(Ù ʅ&Ļ){ȱ,p=ɟ,p2=П;н,ȇ,bi,t0,t1,t2;
	#define Â(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,q,r) h(m,b) c(n,b,d) c(m,b,a) h(n,b)c(l,b,e)h(40(%[w]),b)c(l,b,g)c(24(%[w]),b,i)f(e,a,a)h(44(%[w]),b)c(k,b,e)f(g,d,d)f(i,a,a)h(48(%[w]),b)c(k,b,g)c(16(%[w]),b,i)f(e,a,a)h(52(%[w]),b)c(j,b,e)f(g,d,d)f(i,a,a)h(56(%[w]),b)c(j,b,g)c(8(%[w]),b,i)f(e,a,a)h(60(%[w]),b)c(4(%[w]),b,e)f(g,d,d)f(i,a,a)f(e,a,a)f(d,d,d)c(%[ni],a,e)c(%[ni],d,g)c(%[p],e,e)c(%[p],g,g)f(e,a,a)f(g,d,d)q($32,a,a)o($0xaa,d,a,a)r(%[p2],a,e)"vpminud %[t0], %[even], %[even]"
asm(Â(%[Ŧ],%[bi],Ò,%[ȇ],%[t0],Í,%[t1],Ï,%[t2],12(%[w]),20(%[w]),28(%[w]),32(%[w]),36(%[w]),Î,Ó,Ô)
#undef Â
:[Ŧ]"=&x"(Ŧ),[ȇ]"=&x"(ȇ),[bi]"=&x"(bi),[t0]"=&x"(t0),[t1]"=&x"(t1),[t2]"=&x"(t2):[w]"r"(Ļ.ɽ),[ni]"x"(ni),[p]"x"(p),[p2]"x"(p2),"m"(*õ ğ(*)[17]>(Ļ.ɽ)))Ū Ŧ;}Ɯ InverseSquareBottom{Ǐ Ś,Ƚ=Ƈ;М;Ɣ ô;Һ;ĉ Ĉ ѐ(Õ g,ʅ(&Ļ)ɛ)Ù{Ъ(Ļ,f,ķ,g);}Ǣ{ɥ ɛ;Ȃ{ɕ;ϣ;
#pragma GCC unroll 4
ҿ f Ѱ=leaf_square(Ļ[j&1]Ѱ);τ(f,Τ(ô,Ў j))ø(ý Ń)Ӿ}}};ò Ǝ ɷ(Ԅ){Ú ĩ(nv)%2 ε;}Ϣ ѧ(Ù ǆ<ƭ>&Ŝ,Ң,Ԅ){if(!ɷ(nv)){ő(a,nv/2,0)Ū ő(a+4*nv,nv/2,1);}Ć(Õ q Ē q<4 ė q)ő(a ʳ q*nv,nv/4,q);}Ϣ ϰ(Ƶ,Ù ǆ<ƭ>&Ŝ,Ң,Ԅ,Ǝ p7=Ƈ){Ù Ɏ in(a,4*nv,0)ĺ(!ɷ(nv))Ú radix2_halves(Ŝ,in,a,nv,Ȱ)ĺ(!p7)forward_top4(in,ț(a),nv/4,Ȱ);ѧ(Ŝ,a,nv);}í inverse_top(Ƶ,ҳ v,Ԅ,Π,Ö ŧ){if(ɷ(nv))Ú inverse_top4(v,nv/4,ǁ,ɧ,Ʈ(ŧ));Ӥ(v,nv/2,ǁ,ŧ);}Ą<Half kOutput,Ũ Visit>ˮ ǔ(Ƶ,ҳ v,Ԅ,Ö ŧ,Visit ϖ){è Ǝ kLower=kOutput!=ƨ,kUpper=kOutput!=Ʃ Ƥ s(ŧ)ĺ(ɷ(nv)){ƽ 4 Ƥ z(ɧ ƹ,ɧ[9])ʋ{ġ p0=v Ƃ,p1=Ӊ,p2=v ɋ,p3=v Ɋ ι ab ӏ(ƫ(p0,p1)),cd ӏ(ƫ(p2,p3))ι amb ӏ(ǘ(p0,p1)),cmd Ҝ(ǘ(p2,p3),z);ĭ lo ǥ={},hi ǥ={}Њ(kLower)lo Г ŭ(ϑ,s),lo ƹ Ҝ(ϐ,s)Њ(kUpper)hi Г ŭ(Ќ,s),hi ƹ Ҝ(Ϗ,s);ϖ(Ø true_type{},j,h,lo,hi);}Ú;}ƽ 2 ʋ{ĭ lo ƹ={},hi ƹ={}Њ(kLower)lo Г ŭ(ƫ(v Ƃ,Ӊ),s)Њ(kUpper)hi Г ŭ(ǘ(v Ƃ,Ӊ),s);ϖ(Ø false_type{},j,h,lo,hi);}}ˮ forward_lower4(ҳ v,ʍ,Ϩ,ĭ a,ĭ b,Œ&z){ġ zb Ҝ(b,z);v ˉ ƫ(a,b),Ӊ=ǘ(a,b),v ɋ=ƫ(a,zb),v Ɋ=ǘ(a,zb);}ˮ forward_upper4(ҳ v,ʍ,Ϩ,ĭ c,ĭ d,Œ&z){ġ mc=ê(ɟ,c),zmd Ҝ(ê(П,d),z);v ˉ ƫ(c,d),Ӊ=ǘ(c,d),v
 ɋ=ƫ(mc,zmd),v Ɋ=ǘ(mc,zmd);}ò Ʈ top_root(Ƶ){Ú Ʈ(Ȱ ƹ,Ȱ[9]);}Ǹ*И(ҽ){Ú ț(ʰ);}ԃ product_scale(ƿ){Ú Ȍ ƹ[ĩ(n)];}Ǹ Ʌ(О,Ό,Õ at,ĭ s){Œ Ơ((kP+1)/2)ι fs=ƫ(load_padded(f,at),load_padded(f,m+at))Ū ń(ŭ(ê(ƫ(s,П),fs),Ơ),kP);}í Ʌ(О,Ό,Õ ù,Ɣ s,Ȩ,ȡ){Ć(Ů þ ƞ)store_first(є i,Ʌ(f,m,Ў i,Ō(s+i)),þ-i);}í sqrt_step(Ƶ,О,Ό,Ņ g,Ņ h,Ĝ b,Ĝ ht){Ģ Ơ=m/2,nv=m/8 ǫ gt=b ʴ,w=b ч,Ǡ=b ϟ Ƥ z=top_root(t)Ĩ ȥ p5{Ȱ,ɧ,ht â};ğ ŧ=product_scale(m),m4=product_scale(2*m)Ȇ(ĝ(g,m),0,gt);ҳ v=И(w);ѧ(ǆ(Ȱ,ɧ,Ș{p5,gt â}),w â,nv);ǔ<ƨ>(t,v,nv,ŧ,ѯ(ł ɷ,ʍ,Õ q,ġ*,ġ*e){Ĺ(ɷ)forward_upper4(v,j,q,ń(e Ŭ,kP),e ƹ,z)ї v ˉ e Ŭ,v[j+q]=ê(П,e Ŭ);});ѧ(ǆ(Ȱ,ɧ,p5),w â,nv);ǔ<ƨ>(t,v,nv,ŧ,ѯ(ł ɷ,ʍ,Õ q,ġ*,ġ*x){ū(h+Ơ Ƨ j,ń(x Ŭ,kP))Њ(ɷ)ū(h+Ơ Ƨ(j+q),ń(x ƹ,kP));})Ȇ(ĝ(h,m),0,ht);v=И(gt);ѧ(ӕ,gt â,nv);ǔ<ȉ>(t,v,nv,ŧ,ѯ(ł ɷ,ʍ,Õ q,ġ*lo,ġ*hi){Ĺ(ɷ){v ˉ Ʌ(f,m,8*j,lo Ŭ),v[j+q]=Ʌ(f,m,8*(j+q),lo ƹ);v[j ʳ q]=Ʌ(f,m,Ơ Ƨ j,hi Ŭ);v[j+3*q]=Ʌ(f,m,Ơ Ƨ(j+q),hi ƹ);}ŝ{forward_lower4(v,j,q,Ʌ(f,m,8*j,lo Ŭ),Ʌ(f,m,Ơ Ƨ j,hi Ŭ),z);}});ϰ(t,ǆ(Ȱ,ɧ,p5),Ǡ â,2*nv);ǔ<Ʃ>(t,И(Ǡ),2*nv,m4,Ͷ{ū(g+m Ƨ j,ń(d Ŭ,kP))Њ(ɷ
)ū(g+m Ƨ(j+q),ń(d ƹ,kP));});}}ò ý sqrt_log(ƿ){Ú Ŋ(ʵ,ý(ʌ(ʘ(n,2)-1))-1);}Ί sqrt_scratch(ƿ){if(n<=ä Ξ)Ú 0 Ū 3 ӌ(Ǽ sqrt_log(n));}í sqrt_steps(Ƶ,О,Ö c,ҽ,Ĝ b,Ĝ ht){ѻ ǯ=ư ĺ(f ű||f Ŭ ε||ʀ(c,c)!=f Ŭ||ǯ<Ξ)ǩ;sqrt_direct(f,c,a Ȓ(Ξ));ǚ h=a Ʒ ǯ/2;ћ h0[Ξ/2];Ѩ(a Ȓ(Ξ/2),h0)ø(Ö&x:h0)x=x?Ͼ x:0 Ȇ(h0,0,ht Ȓ(Ξ))ĺ(ǯ>Ξ)ĵ(h0,Ξ/2,h)ø(Ό=Ξ;m<ǯ;m*=2)sqrt_step(t,f,m,ʰ,h,b ϟ,ht ϟ);}í sqrt_last_step(Ƶ,О,ĝ gt,ĝ ht,Ĝ Ǡ,Ĝ hi){ѻ m=gt Û,Ơ=m/2,ɼ=hi Û,nv=m/8 ĺ(ɼ ε||ɼ>m)ǩ;ğ ŧ=product_scale(m)Ƥ z=top_root(t)Ĩ ȥ p4{Ȱ,ɧ,gt â},p5{Ȱ,ɧ,ht â}Ĩ ǆ product_h(Ȱ,ɧ,p5);ҳ v=И(Ǡ)ǌ source=ѯ(ł ɷ,ʍ,Õ q,ł x){Ĺ(ɷ)forward_lower4(v,j,q,x(0,j),x(1,j+q),z)ї v ˉ x(0,j);}ǌ each=[](ł ɷ,ʍ,Õ q,ł put){put(0,j)Њ(ɷ)put(1,j+q);}ǌ put=ѯ(Ȩ,ȡ,Õ p,ɍ)Ý{if(8*p<þ)store_first(į Ƨ p,x,þ-8*p);};ѧ(ӕ,Ǡ â,nv)Ĩ Ǝ split=ɼ>Ơ;inverse_top(t,v,nv,split?ȉ:Ʃ,ŧ);Ʌ(f,m,0,Ǡ â,Ǡ â,ĥ(ɼ,Ơ))Ȣ þ=split?ɼ-Ơ:0;ǚ ʓ=hi Ʒ(ɼ-þ);Ʌ(f,m,Ơ,Ǡ Ʒ Ơ,ʓ,þ);ϰ(t,product_h,Ǡ â,nv,Ś)ĺ(!split){ǔ<Ʃ>(t,v,nv,ŧ,Ͷ{each(ɷ,j,q,ѯ(ң,Õ p){put(hi â,ɼ,p,ń(d Ƭ,kP));});})а}ǔ<Ʃ>(t,v,nv,ŧ,Ͷ{source(ɷ,j,q,ѯ(ң,Õ p){ġ d0=ń(d Ƭ
,kP);ū(hi ʥ p,d0)Ū d0;});});ϰ(t,ǆ(Ȱ,ɧ,p4),Ǡ â,nv);ǔ<ƨ>(t,v,nv,ŧ,ѯ(ł ɷ,ʍ,Õ q,ġ*,ġ*x){source(ɷ,j,q,ѯ(ң,Õ p){Ģ at=8*p Ū ů(ƫ(x Ƭ,load_first(ʓ+ĥ(at,þ),at<þ?þ-at:0)));});});ϰ(t,product_h,Ǡ â,nv);ǔ<Ʃ>(t,v,nv,ŧ,Ͷ{each(ɷ,j,q,ѯ(ң,Õ p){put(ʓ,þ,p,ń(d Ƭ,kP));});});}í sqrt(Ƶ,О,Ö c,Ĝ g,Є){ѻ n=ȸ ĺ(f ű||f Ŭ ε||ʀ(c,c)!=f Ŭ)ǩ ĺ(n<=Ξ)Ú sqrt_direct(f,c,g)Ȣ m=Ǽ sqrt_log(n),č=à(m)ǫ b=ĸ ʴ,ht=ӿ(č,m),Ǡ=ӿ(2*č,m);sqrt_steps(t,f,c,g ʴ,b,ht)Ȇ(g ʴ,0,b);sqrt_last_step(t,f,b,ht,Ǡ,g Ė(m));}}ā Ü ē{í native_sqrt(ǣ*in,Õ ʭ,л,ƿ,Ī ˏ){Ƞ lg=sqrt_log(n)Ȣ c Ч(ʭ,n)й(ɠ(c)Я(n)Я(sqrt_scratch(n)))Ǭ f ф(c),g к,ĸ ф(sqrt_scratch(n))ø(Ů c Į){Ù ī v=Ө;f ƈ ĥ(v,v-Ƀ);}ğ ct=ʀ(ĥ(ˏ.v,ˏ.v-Ƀ),ã ǹ(65536,Ƀ-2));sqrt(t,f,ct,g,ĸ);append_scaled(g,į,65536);}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
Ň ā Ü ʉ::unbalanced{Ę ā ē;ò Ǝ fits(ƿ,Ό){ʦ n0 Ч(n,m),i4=Ŋ(n,m)Ū n0>32&&i4>=32768&&n0<=i4/32;}ò ƶ Ź(ˢ a,ˢ b){Ģ ǯ=8*ɬ(ĥ(ư,ζ))ǌ&x=ư>=ζ?a:b ǌ&y=ư>=ζ?b:a Ȣ n=x Û,m=y Û,ď=ǯ-m+1 Ȋ lg=ĩ(ǯ)й(Č Ư(lg)+3 ӌ(ǯ))Ǭ bt ф(ǯ),in ф(ǯ),į ф(ǯ)ø(Ů m Į)in ƈ y Ĕ Ȅ Ȇ(in ʴ,0,bt);ƶ ă;Ü ƚ(ă,n+m-1)ø(ә n;p+=ď){Ģ þ Ч(ď,n-p)ø(Ů þ Į){Ù ī v=x[p+i].v;in ƈ ĥ(v,v-Ƀ);}t.Ŷ(in Ȓ(þ),0,į,bt)Ȣ j8=p ε?0:m-1 ø(Ů j8 Į){Ù ī v=ă[p+i].v+į Ĕ;ă[p+i]=һ(ĥ(v,v-Ƀ));}Ü ž(ă,Ԁ j8,þ+m-1-j8);}Ҥ}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
Ň ā Ü ʉ::ˏ{ò ī ů(Ī x)Ш ò Ǝ same_eight(ǣ*p,ī Ĵ){Ϸ v;Ǵ(&v,p,ƛ(v));v=ą(v,ê(v,ç(Ƀ)))Ū _mm256_movemask_epi8(_mm256_cmpeq_epi32(v,ç(Ĵ)))==-1;}ò Ǝ remaining(ˢ a,ī Ĵ){ɮ=8 ø(ʲ ư ƞ)if(!same_eight(a ȝ,Ĵ))Ú Ś ø(ϼ ư Į)if(ů(ɨ)!=Ĵ)Ú Ś Ū Ƈ;}Ɨ ò Ǝ fits(Ҧ a,Ҧ b){if(ĥ(ư,ζ)<4096)Ú Ś Ĩ ī av=ů(a Ŭ),bv=ů(b Ŭ)ĺ(!same_eight(ʰ,av)||!same_eight(b â,bv))Ú Ś Ū remaining(a,av)&&(&a==&b||remaining(b,bv));}Ɨ ò ƶ Ź(ˢ a,ˢ b){Ģ n=Ŋ(ư,ζ),m Ч(ư,ζ),æ=n+m-1 Ĩ ī c=ů(һ(ů(a Ŭ))*һ(ů(b Ŭ)));ƶ ă(æ)ĺ(c ε)Ҥ Ť ī ù Ҹ(ī i=0;i<8 Į)ù ƈ ŉ(c)*(i+1)%Ƀ;Ϸ Ĵ=_mm256_load_si256(õ Ù Ϸ*>(ù))ǌ ɰ ʖ(ŉ(c)*8%Ƀ),p ʖ(Ƀ)ǌ ˁ=ʞ;ɮ Ɠ(ʲ m ƞ){Ǵ(ш(ă ȝ),&Ĵ,ƛ(Ĵ))ǌ l6=Ľ(Ĵ,ˁ);Ǵ(ш(ă Ʒ æ-i-8),&l6,ƛ(l6));Ĵ=ň(Ĵ,ɰ);Ĵ=ą(Ĵ,ê(Ĵ,p));}ī d6=_mm256_extract_epi32(Ĵ,0)ø(ϼ m Į){ă ƈ ă[æ-i-1]=һ(d6);d6+=c;d6 Ч(d6,d6-Ƀ);}if(n>m+1)ż(ă ʷ m,ă ʷ Ԕ,һ(ŉ(c)*m%Ƀ))ȧ}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
Ň ā Ü ʉ{ā ntt{ā kernels=ì kernels;Ϥ=å;Ũ Ғ{Õ n_,m_,len_;ʃ Ě arena_;ʃ Č Ǫ;Ņ a_,*b_;ċ ý ɶ(ƿ,Ό){if(n ε||m ε||n>(1u<<23)||m>(1u<<23)||n+m-1>(1u<<23))ǩ Ū Ŋ(6,ý(ʌ(n+m-2)));}Ⱥ Ғ(ƿ,Ό):Ғ(n,m,Ƿ){}Ғ(ƿ,Ό,Ņ b8):n_(n),m_(m),len_(Ǽ ɶ(n,m)),arena_(ʃ Č Ư(ɶ(n,m))+(b8?1:2)*ʃ à(len_)),Ǫ(arena_,ɶ(n,m)),a_(b8?b8:arena_ ņ(len_)â),b_(arena_ ņ(len_)â){if(n ε||m ε||ɶ(n,m)>23)ǩ;}ċ Ǝ fits(ƿ,Ό){Ƞ lg=ɶ(n,m)Ū lg%2 ε&&lg>=10&&2*Ŋ(n,m)<=Ǽ lg;}Ң(){Ú a_;}Ņ b(){Ú b_;}Ɣ Ź(){Ǫ.ı(ĝ(b_,m_),0,Ĝ(b_,len_));Ǫ.Ŷ(ĝ(a_,n_),0,Ĝ(a_,len_),ĝ(b_,len_),ʃ ȉ,ä ǹ(ä kR,ƥ ӓ))Ū a_;}};}Ę Product=ntt::Ғ ȅ ĭ ȅ Ʈ ȅ ƫ ȅ ǘ ȅ kP ȅ ń;Ǹ ŭ(ɍ,Œ&f){Ú ntt::ä Ź(x,f);}Ǹ diff_canonical(ɍ,ĭ y){Ú ê(ň(x,ntt::ä ɟ),y);}Ũ Subtrees{Ɣ Ɛ,*ϯ;Ⱥ Subtrees(Ɣ r,Ɣ ir):Ɛ(r),ϯ(ir){}Ĉ ϖ(ҳ a,ҳ b,Ԅ,ȕ)Ù{Ę ā ʃ ë;ǆ<ǂ>(Ɛ,ϯ,ǂ{Ɛ}).ϖ(õ Ņ>(b),nv,k);ȥ b9{Ɛ,ϯ,õ Ņ>(b),k*nv};ǆ<ȥ>(Ɛ,ϯ,b9).ϖ(õ Ņ>(a),nv,k);}};í forward_radix8(ҳ f,Õ q,М){Œ i(ķ ƹ,ķ[9]),y(ķ ǥ,ķ[10]),z(ķ Ѐ,ķ[11])ø(ǈ q ų){ġ f0=f Ƃ,f1=f[j+q],f2=f[j ʳ q],f3=f[j+3*q]ι g0=ƫ(f0,f2),g1=ƫ(f1,f3)ι h0=diff_canonical(f0,f2),ih1 Ҝ(diff_canonical
(f1,f3),i);f ˉ ƫ(g0,g1),f[j+q]=ǘ(g0,g1);f[j ʳ q]=ƫ(h0,ih1),f[j+3*q]=ǘ(h0,ih1)ι if2 Ҝ(f2,i),if3 Ҝ(f3,i)ι u0=ń(ƫ(f0,if2),2*kP),v0=ń(ǘ(f0,if2),2*kP)ι yu1 Ҝ(ƫ(f1,if3),y),zv1 Ҝ(ǘ(f1,if3),z);f[j+4*q]=ƫ(u0,yu1),f[j+5*q]=ǘ(u0,yu1);f[j+6*q]=ƫ(v0,zv1),f[j+7*q]=ǘ(v0,zv1);}}í copy_input(Ҧ in,Ȩ,Õ ǯ){ɮ Ē ʦ p ʖ(Ƀ)ø(ʲ in Û ƞ){ʦ x=ɂ(õ Ù Ϸ*>(in ȝ));ѩ(õ Ϸ*>(є i),Ǧ);}Ć(ϼ in Û Į)į ƈ ĥ(Ө,Ө-Ƀ);ż(є i,є(i<=ǯ/2?ǯ/2:ǯ),0);ż(є ǯ,є ǯ ү,0);}í copy_input_mint(Ҧ in,Ī*į){Ά;ɮ Ē ʦ p ʖ(Ƀ)ø(ʲ in Û ƞ){Ϸ x;Ǵ(&x,in ȝ,ƛ(x));x=Ǧ;Ǵ(ш(є i),&x,ƛ(x));}Ć(ϼ in Û Į)į ƈ һ(ĥ(Ө,Ө-Ƀ));}Ő Plan>ò ƶ run_borrowed_as(Ҧ a,Ҧ b){Ģ ǯ=Ǽ Ŋ(6,ý(ʌ(ư+ζ-2)));ɿ Б=32 Ȣ č=ǯ ү+Б/ƛ(Ī)-1,æ=ư+ζ-1;ƶ ă;ƚ(ă,č);ă.assign(č,һ(0))ǌ a6=Λ(ă â)Ȣ Ų=((Б-(a6&(Б-1)))&(Б-1))/ƛ(Ī);Ī*Ù to=ă Ʒ Ų;Plan plan(ư,ζ,õ Ņ>(to));copy_input_mint(a,to);copy_input(b,Ƹ b(),ǯ);Ƹ Ź()ĺ(Ų)Ø memmove(ш(ă â),to,æ*ƛ(Ī));ă Ř(æ)ȧ}ò ƶ run_borrowed(Ҧ a,Ҧ b){Ú run_borrowed_as<Product>(a,b);}Ő Plan>ò ƶ run(Ҧ a,Ҧ b){Plan plan(ư,ζ)ǌ ǯ=Ǽ Ŋ(6,ý(ʌ(ư+ζ-2)));copy_input(a,Ƹ a(),ǯ);copy_input(b,Ƹ b(),ǯ)ǌ*p=Ƹ Ź();
ƶ ă Ȣ æ=ư+ζ-1;ƚ(ă,æ);ž(ă,p,æ)ȧ}ò ƶ Ź(Ҧ a,Ҧ b){Ƣ(!a ű&&!b ű&&ư+ζ-1 ɱ)ĺ(ˏ::fits(a,b))Ú ˏ::Ź(a,b)ĺ(unbalanced::fits(ư,ζ))Ú unbalanced::Ź(a,b)Ū run_borrowed_as<ntt::Ғ>(a,b);}}
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā Ü ē{Ί З(ƿ,Ό){Ú n>=m?n-m+1:0;}Ί remainder_size(ƿ,Ό){Ú ĥ(n,m-1);}ā ƺ{Ɵ;ò ɿ Ϯ=8;ò ɿ kLongDivision=Ǽ 12;ò ɿ kDirectRemainder=64;ò è ǜ ʬ,26>kTransformNs={0,0,0,0,0,0,0.45,0.48,0.51,0.55,0.6,0.65,0.7,0.76,0.81,0.88,0.95,1.02,1.09,1.17,1.23,1.3,1.37,1.44,1.51,1.58};ò è ʬ kLeafProductNs=0.92,kDirectTermNs=0.04,kFoldNs=0.25;Ί transform_length(ƿ){Ú ʘ(64,ɬ(n));}ò ʬ ʾ(Õ ǯ){ʦ lg=ĩ(ǯ)Ū Õ(lg)<kTransformNs Û?kTransformNs[lg]*ʬ(ǯ):ǻ;}ò ʬ Ӈ(Õ ǯ){Ú 2*ʾ(ǯ)+kLeafProductNs*ʬ(ǯ);}ò ʬ inverse_ns(ƿ){ʬ ns Ɠ(ȕ=kInverseBase;k<n;k*=2)ns+=ʾ(Ӷ)ʳ Ӈ(Ӷ)Ū ns;}Ɯ ɢ{Õ s=0,Ǿ=0,pad=0,Ǯ=0,Η Ē Ǝ Ѧ=Ś;ʬ ns=ǻ;ɢ()=default;ɢ(ȕ,Õ d,Õ ŀ):s(ŀ),Ǿ((k+ŀ-1)/ŀ),pad(Ǿ*ŀ-k),Η(transform_length(2*d)),Ѧ(d<ŀ){Ģ ǯ=2*s;ns=inverse_ns(ɒ(k))+ʾ(ǯ)+ʬ(Ǿ)*Ӈ(ǯ)ĺ(Ǿ у)Ú ĺ(Ѧ){ns+=ʾ(Η)+ʬ(Ǿ-1)*Ӈ(Η)а}Ģ l3=d/s+1;Ǯ Ч(l3,Ǿ-1)ĺ(Ǯ>Ϯ){ns=ǻ а}ʬ ơ Ɠ(ʍ=1;j<Ǿ ų)ơ+=ʬ(ĥ(j,l3));ns+=ʬ(Ǯ ʳ(Ǿ-1))*ʾ(ǯ)+ơ*kLeafProductNs*ʬ(ǯ);}Õ ɒ(ȕ)Ù{Ú Ǿ у?k:s;}};enum Ũ ǵ{kDirect,kWrap,kSplit,kTail};Ɯ RemainderPlan{ǵ method;Õ î Ē ʬ ns=ǻ;RemainderPlan(ǵ how,ƿ,ȕ,Õ d,Ù ɢ&ť):method(how){Ģ wrap=transform_length
(d);switch(how){case ǵ::kDirect:if(k<=kDirectRemainder)ns=ʬ(k)*ʬ(d)*kDirectTermNs;ϵ case ǵ::kWrap:î=wrap;ns=Ӈ(î)+ʾ(î)+ʬ(n+k)*kFoldNs;ϵ case ǵ::kSplit:î=wrap/2 ĺ(î>=64&&d>î&&k+(d-î)<=î+1)ns=ʾ(î)ʳ Ӈ(î)+ʬ(n+d)*kFoldNs;ϵ case ǵ::kTail:if(!ť.Ѧ)ϵ ns=(ť.Ǿ у?ʾ(ť.Η):0)+Ӈ(ť.Η)+ʬ(d)*kFoldNs;ϵ}}Õ buffers()Ù{Ú method==ǵ::kWrap?2:method==ǵ::kSplit?3:0;}};Ɯ Ν{ƿ,m,k,d;Ǝ ˡ;ɢ ť;RemainderPlan ɑ;ý lg=ʵ;Ν(Õ n_,Õ m_):n(n_),m(m_),k(З(n_,m_)),d(m_-1),ˡ(k*d<=kLongDivision),ɑ(ǵ::kWrap,n,k,d,ť){if(ˡ)Ú;ʬ best=ǻ ø(Õ s=32;;s*=2){Ù ɢ p(k,d,s)ø(ǵ how:{ǵ::kDirect,ǵ::kWrap,ǵ::kSplit,ǵ::kTail}){Ù RemainderPlan r(how,n,k,d,p)ĺ(p.ns+r.ns<best)best=p.ns+r.ns,ť=p,ɑ=r;}if(s>=k||4*s>(Ǽ Č::Ҕ))ϵ}set_log();}Ν(Õ n_,Õ m_,Õ s,ǵ how):n(n_),m(m_),k(З(n_,m_)),d(m_-1),ˡ(Ś),ť(k,d,s),ɑ(how,n,k,d,ť){set_log();}Ǝ Ѧ()Ù{Ú ť.Ѧ&&(ť.Ǿ>1||ɑ.method==ǵ::kTail);}Ĉ set_log(){lg=ĩ(Ŋ({2*ť.s,Ѧ()?ť.Η:0,ɑ.î}));}Õ λ()Ù{Ù ɢ&p=ť Ū Ѧ()?m:ĥ(m,Ŋ(p.ɒ(k),(p.Ǯ+1)*p.s));}Õ quotient_words()Ù{Ù ɢ&p=ť Ȣ ǯ=2*p.s;Õ h9=à(p.Ǿ*p.s+p.s)ĺ(Ѧ())h9+=2 ӌ(p.Η)ĺ(p.Ǯ)h9+=(2*p.Ǯ+1)
ӌ(ǯ)Ū à(λ())Я(ǯ)+Ŋ(ȵ(p.ɒ(k)),h9);}Õ ĸ()Ù{Ú Ŋ(quotient_words(),ɑ.buffers()ӌ(ɑ.î));}Õ č()Ù{Ú ˡ?à(n):ɠ(ĸ());}};Ũ Bump{Ϳ Bump(Ĝ i6):ю(i6){}Ĝ ś(ƿ){if(used_>ю Û||à(n)>ю Û-used_)throw Ø bad_alloc()ǫ s=ю Ė(used_,n);used_+=à(n)Ū s;}Ĝ ɼ()Ù{Ú ю Ė(used_);}Ⱦ Ĝ ю;Õ used_ Ē};Ǹ ґ(н,ĭ ȇ){ȱ,p=ɟ;Ĥ Ú ǝ;}ԃ А(Ö x){Ú x;}ԃ А(Ī x)Ш Ǹ Β(Ɣ p){Ú ɂ(õ ġ*>(p));}Ǹ Β(ǣ*p){Ά;ĭ Ĵ;Ǵ(&Ĵ,p,ƛ(Ĵ))Ū ń(Ĵ,kP);}í store_any(Ņ p,ɍ){Ψ(ț(p),x);}Ǹ reversed(ɍ){Ú Ľ(x,ʞ);}í append_words(л,Ɣ Ƴ,ƿ,Ö Ÿ=1){Œ ŧ(Ÿ);ɮ Ɠ(ʲ n ƞ){ĭ ɽ=Β(Ƴ+i)ĺ(Ÿ!=1)ɽ Ҝ(ɽ,ŧ);Ī ŀ ɚ;Ə(ƛ(ŀ)==ƛ(ɽ)&&Ɖ Ī>);Ǵ(ш(ŀ),&ɽ,ƛ(ŀ));į ӭ(į ƃ,ŀ,ŀ+8);}Ć(Ȝ)į η(һ(Ÿ у?Ƴ Ĕ:ʀ(Ƴ Ĕ,Ÿ)));}í Ї(л,Ɣ Ƴ,ƿ,Ö Ÿ=1){Ά Ƥ ŧ(Ÿ);ǜ Ī,64>batch;ɮ Ɠ(;i+64<=n Ƽ 64){if(Ÿ у)Ǵ(ш(batch â),Ƴ+i,ƛ(batch))ї{
#pragma GCC unroll 8
Ć(ǈ 64 Ӕ){ġ ɽ Ҝ(Β(Ƴ+i+j),ŧ);Ǵ(ш(batch Ʒ j),&ɽ,ƛ(ɽ));}}į ӭ(į ƃ,batch ü,batch ƃ);}if(i<n)append_words(į,Ƴ+i,n-i,Ÿ);}ˬ í ʫ(Ù T*a,Ҡ,Ȩ,ȡ){ɮ Ɠ(ʲ þ ƞ)store_any(є i,ɸ(Β(a+i),Β(b+i)))ø(ϼ þ Į)į ƈ(А(ɨ)+Ͼ ϸ)Ұ;}ˬ í ʽ(Ğ T>a,ř top,Ȩ,ȡ,Ɣ sub=Ƿ){Ù ř æ=ř(ư)ǌ Ĵ=ѯ(ɮ){Ù ř p=top-ř(i);ğ x=p>=0&&p<æ?А(a[Õ(p)]):0 Ū sub?(x+Ͼ sub Ĕ)Ұ:x;};ɮ=top>=æ?ĥ(þ,Õ(top-æ+1)):0 ĺ(sub){Ć(ǈ i ų)į ˉ Ĵ(j);}ŝ{ʇ(į,i,0);}Ć(ʲ þ&&top-ř(i)>=7 ƞ){Ҽ reversed(Β(a Ʒ(top-ř(i)-7)));store_any(є i,sub?ɸ(x,Β(sub+i)):x);}Ć(ϼ þ Į)į ƈ Ĵ(i);}ˬ í ё(Ğ T>a,ɐ){Ģ ǯ=ǲ;ż(į ü,į ƃ,0)ø(Õ ʝ Ē ʝ<ư;ʝ+=ǯ){Ģ þ Ч(ǯ,ư-ʝ),full=þ/8*8 Ĩ T*x=a Ʒ ʝ ø(ǈ full Ӕ){Š(Ԁ j,ń(ƫ(Ō(Ԁ j),Β(x+j)),kP));}Ć(ʍ=full;j<þ ų)į ˉ(į Ƃ+А(x Ƃ))Ұ;}}ˬ í ˡ(ԁ T>f,ĝ g,Ĝ q,Ĝ r,Ĝ ɼ){Ę ʀ Ȣ d=ȸ-1 Њ(ģ T,Ö>)Ȫ(f ü,f ƃ,ɼ ü)ї Ć(Ů Ǳ Į)ɼ ƈ А(f Ĕ);ğ Μ=ã ǹ(g[d],ӓ)ø(ɮ=Ѹ;i-->0;){ğ c=Đ(ɼ[i+d],Μ);q ƈ c ø(Ӄ<d ė t)ɼ[i+t]=(ɼ[i+t]+Ͼ Đ(c,g Ѱ))Ұ;}ĵ(ɼ ü,r Û,r ü);}ˬ í remainder_direct(ԁ T>f,ĝ g,ĝ q,Ĝ r){Ģ d=r Û,k=Ѹ;ĭ Ÿ[kDirectRemainder]ø(Ů k Į)Ÿ ƈ _mm256_set1_epi64x(Ø int64_t((ŉ(q Ĕ)<<32)Ұ))ǌ scalar=ѯ(ʍ){ŉ ʂ Ɠ(ɮ Ē i<=ĥ(j,k
-1)Į)ʂ=(ʂ+ŉ(q Ĕ)*g[j-i])Ұ;r ˉ Ö((А(f Ƃ)+Ͼ ʂ)Ұ);};ʍ Ɠ(;j<ĥ(d,k)ų)scalar(j)ø(;j+8<=d Ӕ){ĭ o4=Β(f Ʒ j)ø(Ů k Ƽ 4){н=đ,ȇ=đ ø(Õ t=i;t<ĥ(k,i+4)ė t){Ҽ Β(g Ʒ(j-t));Ŧ=ð(Ŧ,ß(x,Ÿ Ѱ));ȇ=ð(ȇ,ß(ʤ,Ÿ Ѱ));}o4=ń(ê(ƫ(o4,П),ґ(Ŧ,ȇ)),2*kP);}store_any(r Ʒ j,ń(o4,kP));}Ć(;j<d ų)scalar(j);}ˬ í remainder_wrap(Ƶ,ԁ T>f,ĝ g,ĝ q,Ĝ r,ҽ,Ĝ b){Ģ ǯ=ư ĺ(Ѹ<=ǯ){t.ı(q,0,a);}ŝ{ё(q,a)Ȇ(a);}ĝ gl=g ĺ(ȸ>ǯ){ĵ(g ü,ǯ,b ü);b Г(b Ŭ+g[ǯ])Ұ;gl=b;}t.Ŷ(gl,0,b,a);ё(f,a);ʫ(ʰ,b â,r â,r Û);}ˬ í remainder_split(Ƶ,ԁ T>f,ĝ g,ĝ q,Ĝ r,ҽ,Ĝ b,Ĝ c){Ģ ǯ=ư,k=Ѹ,e=r Û-ǯ Ȇ(q,0,a);t.Ŷ(g Ė(ǯ+1-k,k+e-1),0,c,a);ʫ(f Ʒ ǯ,c Ʒ(k-1),r Ʒ ǯ,e);ё(g,b);t.Ŷ(b,a);ё(f,c);ʫ(c â,b â,r â,ǯ);ʫ(r â,r Ʒ ǯ,r â,e);}Ą<Õ K>Ĉ ʼ(Ƶ,ԁ ɝ>Ǜ,ɐ){ϗ<K>b9{}ø(Ů K Į)b9.ơ ƈ{{Ȱ,ɧ,Ǜ Ĕ.b â},Ǜ Ĕ.ʰ};t.Ɇ(b9,į,ƨ);}í ʼ(Ƶ,ԁ ɝ>Ǜ,ɐ){switch(Ǜ Û){case 1:case 2:case 3:Ú t.Ŗ(Ǜ,į,ƨ)ϻ 4:Ú ʼ<4>(t,Ǜ,į)ϻ 5:Ú ʼ<5>(t,Ǜ,į)ϻ 6:Ú ʼ<6>(t,Ǜ,į)ϻ 7:Ú ʼ<7>(t,Ǜ,į)ϻ 8:Ú ʼ<8>(t,Ǜ,į);default:ǩ;}}ˬ í subtract_reversed(Ù T*a,Ҡ,Ȩ,ȡ){ʍ Ɠ(;j+8<=þ Ӕ)store_any(є j,ɸ(Β(a+j),reversed(Β(b+þ-8-j))))ø(;j<þ ų)į ˉ(А(a Ƃ)+Ͼ b[þ-1-j])Ұ;}ˬ í divide_blocks
(Ƶ,Ù Ν&plan,ԁ T>f,ĝ g,Ĝ q,Ĝ r,Є){Ù ɢ&p=Ƹ ť Ȣ k=Ѹ,s=p.s,ǯ=2*s,d=Ƹ d Ĩ ř top=ř(Ǳ-1+p.pad);Bump bump(ĸ)ǫ G=bump ņ(Ƹ λ()),ht=ɭ Ӎ(g ƃ-ř(G Û),g ƃ,G ü)Ȣ ɒ=p.ɒ(k);ć(t,G Ȓ(ĥ(G Û,ɒ)),ht Ȓ(ɒ),bump.ɼ())Ȇ(ht Ȓ(ɒ),0,ht)ǫ qs=bump ņ(p.Ǿ*s+s);Ĝ gt,mp,Ǡ;ǜ Ĝ,Ϯ>Ǯ{},ring{}ĺ(Ƹ Ѧ()){gt=bump ņ(p.Η),mp=bump ņ(p.Η)Ȇ(G,0,gt);}if(p.Ǯ){Ǡ=ɭ ø(Ů p.Ǯ Į){Ǯ ƈ ɭ,ring ƈ ɭ Ȇ(G Ė(i*s,ĥ(ǯ,G Û-i*s)),0,Ǯ Ĕ);}}ʦ tail_product=ѯ(ʍ){t.Ŷ(qs Ė(j*s-d,d),p.Η-2*d,mp,gt,ƨ)Ū mp Ʒ(p.Η-d);}ø(ǈ p.Ǿ ų){ǚ ŀ=qs Ʒ j*s Ĩ ř ʝ=top-ř(j*s)ĺ(j ε){ʽ(f,ʝ,ŀ,s);}м(p.Ѧ){ʽ(f,ʝ,ŀ,d,tail_product(j));ʽ(f,ʝ-ř(d),ŀ+d,s-d);}ŝ{Ģ ơ Ч(j,d/s+1);ǜ ɝ,Ϯ>Ǜ ø(Ů ơ Į)Ǜ ƈ{Ǯ Ĕ,ring[(j-1-i)%p.Ǯ]};ʼ(t,ԁ ɝ>(Ǜ â,ơ),Ǡ);ʽ(f,ʝ,ŀ,s,Ǡ Ʒ s);}t.Ŷ(qs Ė(j*s,s),0,qs Ė(j*s,ǯ),ht,Ʃ)ĺ(p.Ǯ&&j+1<p.Ǿ)t.ı(qs Ė(j*s,s),0,ring[j%p.Ǯ]);ʽ(qs Ȓ((j+1)*s),ř((j+1)*s-1),q Ʒ(p.Ǿ-1-j)*s,j ε?s-p.pad:s);}if(Ƹ ɑ.method==ǵ::kTail)subtract_reversed(f â,tail_product(p.Ǿ),r â,d);}ˬ í divide(Ě&ŋ,Ù Ν&plan,ԁ T>f,ĝ g,Ĝ q,Ĝ r){if(Ƹ ˡ)Ú ˡ(f,g,q,r,Ĭ(Ǳ))Ĩ Č t(ŋ,Ƹ lg)ǫ ĸ ф(Ƹ ĸ());divide_blocks(t,plan,f,g,q,r,ĸ);Bump bump(
ĸ)Ȣ ǯ=Ƹ ɑ.î;switch(Ƹ ɑ.method){case ǵ::kDirect:Ú remainder_direct(f,g,q,r)ϻ ǵ::kWrap:Ú remainder_wrap(t,f,g,q,r,ɭ,ɭ)ϻ ǵ::kSplit:Ú remainder_split(t,f,g,q,r,ɭ,ɭ,ɭ)ϻ ǵ::kTail:Ú;}}}Ί divide_words(ƿ,Ό){Ú ƺ::Ν(n,m).č();}í divide(Ě&ŋ,О,ĝ g,Ĝ q,Ĝ r){Ù ƺ::Ν plan(Ǳ,ȸ)ĺ(Ƹ k ε){Ȫ(f ü,f ƃ,r ü)а}ƺ::divide(ŋ,plan,f,g,q,r);}}ā Ü ē{í native_divide(ǣ*a,ƿ,ǣ*b,Ό,ƶ&ť,ƶ&ɑ){Ù ƺ::Ν plan(n,m)Ȣ k=З(n,m),d=remainder_size(n,m)й(Ƹ č()Я(m)Я(k)Я(d))Ĩ ˢ f(a,n)ѵ g ф(m),q ф(k),r ф(d)ø(Ů m Į)g ƈ ĥ(ϸ.v,ϸ.v-Ƀ);ƺ::divide(ŋ,plan,f,g,q,r);ƚ(ť,k),ƚ(ɑ,d);ƺ::Ї(ť,q â,Ѹ,ã kR);ƺ::Ї(ɑ,r â,r Û);}í native_div_series(ǣ*a,Õ ʭ,ǣ*b,Õ λ,л,ƿ){Ę ā ƺ Ȣ þ Ч(λ,n),ũ=þ-1;ɢ plan ø(Õ s=32;;s*=2){ɢ candidate(n,ũ,s)ĺ(candidate.ns<Ƹ ns)plan=candidate ĺ(s>=n||4*s>(Ǽ Č::Ҕ))ϵ}Ģ s=Ƹ s,ǯ=2*s Ĩ Ǝ Ѧ=Ƹ Ѧ&&Ƹ Ǿ>1 Ȣ ɒ=Ƹ ɒ(n),j4 Ч(ʭ,n)Ȋ lg=ĩ(Ŋ(ǯ,Ѧ?Ƹ Η:0))Ȣ h9=à(Ƹ Ǿ*s+s)+(Ѧ?2 ӌ(Ƹ Η):0)+(Ƹ Ǯ?(2*Ƹ Ǯ+1)ӌ(ǯ):0)й(ɠ(j4)Я(þ)Я(ǯ)Я(Ŋ(ȵ(ɒ),h9)))Ǭ f ф(j4),g ф(þ),ht ф(ǯ)ø(Ů j4 Į)f ƈ ĥ(ɨ.v,ɨ.v-Ƀ)ø(Ů þ Į)g ƈ ĥ(ϸ.v,ϸ.v-Ƀ)ѵ ĸ ф(Ŋ(ȵ(ɒ),h9));ć(t,g Ȓ(ĥ(þ,ɒ)),ht Ȓ(ɒ),ĸ)Ȇ(ht Ȓ(ɒ),0,
ht);Bump bump(ĸ)ѵ qs=bump ņ(Ƹ Ǿ*s+s);Ĝ gt,mp,Ǡ;ǜ Ĝ,Ϯ>Ǯ{},ring{}ĺ(Ѧ){gt=bump ņ(Ƹ Η),mp=bump ņ(Ƹ Η)Ȇ(g,0,gt);}if(Ƹ Ǯ){Ǡ=ɭ ø(Ů Ƹ Ǯ Į){Ǯ ƈ ɭ,ring ƈ ɭ Ȇ(g Ė(i*s,ĥ(ǯ,þ-i*s)),0,Ǯ Ĕ);}}ƚ(į,n)ø(ǈ Ƹ Ǿ ų){ł ŀ=qs Ė(j*s,ǯ)Ȣ ʝ=j*s,ɲ=ʝ<j4?ĥ(s,j4-ʝ):0 ĺ(ɲ)ĵ(f Ʒ ʝ,ɲ,ŀ â);ż(ŀ ʷ ɲ,ŀ ʷ s,0u)ĺ(j&&Ѧ){t.Ŷ(qs Ė(j*s-ũ,ũ),Ƹ Η-2*ũ,mp,gt,ƨ);ʫ(ŀ â,mp Ʒ Ƹ Η-ũ,ŀ â,ũ);}м(j){Ģ ơ Ч(j,ũ/s+1);ǜ ɝ,Ϯ>Ǜ ø(Ů ơ Į)Ǜ ƈ{Ǯ Ĕ,ring[(j-1-i)%Ƹ Ǯ]};ʼ(t,ԁ ɝ>(Ǜ â,ơ),Ǡ);ʫ(ŀ â,Ǡ Ʒ s,ŀ â,s);}t.Ŷ(ŀ Ȓ(s),0,ŀ,ht,Ʃ)ĺ(Ƹ Ǯ&&j+1<Ƹ Ǿ)t.ı(ŀ Ȓ(s),0,ring[j%Ƹ Ǯ]);Ї(į,ŀ â,ĥ(s,n-ʝ),ã kR);}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
Ő T=fps_mint>Ɯ Ď:џ,private Ü Ǆ{Ə(Ҳ==998244353,"FormalPowerSeries supports only mod 998244353")ͼ W=Ď Ӹ ͼ ƻ=џ ͼ F=Ď ͼ ƻ::ƻ;Ď()=default;Ď(ƻ v):ƻ(Ɔ(v)){}F&ŕ=(ƻ v){ƻ::ŕ=(Ɔ(v))Ū ˈ;}F&ŕ=(Ø initializer_list<T>v){ƻ::ŕ=(v)Ū ˈ;}ċ F Ź(ʧ a,ʧ b){if(a ű||b ű)Ú{};ƀ{W x=ź(a)Ū ƾ(&a==&b?W::Ź(x,x):W::Ź(x,ź(b)));}if(ĥ(ư,ζ)<=32){F r(ư+ζ-1)Ĩ F&small=ư<ζ?a:b Ĩ F&h8=ư<ζ?b:a;Ġ{Ć(Ȕ ó(small Û)ų)Ű add_scaled(č(r)+j,č(h8),h8 Û,small Ƃ Ȅ);}ŝ Ć(Ȕ ó(small Û)ų)Ć(ö ó(h8 Û)Į)r[i+j]+=h8 Ĕ*small Ƃ Έ}ǧ=1,æ=ư+ζ-1 ǟ(n<æ)n<<=1;Ƣ(n ɱ);
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(Ű ě)Ú F(Ü ʉ::Ź(a,b));}
#endif
F x=a,y;x Ř(n)ĺ(&a!=&b){y=b;y Ř(n);}if(n>=64&&&a!=&b){ł&t=Ɓ(n/16);t.convolution(č(x),č(y),n);x Ř(æ);t.ŧ(č(x),æ,Ǘ(n/8))Ū x;}ntt(x)ĺ(&a==&b)Ȉ(x,x,x,n)ї{ntt(y);Ȉ(x,x,y,n);}intt(x);x Ř(æ)Ū x;}F ŕ-()Ù{F r=ˈ ø(T&x:r)x=-x Έ}F&ŕ+=(ʧ g){ԓ+=g Ĕ Ū ˈ;}F&ŕ-=(ʧ g){ԓ-=g Ĕ Ū ˈ;}F&ŕ+=(T x){if(Ƕ empty())Ƕ resize(1);(ˈ)Ŭ+=x Ū ˈ;}F&ŕ-=(T x){Ú ˈ+=-x;}F&ŕ*=(ʧ g){Ú ˈ=Ź(ˈ,g);}F&ŕ*=(T x){Ġ{Ζ(ˈ,x)Ū ˈ;}Ć(T&a:ˈ)a*=x Ū ˈ;}F&ŕ/=(ʧ g){Ú ˈ=Α(g);}F&ŕ%=(ʧ g){Ú ˈ=divmod(g).χ;}F&ŕ/=(T x){Ú ˈ*=x ˆ;}F&ŕ<<=(ý d){Ƣ(d>=0)Ĩ ǧ=ϥ ĺ(d>=n)Ú ˈ=F(n);Ƕ insert(Ƕ Ă,d,ȓ);Ƕ resize(n)Ū ˈ;}F&ŕ>>=(ý d){Ƣ(d>=0)Ĩ ǧ=ϥ ĺ(d>=n)Ú ˈ=F(n);Ƕ erase(Ƕ Ă,Ƕ Ă+d);Ƕ resize(n)Ū ˈ;}F ŕ+(ʧ g)Ù{і+=g Έ}F ŕ-(ʧ g)Ù{і-=g Έ}F ŕ*(ʧ g)Ù{Ú Ź(ˈ,g);}F ŕ/(ʧ g)Ù{Ú Α(g);}F ŕ%(ʧ g)Ù{Ú divmod(g).χ;}F ŕ+(T x)Ù{і+=x Έ}F ŕ-(T x)Ù{і-=x Έ}F ŕ*(T x)Ù{і*=x Έ}F ŕ/(T x)Ù{і/=x Έ}F ŕ<<(ý d)Ù{і<<=d Έ}F ŕ>>(ý d)Ù{і>>=d Έ}F pre(ǧ)Ù{Ƣ(n>=0)Ū F(Ƕ Ă,Ƕ Ă+ʡ(n,ǰ));}F Α(ʧ g,Ѡ)Ù{ƀ Ú ƾ(ź(ˈ).Α(ź(g),ǀ))Н ǰ;Ƣ(ǀ>=0)ĺ(!ǀ)Ú{};Ƣ(!g ű&&g Ŭ!=ȓ)ĺ(Ƕ empty())Ú F(ǀ);
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(ǀ>=129&&ǀ ɱ&&ȸ>=64 ȏ){F r;Ü ʃ native_div_series(Ƕ é,ǰ,g â,ȸ,r,ǀ)Έ}}
#endif
if(ǀ<=128||ȸ<64){F r=Ź(pre(ǀ),g.ђ(ǀ)).pre(ǀ);r ӗ Έ}ϓ ĺ(ǀ<æ-æ/4)æ>>=1 Ȋ ŀ=æ/32,î=2*ŀ,k4=î ү,þ=(ǀ+ŀ-1)/ŀ;F ć=g.ђ(ŀ),r=Ź(pre(ŀ),ć).pre(ŀ);r ӗ ȹ d8=ʟ ӯ,*ť=d8+þ*k4 ø(ö þ Į){ɓ h=d8+i*k4 Ȋ ɲ=Ŋ(0,ʡ(ŀ,Џ-i*ŀ))ĺ(ɲ)ĵ(č(g)+i*ŀ,ɲ,h);ż(h+ɲ,h+ŀ,0u);ʒ(h,î);}ǅ Ц(d8,î,þ,k4);ƻ ƌ(î)ø(κ þ Į){Ƞ Ų=i*ŀ,n Ч(ŀ,ǀ-Ų);ɓ q=ť+(i-1)*k4 ԕ(č(r)+Ų-ŀ,ŀ,q);ʒ(q,î);Ű scale_normalize(q,î,norm)ø(Ȕ i;j+=7){Ù ɓ a[7],*b[7];ý ơ Ч(7,i-j)ø(ˋ ơ Ȟ){a ѓ ť+(j+k)*k4;b ѓ d8+(i-j-k)*k4;}ǎ}Ʋ(ƌ)ø(Ȕ ŀ ų)ƌ ˉ(Ų+j<ϥ?(ˈ)[Ų+j]:ȓ)-ƌ Ƃ Ђ ĵ(ƌ ü,n,r ʷ Ų);}Ú r;}F ђ(Ѡ)Ù{ƀ Ú ƾ(ź(ˈ).ђ(ǀ))ĺ(ǀ<0)Ϊ(!Ƕ empty()&&(ˈ)Ŭ!=ȓ);F g{(ˈ)Ŭ ˆ}ѵ ơ=sparse_terms(ǀ,32);Ӂ=ơ Û<=32?ѫ(ơ):1 ĺ(ơ Û<=32&&(ǀ<=256||ơ Û<=12ULL Ԍ)){g ӗ ĺ(ơ Û у&&ď у){ż(g ü,g ƃ,g Ŭ);ɘ(g,Ӽ,-ơ Ŭ.χ*g Ŭ)Ū g;}Ć(Ǉ i=ď;ď&&i<ǀ Ƽ ď)g ƈ-sparse_dot(ơ,g,ó(i))*g Ŭ Ū g;}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(ǀ>=512&&ǀ ɱ ȏ){F ă Ȋ ŀ=1<<(ʌ(ú ī>(ǀ))-1),Η=ǀ-ŀ ĺ(Η>0&&Η<=96){Ü ʃ native_inverse(Ƕ é,ǰ,ă,ŀ);ă ӗ ø(Ż ŀ ϼ ǀ Į)ă ƈ-ă Ŭ*dot(Ƕ é+1,ă ȝ-1,ʡ(i,ǰ-1))ȧ}Ü ʃ native_inverse(Ƕ é,ǰ,ă,ǀ)ȧ}}
#endif
if(ǀ>256&&ǰ>=64){
#if defined(__x86_64__) || defined(__i386__)
if(ǀ<1024||!Ű ě)Ú F{1}.Α(ˈ,ǀ);
#else
Ú F{1}.Α(ˈ,ǀ);
#endif
}g Ř(ĥ(ǀ,32))ø(κ Џ Į)g ƈ-g Ŭ*dot(Ƕ é+1,g ȝ-1,ʡ(i,ǰ-1));
#if defined(__x86_64__) || defined(__i386__)
Ü FpsWords ʟ;ɓ a=Ƿ,*b=Ƿ ĺ(ǀ>96 ȏ){ý δ=64 ǟ(δ<ǀ&&δ<(1<<23))δ<<=1;ʟ=Ü ɀ(2*δ);a=ʟ ӯ;b=a+δ;g.reserve(δ);Ɓ(Ŋ(32,δ/16));}
#endif
while(Џ<ǀ){Ƞ m=Џ,n=2*m ĺ(m>=32&&(ǀ-m<=32||(m>=512&&ǀ-m<=96))){g ӗ ø(Ż m ϼ ǀ Į){Ƞ þ=ʡ(i,ǰ-1);g ƈ-g Ŭ*dot(Ƕ é+1,g ȝ-1,þ);}ϵ}
#if defined(__x86_64__) || defined(__i386__)
if(m>=64&&n ɱ ȏ){ý lo=ʡ(n,ǰ)ԕ(č(ˈ),lo,a);ż(a+lo,a+n,Word(0))ԕ(č(g),m,b);Ɓ(Ŋ(1,n/16)).inverse_newton8(a,b,n);g Ř(n)ԕ(a+m,m,č(g)+m);}ŝ
#endif
{ƻ f=Ƕ pre(n),r=g;f Ř(n);r Ř(n);ntt(f);ʒ(r);Ȉ(f,f,r,n);Ʋ(f);ż(f ü,f ʷ m,ȓ);ntt(f);Ȉ(f,f,r,n);Ʋ(f)Ĩ T ђ=T θ(Ǘ n)Ĩ T ŧ=-ђ*ђ;g Ř(n);Ɓ(1).ŧ(č(f)+m,m,ŧ Ȅ)ԕ(f ʷ m,m,g ʷ m);}}g ӗ Ū g;}F sqrt(Ѡ)Ù{ƀ Ú ƾ(ź(ˈ).sqrt(ǀ))Н ϥ;Ƣ(ǀ>=0);ý ȣ=ó(ĥ<Õ>(ǀ,ǰ)),ù Ē while(ù<ȣ&&(ˈ)[ù]Ү)++ù ĺ(ù==ȣ)Ú F(ǀ)ĺ(ù&1)Ú{}Ĩ T ˏ=mod_sqrt((ˈ)[ù])ĺ(ˏ Ү)Ú{}ǟ((ˈ)[ȣ-1]Ү)--ȣ ĺ(Ԏ==ȣ){Ѭ;ă[ù/2]=ˏ ȧ}Ƞ ƍ=ù/2,ƪ=ǀ-ƍ;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(ƪ>=256&&ƪ ɱ ȏ){F ă;Ü ʃ native_sqrt(Ƕ é+ù,ȣ-ù,ă,ƪ,ˏ);ă ӭ(ă ü,ƍ,ȓ)ȧ}}
#endif
F f(Ƕ Ă+ù,Ƕ Ă+ȣ);f Ř(ƪ)Ĩ T Ơ=T(2)ˆ,ŧ=Ơ/ˏ;F g(ĥ(ƪ,32));g Г ˏ ø(κ Џ Į){T ʂ Ɠ(ý j=1;j<i ų)ʂ+=g Ƃ*g[i-j];g ƈ(f Ĕ-ʂ)*ŧ;}if(ƪ>256){ý æ=1 ǟ(æ<ƪ)æ<<=1 ĺ(ƪ<æ-æ/4)æ>>=1 Ȋ ŀ=æ/32,î=2*ŀ,k4=î ү,þ=(ƪ+ŀ-1)/ŀ;g=f.pre(ŀ).sqrt(ŀ);ƻ ć=g.ђ(ŀ);ć Ř(î);ʒ(ć)Ĩ Word f7=Ǘ(2*î)Ȋ ţ=__builtin_ctz((ϒ)î);T l9=T θ(Ǘ(1<<(ţ/2)))ĺ(ţ&1)l9*=T θ(441024591);Ɓ(1).ŧ(č(ć),î,f7)ѵ ʟ=Ü ɀ((þ-1)*k4);ɓ Ǥ=ʟ ӯ;ƻ ƌ(î);ƻ zero(î);g Ř(ƪ)ø(κ þ Į){ý Ų=i*ŀ,n Ч(ŀ,ƪ-Ų)ύ(q,î,l9 Ȅ)ø(Ȕ(i+1)/2;j+=6){Ù ɓ a[6],*b[6],*c[6];ý ơ Ч(6,(i+1)/2-j)ø(ˋ ơ Ȟ){a ѓ Ǥ+(j+k)*k4;b ѓ(j+k?Ǥ+(i-j-k)*k4:č(zero));c ѓ Ǥ+(i-j-k-1)*k4;}ł update=ѯ(ł number){è Ԃ=decltype(number)::Ĵ ĺ(j ε){Ϋ,Ƈ,ƅ(č(ƌ),a,b,c,ŀ,Ś);Ϋ,Ƈ,ƅ(č(ƌ)+ŀ,a,b,c,ŀ,Ƈ);}ŝ{Ϋ,Ś,ƅ(č(ƌ),a,b,c,ŀ,Ś);Ϋ,Ś,ƅ(č(ƌ)+ŀ,a,b,c,ŀ,Ƈ);}};ħ}Ű square_fold(č(ƌ),Ǥ+(i/2)*k4,ŀ,i&1);Ʋ(ƌ)ø(Ȕ ŀ ų)ƌ ˉ(Ų+j<ƪ?f[Ų+j]:ȓ)-ƌ Ƃ Ђ ĵ(ƌ ü,n,g ʷ Ų);}g ӭ(g ü,ƍ,ȓ)Ū g;}F h;ƻ Ǥ ĺ(Џ<ƪ){h=g.pre(16).ђ(16);Ǥ=h;Ǥ Ř(32);ntt(Ǥ);}while(Џ<ƪ){Ƞ m=Џ,n=2*m ĺ(ƪ-m<=32){g Ř(ƪ)ø(Ż m ϼ ƪ Į)g ƈ(f Ĕ-dot(g Ʒ 1,g ȝ-1,i-1))*ŧ;ϵ}Ƣ(n ɱ)Ĩ T in=T θ(Ǘ n);ƻ a=g,ο Ȉ(ƌ,a,a,n);Ʋ(ƌ);ż
(ƌ ü,ƌ ʷ m,ȓ)ø(Ż m Ȝ)ƌ ƈ(i<ƪ?f Ĕ:ȓ)-ƌ Ĕ*in;ntt(ƌ);Ȉ(ƌ,ƌ,Ǥ,n);F::Ζ(ƌ,Ơ);Ʋ(ƌ);g Ř(ĥ(n,ƪ))ø(Ż m ϼ Џ Į)g ƈ ƌ Ĕ*in;}g Ř(ƪ);g ӭ(g ü,ƍ,ȓ)Ū g;}T eval(T x)Ù{Ġ Ú Ü horner(Ƕ é,ǰ,x);T ans Ɠ(ł it=Ƕ rbegin();it!=Ƕ rend()ė it)ans=ans*x+*it Ū ans;}Ĉ onemul(ý d,T c){Ƣ(d>=0)ĺ(d ε){ˈ*=Ӽ+c а}Ć(Ż(ϥ)-1;i>=d;--i)(ˈ)Ĕ+=(ˈ)[i-d]*c;}Ĉ onediv(ý d,T c){Ƣ(d>=0)ĺ(d ε){ˈ/=Ӽ+c а}Ć(Ż d ϼ ϥ Į)(ˈ)Ĕ-=(ˈ)[i-d]*c;}F ǘ()Ù{Ù ǧ=ϥ;F r(n);Ġ{if(n>1)Ű weighted(č(r),č(ˈ)+1,Ԕ,1);}ŝ Ć(Ż 1 Ȝ)r[i-1]=(ˈ)Ĕ*i Έ}F integral()Ù{Ù ǧ=ϥ;Ƣ(ī(n)<ī(Ҳ));F r(n)ǌ&ђ=inverses(n);Ġ{if(n>1)Ű Ŏ(č(r)+1,č(ˈ),č(ђ)+1,Ԕ);}ŝ Ć(Ż 1 Ȝ)r ƈ(ˈ)[i-1]*ђ Ĕ Έ}F log(Ѡ)Ù{ƀ Ú ƾ(ź(ˈ).log(ǀ))ĺ(ǀ<0)Ϊ(!Ƕ empty()&&(ˈ)Ŭ==Ӽ)Ū Γ(ǀ,Ƈ);}F exp(Ѡ)Ù{Ġ Ú exp_impl ə(ǀ)ї Ú ƾ(ź(ˈ).exp(ǀ));}F pow(Ǉ k)Ù{ƀ Ú ƾ(ź(ˈ).pow(k));Ƣ(k>=0)Ĩ ǧ=ϥ ĺ(n ε)Ú{}ĺ(k ε){F r(n);r Г 1 Έ}if(k у)Ú ˈ ĺ(k<=4){F n3=Ź(ˈ,ˈ).pre(n)Ū k==2?n3:Ź(n3,k==3?ˈ:n3).pre(n);}ý ù Ē while(ù<n&&(ˈ)[ù]Ү)++ù ĺ(ù==n||ù>(Ԕ)/k)Ú F(n)Ȋ ƍ=ù*k Ȋ ǯ=n-ƍ Ĩ T Μ=(ˈ)[ù];
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(ǯ>=256&&ǯ<=(1<<24)ȏ){F r;Ü ʃ native_power(Ƕ é+ù,ǰ-ù,r,ǯ,T(k)Ȅ,Μ.pow(k));r ӭ(r ü,ƍ,ȓ)Έ}}
#endif
F f(Ƕ Ă+ù,Ƕ end());f Ř(ǯ);f=f.Γ(ǯ);f*=T(k);f=f.Ą exp_impl ԍ(ǯ);f*=Μ.pow(k);F r(n)ø(ö ǯ Į)r[ƍ+i]=f Ĕ Έ}Ø pair<F,F>divmod(ʧ λ)Ù{ƀ{ł[q,r]=ź(ˈ).divmod(ź(λ))Ū{ƾ(q),ƾ(r)};}ǧ=ϥ,m=ó(λ Û)ǟ(n&&(ˈ)[Ԕ]Ү)--n ǟ(m&&λ[m-1]Ү)--m;Ƣ(m)ĺ(n<m)Ú{{},pre(n)}Ȋ qn=n-m+1;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(m>2&&(Õ)m*qn>4096&&n ɱ ȏ){F q,r;Ü ʃ native_divide(Ƕ é,n,λ â,m,q,r);q.trim(),r.trim()Ū{Ɔ(q),Ɔ(r)};}}
#endif
F a=pre(n)ĺ(m<=2||(Õ)m*qn<=4096){F q(qn)Ĩ T ђ=λ[m-1]ˆ ø(Ż qn-1;i и i){q ƈ a[i+m-1]*ђ ø(Ȕ m ų)a[i+j]-=q Ĕ*λ Ƃ;}a Ř(m-1)ώ}F ra=a,rb(λ ü,λ ʷ m);Ø ˁ(ra ü,ra ƃ);Ø ˁ(rb ü,rb ƃ);F q=ra.pre(qn).Α(rb,qn);Ø ˁ(q ü,q ƃ);Ø ˁ(rb ü,rb ƃ);ý Ҫ=1 ǟ(Ҫ<Ŋ(m,qn))Ҫ<<=1 ĺ(ĥ(m-1,qn)>32&&Ҫ<m+ĥ(m-1,qn)-2){Ƣ(Ҫ ɱ);F Ŏ=rb,ť=q;Ŏ Ř(Ҫ);ť Ř(Ҫ)ѵ&t=Ɓ(Ҫ/16);t.convolution(č(Ŏ),č(ť),Ҫ);t.ŧ(č(Ŏ),m-1,Ǘ(Ҫ/8))ø(ö m-1 Į){ɨ-=Ŏ Ĕ ĺ(i+Ҫ<ó(ư))ɨ+=a[i+Ҫ];}a Ř(m-1)ώ}F Ŏ=Ź(rb.pre(m-1),q.pre(m-1));a Ř(m-1)ø(ö ʡ(ư,Ŏ Û)Į)ɨ-=Ŏ Ĕ ώ}Ⱦ ƙ ˤ ź ˤ ƾ ˤ Ɓ ͼ Ȯ ˤ Җ ˤ ntt ˤ ʒ ˤ Ʋ ˤ Ζ ˤ ɘ ˤ batch_inverse ˤ Ȉ ˤ inverse_extend ˤ dot ˤ ѫ ˤ sparse_dot ˤ intt ˤ mod_sqrt ˤ inverse_ntt_roots ˤ ntt_roots ˤ inverses ͼ Word=ӆ ǅ Word;F Γ(ý ǀ,Ǝ g7=Ś)Ù{
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(ǀ>=256&&ǀ<=(1<<24)&&ǰ>=64 ȏ){F r;Ü ʃ native_log(Ƕ é,ǰ,r,ǀ,g7)Έ}}
#endif
F Ǟ(ʡ(ǀ,ǰ))ĺ(Ǟ Û>1)Ű weighted(č(Ǟ)+1,č(ˈ)+1,Ǟ Û-1,1);F r=Ǟ.Α(ˈ,ǀ)ĺ(g7){ʦ&iv=inverses(ǀ-1);Ű Ŏ(č(r),č(r),č(iv),ǀ);}Ú r;}έ ŵ>F exp_impl(ý ǀ)Ù{if(ǀ<0)Ϊ(Ƕ empty()||(ˈ)Ŭ Ү)ѵ ơ=sparse_terms(ǀ,64);Ӂ=ơ Û<=64?ѫ(ơ):1 ĺ(ơ Û<=64&&(ǀ<=256||ơ Û<=24ULL Ԍ)){F g(ǀ);g Г 1 ĺ(ơ Û у&&ď у&&ǀ>=256){ƻ Ԓ(ǀ);Ü Ӧ(č(Ԓ),č(g),ǀ);ɘ(g,Ӽ,ơ Ŭ.χ)Ū g;}if(!ď)Ú g ǌ&iv=inverses((ǀ-1)/ď);T Ÿ=ŵ?T(ď)ˆ:Ӽ ø(ł&[j,x]:ơ)x*=ŵ?Ÿ:T(j/ď);ѫ(ơ)ø(Ǉ i=ď ϼ ǀ Ƽ ď)g ƈ sparse_dot(ơ,g,ó(i))*iv[i/ď]Ū g;}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(ǀ>=256&&ǀ<=(1<<24)ȏ){F r;Ü ʃ native_exp(Ƕ é,ǰ,r,ǀ,ŵ)Έ}}
#endif
ʦ&iv=inverses(ǀ)Ȋ g4 Ч(ǀ,32);F g(g4);g Г 1 ø(κ g4 Į){T ʂ Ɠ(ý j=1;j<=i&&j<ϥ ų)ʂ+=(ŵ?(ˈ)Ƃ:T(j)*(ˈ)Ƃ)*g[i-j];g ƈ ʂ*iv Ĕ;}if(g4==ǀ)Ú g ĺ(ǀ>256){ϓ ĺ(ǀ<æ-æ/4)æ>>=1 Ȋ ŀ=æ/32,î=2*ŀ,k4=î ү,þ=(ǀ+ŀ-1)/ŀ;g=pre(ŀ).Ą exp_impl<ŵ>(ŀ);ƻ ć=g.ђ(ŀ)ȹ Ǟ=ʟ ӯ,*Ǥ=Ǟ+þ*k4 ø(ö þ Į){ɓ d=Ǟ+i*k4 Ȋ Ų=i*ŀ Ȋ ɲ=Ŋ(0,ʡ(ŀ,ʡ(ǀ,ǰ)-Ų))ĺ(ɲ){Ĺ(ŵ)ĵ(č(ˈ)+Ų,ɲ,d)ї Ű weighted(d,č(ˈ)+Ų,ɲ,Ų);}ż(d+ɲ,d+ŀ,0u);ʒ(d,î);}ǅ Ц(Ǟ,î,þ,k4);ƻ ƌ(î);g ӗ ø(κ þ Į){ý Ų=i*ŀ,n Ч(ŀ,ǀ-Ų)ύ(q,î,norm)ø(Ȕ i;j+=7){Ù ɓ a[7],*b[7];ý ơ Ч(7,i-j)ø(ˋ ơ Ȟ){a ѓ Ǥ+(j+k)*k4;b ѓ Ǟ+(i-j-k)*k4;}ǎ}Ʋ(ƌ);ż(ƌ ʷ ŀ,ƌ ƃ,ȓ)Ђ Ű Ŏ(č(ƌ),č(ƌ),č(iv)+Ų,n);ż(ƌ ʷ n,ƌ ƃ,ȓ);ʒ(ƌ);Ű Ŏ(č(ƌ),č(ƌ),Ǥ,î);Ʋ(ƌ)ԕ(ƌ ü,n,g ʷ Ų);}Ú g;}F h=g.pre(16).ђ(16);ƻ Ǥ=h;Ǥ Ř(32);ntt(Ǥ)ǟ(Џ<ǀ){Ƞ m=Џ,n=2*m ĺ(ǀ-m<=32){ƻ Ǟ(ʡ(ǀ,ǰ))ø(κ ó(Ǟ Û)Į)Ǟ ƈ ŵ?(ˈ)Ĕ:T(i)*(ˈ)Ĕ;Ǟ Ř(Ŋ<ý>(1,Ǟ Û));g ӗ ø(Ż m ϼ ǀ Į){Ƞ þ=ʡ(i,Ǟ Û-1);g ƈ dot(Ǟ Ʒ 1,g ȝ-1,þ)*iv Ĕ;}ϵ}Ƣ(n ɱ)Ĩ T im=T θ(Ǘ m)Ĩ T in=T θ(Ǘ n);ƻ a=g,b(m),ο ż(b ü,b ƃ,ȓ)ø(κ m&&i<ϥ Į)b[i-1]=ŵ?(ˈ)Ĕ:T(i)*(ˈ)Ĕ;ntt(b);Ȉ(b,b,a,m);Ʋ(b);ƌ[m-1]=b[m-1]*im ø(ö m-1 Į)ƌ[m+i]=ϸ*im-T(i+1)*g Υ;ntt(ƌ);Ȉ(ƌ
,ƌ,Ǥ,n);Ʋ(ƌ)ø(Ż(ĥ(n,ǀ))-1;i>=m;--i)ƌ ƈ(i<ϥ?(ˈ)Ĕ*(ŵ?iv Ĕ:Ӽ):ȓ)+ƌ[i-1]*in*iv Ĕ;ż(ƌ ü,ƌ ʷ m,ȓ);ż(ƌ ʷ ĥ(n,ǀ),ƌ ƃ,ȓ);ntt(ƌ);Ȉ(ƌ,ƌ,a,n);Ʋ(ƌ);g Ř(ĥ(n,ǀ))ø(Ż m ϼ Џ Į)g ƈ ƌ Ĕ*in;}Ú g;}Ħ Ø pair<ý,T>>sparse_terms(ý ǀ,ý i2)Ù{Ħ Ø pair<ý,T>>ơ ø(κ ʡ(ǀ,ǰ)&&ó(ơ Û)<=i2 Į)if((ˈ)Ĕ!=ȓ)ơ η(i,(ˈ)Ĕ)Ū ơ;}Ĉ trim(){ɞ(ˈ);}}ͼ fps=Ď<fps_mint>;
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā Ü bostan_detail{í inverse_twisted(Ù NTT&plan,Ù Ҿ in,Ҿ į,Ù Ҿ ж,ǧ){Ę N=NTT ĺ(n==8){ł z=Ƹ Σ 4,ƅ(Ƹ Σ 2,ƅ(Ƹ Σ 1,ƅ(Ӆ(in),0),0),0);N::ŗ(į,N::ɏ(z,Ӆ(ж)))а}Ć(Ȕ n Ӕ){ł z=Ӆ(in+j);z=Ƹ Σ 4,ƅ(Ƹ Σ 2,ƅ(Ƹ Σ 1,ƅ(z,j/8),j/8),j/8);N::ŗ(є j,z);}ý h=8 ø(;4*h<n;h*=4)Ƹ radix4_8 ԍ(į,h,n)ĺ(4*h==n){ʦ im ʖ(Ƹ ô ƹ)ǌ iq ʖ(Ƹ ſ ƹ)ø(ö h ƞ){ʦ x0=Ӆ(є i),x1=Ӆ(є h+i)ǌ x2=Ӆ(į ʳ h+i),x3=Ӆ(є 3*h+i)ǌ u=N::ɾ(x0,x1),v=N::sub8(x0,x1),s=N::ɾ(x2,x3)ǌ t=N::Ʀ(N::sub8(x2,x3),im,iq);N::ŗ(є i,N::ɏ(N::ɾ(u,s),Ӆ(ж+i)));N::ŗ(є h+i,N::ɏ(N::ɾ(v,t),Ӆ(ж+h+i)));N::ŗ(į ʳ h+i,N::ɏ(N::sub8(u,s),Ӆ(ж ʳ h+i)));N::ŗ(є 3*h+i,N::ɏ(N::sub8(v,t),Ӆ(ж+3*h+i)));}}ŝ{Ć(ö h ƞ){ʦ x=Ӆ(є i),y=Ӆ(є h+i);N::ŗ(є i,N::ɏ(N::ɾ(x,y),Ӆ(ж+i)));N::ŗ(є h+i,N::ɏ(N::sub8(x,y),Ӆ(ж+h+i)));}}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ T bostan_mori(Ȼ p,Ȼ q,Ǉ k){Ѽ ͼ ƻ=џ;ƙ ͼ Word=ӆ Ü Ǆ::Word;ƀ{Ú T θ(bostan_mori Ӹ(ҹ(p),ҹ(q),k)Ȅ);}ŝ{Ƣ(k>=0&&!q ű&&q Ŭ!=ȓ);ɞ(p);ɞ(q)ĺ(p ű)Ú 0 ѵ ʑ=[](ҥ p,ҥ q,ң){F ć(q ü,q ʷ ʡ(k+1,Ѹ));ć=ć.ђ(k+1)Ū ǅ dot(p â,ć Ʒ k,ʡ(k+1,p Û));}ĺ(k<32)Ú ʑ(p,q,k);T b3 Ē if(p Û>=Ѹ){ł[ť,ɑ]=p.divmod(q)ĺ(k<(Ǉ)ť Û)b3=ť Ƭ;p=Ɔ(ɑ);}if(p ű)Ú b3 ĺ(Ѹ==2)Ú b3+p Ŭ/q Ŭ*(-q ƹ/q Ŭ).pow(k)Ȋ d=ó(Ѹ)-1 ĺ(k<d)Ú b3+ʑ(p,q,k);ǧ=1 ǟ(n<d)n<<=1;Ƣ(n<=(1<<22))Ĩ T Ơ=T(2)ˆ,ђ=T(n)ˆ;T d4=1 Ĩ T ɰ=T(3).pow((Ҳ-1)/(2*n));ƻ ж(n),twists(n)ѵ ʟ=Ü ɀ(4*Õ(n));ɓ a=ʟ ӯ,*b=a ʳ n ԕ(Ȯ(p),p Û,a);ż(a+p Û,a ʳ n,0u)ԕ(Ȯ(q),Ѹ,b);ż(b+Ѹ,b ʳ n,0u)ѵ&plan=ǅ Ɓ(n)ø(Е)Ȯ(twists)ƈ Ƹ ſ Ĕ*(0u-Ҳ);T Μ=q.back();ż(ж ü,ж ƃ,Ӽ);ǅ ɘ(ж,ђ,ɰ);Ƹ Ą Ҭ(a,2*n);Ƹ Ą Ҭ(b,2*n)ѵ doubling=ѯ(ɓ v,T i0){ɓ ȇ=v+n;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(n>=8 ȏ){Ü bostan_detail::inverse_twisted(plan,v,ȇ,Ȯ(ж),n);ȇ Г Ű sub(ȇ Ŭ,(T(2)*i0).v);}ŝ
#endif
{ĵ(v,n,ȇ);Ƹ Ą fft ԍ(ȇ,n);ȇ Г Ű sub(ȇ Ŭ,(T(2*n)*i0).v);Ű Ŏ(ȇ,ȇ,Ȯ(ж),n);}Ƹ Ą Ҭ(ȇ,n);}ǟ(k){if(k&1)Ű reduce_pairs ԍ(a,b,Ȯ(twists),n)ї Ű reduce_pairs ə(a,b,Ȯ(twists),n);d4*=Ơ;Μ=(d&1?-Μ:Μ)*Μ;k>>=1 ĺ(k<d)ϵ doubling(a,ȓ);doubling(b,d==n?Μ:ȓ);}if(k){ƻ aa(n),bb(n)ԕ(a,n,Ȯ(aa))ԕ(b,n,Ȯ(bb));ǅ intt(aa);ǅ intt(bb)ĺ(d==n)bb Ŭ-=Μ Ū b3+d4*ʑ(aa,bb,k);}T j4=0,d8 Ɠ(Е)j4+=T ɪ(ɨ),d8+=T ɪ(ϸ)ĺ(d==n)d8-=T(n)*Μ Ū b3+d4*j4/d8;}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
Ӏ{ā ȗ{Ę ʃ Ě ͼ ʃ Č ͼ ʃ Half ͼ ʃ ϴ;ā kernels{Ę ĭ=Ϸ;ā column_detail=ʃ ë;έ Inverse>Ɨ Ĉ İ(ҳ a,Ϩ,Õ ҕ,Ɣ x,Ɣ y){Ę ā column_detail Ƥ wx(x Ŭ,x ɚ),wy(y Ŭ,y ɚ),wz(y ƹ,y[9])Ǖ ǳ{ġ f0=a Ƃ,f1=a Ǻ,f2=a ɋ,f3=a Ɋ Њ(Inverse){ġ ab ӏ(ƫ(f0,f1)),cd ӏ(ƫ(f2,f3))ι amb Ҝ(ǘ(f0,f1),wy),cmd Ҝ(ǘ(f2,f3),wz);a ˉ ʗ(ϑ),a Ǻ ӏ(ϐ);a ɋ Ҝ(Ќ,wx),a Ɋ Ҝ(Ϗ,wx);}ŝ{ġ lo ӏ(f0),hi ӏ(f1),xc Ҝ(f2,wx),xd Ҝ(f3,wx)ι ac ӏ(ƫ(lo,xc)),amc=low_difference(lo,xc)ι bd Ҝ(ƫ(hi,xd),wy),bmd Ҝ(ǘ(hi,xd),wz);a ˉ ƫ(ac,bd),a Ǻ=ǘ(ac,bd);a ɋ=ƫ(amc,bmd),a Ɋ=ǘ(amc,bmd);}}}í ԉ(ҳ a,Ϩ,Õ ҕ,Ɣ x,Ɣ y){İ ə(a,h,ҕ,x,y);}í ǔ(ҳ a,Ϩ,Õ ҕ,Ɣ x,Ɣ y){İ ԍ(a,h,ҕ,x,y);}í forward_even_columns(ҳ a,Ϩ,Ɣ x,Ɣ y){İ ə(a,h,1,x,y);}í inverse_even_columns(ҳ a,Ϩ,Ɣ x,Ɣ y){İ ԍ(a,h,1,x,y);}}Ϥ{Ę ā ʃ ë ͼ ʃ ä ŭ;Ǒ ǒ Ь}}ā ȗ{Ϥ{ò ɿ Ʉ=32;Ί compose_length(ƿ){Ú ɬ(ʘ(n,256));}í compose_direct(О,ĝ g,Ĝ h){Ģ n=h Û;ż(h ü,h ƃ,0)ø(ɮ Ч(Ǳ,n);i-->0;){Ć(ȕ=n;k-->1;){ŉ ʂ Ɠ(ʍ=1;j<=k&&j<ȸ ų)ʂ+=ŉ(g Ƃ)*h[k-j]Ұ;h ѓ Ö(ʂ Ұ);}h Г f Ĕ;}}ӂ Ґ(Ö x){ğ y=x?2*ӓ*x:0 Ū y>=kP?y-kP:y;}Ǹ Ӣ(ɍ){Ú ą(ê(ɟ,x),ê(đ,x));}Ǹ even_lanes(ɍ,ĭ y){ġ t=_mm256_castps_si256
(_mm256_shuffle_ps(р(x),р(y),0x88))Ū ҭ(t,0xD8);}í spread(ɍ,ĭ(&į)ǥ){į Г Љ(ɦ(x));į ƹ=Љ(_mm256_extracti128_si256(x,1));}Ǹ ϕ(ɍ){Ú ń(ƫ(x,x),kP);}í ʈ(ĭ(&r)ɚ){ĭ t ɚ,u ɚ;
#pragma GCC unroll 4
Ć(ö 8 Ƽ 2)ͻ,t Υ Ȁ(r Ĕ,r Υ);
#pragma GCC unroll 2
Ć(ö 8 Ƽ 4)
#pragma GCC unroll 2
Ć(Ϟ)u[i ʳ j]=ɻ(t[i+j],t[i+j+2]),u[i ʳ j+1]ǿ(t[i+j],t[i+j+2]);
#pragma GCC unroll 4
Т ѹ,з}Ǹ Ŏ(ɍ,ĭ y){Ú ß(x,y);}Ǹ ƒ(ɍ){Ú ð(x,x);}Ǹ plus(ɍ,ĭ y){Ú ð(x,y);}Ǹ ʁ(ɍ,ĭ y){Ú _mm256_sub_epi64(x,y);}Ǹ Ų(){Ú _mm256_set1_epi64x(ú Ǉ>(4*ŉ(kP)*kP));}Ą<Õ N>ˮ Ȗ(ġ(&x)[N],ĭ(&į)[N]){
#pragma GCC unroll 8
Ć(Ů N Į)į ƈ ñ(x Ĕ,32);}Ǹ reduce_sums(н,ĭ ȇ){ȱ,p=ɟ;Ĥ Ú ǝ;}ˮ graeffe_sums(ġ(&c)ɚ,ġ(&tc)ɚ,ĭ(&ʂ)ɛ){ʦ x=[&c](ý i,ý j){Ú Ŏ(c Ĕ,c Ƃ);}ǌ t=[&c,&tc](ý i,ý j){Ú Ŏ(c Ĕ,tc Ƃ);};ʂ Г ʁ(ʁ(plus(plus(x(0,0),ƒ(t(2,6))),plus(t(4,4),Ų())),ƒ(t(1,7))),ƒ(t(3,5)));ʂ ƹ=ʁ(ʁ(plus(plus(ƒ(x(0,2)),ƒ(t(4,6))),Ų()),x(1,1)),plus(ƒ(t(3,7)),t(5,5)));ʂ ǥ=ʁ(ʁ(plus(plus(ƒ(x(0,4)),x(2,2)),plus(t(6,6),Ų())),ƒ(x(1,3))),ƒ(t(5,7)));ʂ Ѐ=ʁ(ʁ(plus(plus(ƒ(x(0,6)),ƒ(x(2,4))),Ų()),ƒ(x(1,5))),plus(x(3,3),t(7,7)));}ˮ ӡ(ġ(&c)ɚ,ġ(&tc)ɚ,ĭ(&į)ɛ){н ɛ,ȇ ɛ,c2 ɚ,tc2 ɚ;graeffe_sums(c,tc,Ŧ);Ȗ(c,c2),Ȗ(tc,tc2);graeffe_sums(c2,tc2,ȇ);
#pragma GCC unroll 4
ʢ į ѓ ʗ(α);}ˮ ΐ(Ɣ ʝ,ĭ(&c)ɚ){
#pragma GCC unroll 8
ѽ c ƈ Ō(ʝ ү*i);ʈ(c);}έ kNegative>ˮ scaled_leaves(ġ(&c)ɚ,Ù я&s,ý ù,ĭ(&tc)ɚ){
#pragma GCC unroll 8
Ć(Ȕ 8 ų){Ҽ ń(ŭ(c Ƃ,s),kP);tc ˉ j<ù?đ:kNegative?Ӣ(x):x;}}í ҩ(ġ(&x)ɛ,ġ(&y)ɛ,Ù я&ć,Ȩ){ĭ r ɚ;
#pragma GCC unroll 4
Т r ƈ ʗ(ƫ(x Ĕ,y Ĕ)),r[i+4]Ҝ(ǘ(x Ĕ,y Ĕ),ć);ʈ(r);
#pragma GCC unroll 8
ѽ Š(į Ƨ i,r Ĕ);}Ɯ ȋ{М,*ô;ċ Õ č(Ό){Ú 2 ӌ(ã ϙ(ĩ(m)+2));}ȋ(Ĝ i6,Ό,Ù Č*m9=Ƿ){if(m9&&m9->Δ()>=ĩ(m)+2){ķ=m9->ķ(),ô=m9->ô()а}ł*Ù ı=i6 â ѵ*Ù ć=ı Я(ã ϙ(ĩ(m)+2));ķ=ı,ô=ć;ż(i6 ü,i6 ƃ,0u);ã build_table(ı,m/4,ã kRoots Ŭ);ã build_table(ć,m/4,ã kRoots ƹ);}};Ɯ PairWeights{я s,ć;PairWeights(ί,ȕ):s(entries(Ķ.ķ,k)),ć(entries(ѝ,k)){}};Ą<ý ҏ>ˮ product_sums(ġ(&a)ɚ,ġ(&c)ɚ,ġ(&tc)ɚ,ĭ(&ʂ)ɛ){
#pragma GCC unroll 4
ʢ{ĭ s=Ų();
#pragma GCC unroll 8
ѽ{Ƞ j=Ӷ+ҏ-i,l=j+8 ĺ(j>=0)s=j%2?ʁ(s,Ŏ(ɨ,c Ƃ)):plus(s,Ŏ(ɨ,c Ƃ))ĺ(l<8)s=l%2?ʁ(s,Ŏ(ɨ,tc[l])):plus(s,Ŏ(ɨ,tc[l]));}ʂ ѓ s;}}Ą<ý ҏ>ˮ Ӡ(ġ(&a)ɚ,ġ(&c)ɚ,ġ(&tc)ɚ,ĭ(&į)ɛ){н ɛ,ȇ ɛ,a2 ɚ,c2 ɚ,tc2 ɚ;product_sums<ҏ>(a,c,tc,Ŧ);Ȗ(a,a2),Ȗ(c,c2),Ȗ(tc,tc2);product_sums<ҏ>(a2,c2,tc2,ȇ);
#pragma GCC unroll 4
ʢ į ѓ ʗ(α);}έ kNumerator>Ɯ FirstLevelProducts{ċ è ý kOutputs=kNumerator?5:3 Ĩ ȋ*Ķ;Ɣ q1,*q2,*p1;Ȩ[kOutputs];ѱ Ǜ(ȕ)Ù{ы a[kOutputs]ɛ,b[kOutputs]ɛ;ȟ ə(16*k,ş.s,a);ȟ ԍ(16*k+8,ş.s,b);
#pragma GCC unroll 5
Ć(ö kOutputs Į)ҩ(ɨ,ϸ,ş.ć,į Ĕ Ƨ k);}ʙ Ĉ ȟ(Õ at,Ù я&s,ĭ(&r)[kOutputs]ɛ)Ù{ĭ c1 ɚ,c2 ɚ,a ɚ,t1 ɚ,t2 ɚ;ΐ(q1+at,c1),ΐ(q2+at,c2)Њ(kNumerator)ΐ(p1+at,a)Φ(c1,s,kNumerator?2:4,t1)Φ(c2,s,1,t2);ӡ(c1,t1,r Ŭ);ӡ(c2,t2,r ƹ);Ӡ<0>(c1,c2,t2,r ǥ)Њ(kNumerator){Ӡ<1>(a,c1,t1,r Ѐ);Ӡ<1>(a,c2,t2,r ɛ);}}};ԃ Ж(ý lg){Ú ʀ(Ȍ Ŭ[lg],Ö((ŉ(1)<<31)Ұ));}Ɯ LevelBottom{Ǐ Ƈ,Ƚ=Ś Ĩ ȋ*Ķ;Ņ f4;Ț{ǂ{Ķ->ķ}(a,þ,ù)ĺ(f4)Ć(Ů þ Ƽ 4)Ǜ(a+32*i,2*(Ў i));}ѱ Ǜ(Ɣ q,ȕ)Ù{ы va ɛ,vb ɛ;ȟ ə(q,ş.s,va);ȟ ԍ(q+8,ş.s,vb);ҩ(va,vb,ş.ć,f4 Ƨ k);}ʙ Ŵ ȟ(Ɣ q,Ù я&s,ĭ(&v)ɛ){ĭ c ɚ,tc ɚ;ΐ(q,c)Φ(c,s,4,tc);ӡ(c,tc,v);}};Ɯ ͱ{Ǐ Ś,Ƚ=Ƈ Ĩ ȋ*Ķ;Ɣ f,*q;Ą<ý ҏ>ĉ Ŵ Ͽ(ġ(&p)ɛ,ġ(&wp)ɛ,ġ(&c)ɚ,ĭ(&ʂ)ɛ){
#pragma GCC unroll 4
ʢ{ġ ù=Ŏ(p Ŭ,c[Ӷ+ҏ]);ĭ s=ҏ?ʁ(Ų(),ù):ù;
#pragma GCC unroll 3
Ć(κ 4 Į){ġ term=Ŏ(i<=k?p Ĕ:wp Ĕ,c[2*((k-i+4)%4)+ҏ]);s=ҏ?ʁ(s,term):plus(s,term);}ʂ ѓ s;}}ĉ Ŵ leaf(ġ(&p)ɛ,ġ(&wp)ɛ,ġ(&c)ɚ,ĭ(&į)ɚ){ĭ p2 ɛ,wp2 ɛ,c2 ɚ,Ŧ ɛ,ȇ ɛ;Ȗ(p,p2),Ȗ(wp,wp2),Ȗ(c,c2);Ͽ<0>(p,wp,c,Ŧ);Ͽ<0>(p2,wp2,c2,ȇ);
#pragma GCC unroll 4
ʢ į[Ӷ]=α;Ͽ<1>(p,wp,c,Ŧ);Ͽ<1>(p2,wp2,c2,ȇ);
#pragma GCC unroll 4
ʢ į[Ӷ+1]=α;}ѱ Ɩ(Õ p,ĭ(&r)[16])Ù{ɍ ɚ,c ɚ,pa ɛ,pb ɛ,spa ɛ,spb ɛ,ra ɚ,rb ɚ;
#pragma GCC unroll 8
ѽ x ƈ Ō(f Ƨ(p+i));ʈ(x)Ĩ я s=entries(Ķ->ķ,p);
#pragma GCC unroll 4
Т{ġ t Ҝ(x[i+4],s);pa ƈ ů(ƫ(x Ĕ,t)),pb ƈ ů(ǘ(x Ĕ,t));spa ƈ ń(ŭ(pa Ĕ,s),kP),spb ƈ Ӣ(ń(ŭ(pb Ĕ,s),kP));}ΐ(q ү*p,c);leaf(pa,spa,c,ra);ΐ(q ү*p+8,c);leaf(pb,spb,c,rb);ʈ(ra),ʈ(rb);
#pragma GCC unroll 8
ѽ r ɔ=ra Ĕ,r Ʊ=rb Ĕ;}ĉ Ŵ finish(ί,ġ(&r)[16],Õ g,Ȩ){
#pragma GCC unroll 4
Т{ϣ={r[4*i],r[4*i+1],r[4*i+2],r[4*i+3]};τ(f,Τ(ѝ,g+i));
#pragma GCC unroll 4
ҿ Š(є 32*i ϛ,f Ѱ);}}Ǣ{Ć(ǈ þ;j+=4,į+=128){ĭ r[16];Ɩ(2*(Ў j),r);finish(*Ķ,r,Ў j,į);}}};Ɯ CompositionSumBottom{Ǐ Ś,Ƚ=Ƈ;ͱ ơ ǥ;Ǣ{Ć(ǈ þ;j+=4,į+=128){ĭ r[16],r2[16];ơ Ŭ.Ɩ(2*(Ў j),r);ơ ƹ.Ɩ(2*(Ў j),r2);
#pragma GCC unroll 16
Ć(ö 16 Į)r ƈ ʗ(ƫ(r Ĕ,r2 Ĕ));ͱ::finish(*ơ Ŭ.Ķ,r,Ў j,į);}}};Ϣ forward_with(ҽ,Õ æ,ί,Ϧ){Ù ǆ Ŝ(Ķ.ķ,ѝ,b9)Ĩ Ɏ in(ʰ,æ,0)Ω{ƽ 4;forward_top4(in,v,h,Ķ.ķ)Ȳ ő(a ʥ t*h,h,t);}ŝ{ƽ 2;forward_top2(in,v,h)Ѵ;}}Ϣ Ɇ(ҽ,ί,Ϧ,Ö ŧ,Π){Ù ǆ Ŝ(Ķ.ķ,ѝ,b9)Ω{ƽ 4 Ȳ ő(a ʥ t*h,h,t);inverse_top4(v,h,ǁ,ѝ,Ʈ(ŧ));}ŝ{ƽ 2 Ѵ;Ӥ(v,h,ǁ,ŧ);}}í ԉ(Ң,Ϩ,Õ b,М,ȕ){ҳ Ù v=ț(a)ĺ(b ε)Ú ƥ forward_even_columns(v,h,ķ+υ(k),ķ+υ(Ӷ));ƥ ԉ(v,h,Ǽ b,ķ+υ(k),ķ+υ(Ӷ));}í ǔ(Ң,Ϩ,Õ b,Ɣ ô,ȕ){ҳ Ù v=ț(a)ĺ(b ε)Ú ƥ inverse_even_columns(v,h,ô+υ(k),ô+υ(Ӷ));ƥ ǔ(v,h,Ǽ b,ô+υ(k),ô+υ(Ӷ));}í forward_half(Ң,Ϩ,Ǝ high,Ù Τ&w){Ć(Ņ x=a;x<ϋ h;x+=8){ġ u ӏ(Ō(x))ĺ(high){ġ b=Ō(x Ƨ h),yb Ҝ(b,w.y),zb Ҝ(b,w.z);Š(x,ƫ(u,yb)),Š(x Ƨ h,ǘ(u,yb));Š(x ү*h,ƫ(u,zb)),Š(x+24*h,ǘ(u,zb));}ŝ{ġ xc Ҝ(Ō(x ү*h),w.x),ʂ ӏ(ƫ(u,xc)),ɸ=low_difference(u,xc);Š(x,ʂ),Š(x Ƨ h,ʂ),Š(x ү*h,ɸ),Š(x+24*h,ɸ);}}}í inverse_half(Ң,Ϩ,Ǝ high,Ù Τ&w){Ć(Ņ x=a;x<ϋ h;x+=8){ġ f0=Ō(x),f1=Ō(x Ƨ h),f2=Ō(x ү*h),f3=Ō(x+24*h)ι ab ӏ(ƫ(f0,f1)),cd ӏ(ƫ(f2,f3));Š(x,ʗ(ϑ))ĺ(high)Š(x Ƨ h,Ӑ)ї Š(x ү*h,ŭ(Ќ,w.x));}}Ő ƭ>Ũ Pruned{Ⱥ Pruned(ί,Ϧ,Õ b):ӟ(Ķ.ķ,
ѝ,b9),ʻ(Ķ),b_(b){}Ĉ ı(Ң,Ԅ,ȕ)Ù{ƽ 4,ʗ=Õ(ĩ(h))ĺ(nv<=kForwardFull||ʗ+1<b_)Ú ӟ.ϖ(a,nv,k)ĺ(ʗ>b_){ԉ(a,h,b_,ʻ.ķ,k)Ȳ ı(ϋ t*h,h,4*k+t)а}forward_half(a,h,ʗ+1==b_,Τ(ʻ.ķ,k))Ȳ ӟ.γ}Ĉ ć(Ң,Ԅ,ȕ)Ù{ƽ 4,ʗ=Õ(ĩ(h))ĺ(nv<=kInverseFull||ʗ+1<b_)Ú ӟ.ϖ(a,nv,k)ĺ(ʗ>b_){Κ ć(ϋ t*h,h,4*k+t)Ū ǔ(a,h,b_,ʻ.ô,k);}Κ ӟ.γ inverse_half(a,h,ʗ+1==b_,Τ(ʻ.ô,k));}Ⱦ ċ ɿ kForwardFull=64,kInverseFull=16;ǆ<ƭ>ӟ Ĩ ȋ&ʻ;Õ b_;};Ϣ forward_pruned(ҽ,Õ æ,Õ b,ί,Ϧ){Ù Pruned<ƭ>Ҩ(Ķ,b9,b)Ĩ Ɏ in(ʰ,æ,0)Ȣ nv=ư/8,ҕ=Ǽ b ѵ*v=ț(ʰ)ι p=ɟ,zero=đ ǡ{ƽ 4 Ƥ z(Ķ.ķ ƹ,Ķ.ķ[9])Ǖ{ǳ{ġ x0=in(j),x1=in(j+h),x2=in(j ʳ h),x3=in(j+3*h)ι ac=ƫ(x0,x2),amc=ê(ƫ(x0,p),x2)ι bd=ƫ(x1,x3),zbmd Ҝ(ê(ƫ(x1,p),x3),z);v ˉ ƫ(ac,bd),Ӊ=ǘ(ac,bd);v ɋ=ƫ(amc,zbmd),v Ɋ=ǘ(amc,zbmd);}ȳ v ɋ=v Ɋ=zero;}Κ Ҩ.ı(a ʥ t*h,h,t);}ŝ{ƽ 2 Ǖ{ǳ{ġ x0=in(j),x1=in(j+h);v ˉ ƫ(x0,x1),Ӊ=ê(ƫ(x0,p),x1);}ȳ zero;}Ҩ.ı(ʰ,h,0);Ҩ.ı(a ʥ h,h,1);}}Ϣ inverse_pruned(ҽ,Õ b,ί,Ϧ,Ö ŧ,Π=ȉ){Ù Pruned<ƭ>Ҩ(Ķ,b9,b)Ȣ nv=ư/8,ҕ=Ǽ b ѵ*v=ț(ʰ)Ƥ s(ŧ)ǌ е Ѓ Ĩ Ǝ lower=ǁ!=ƨ,ʓ=ǁ!=Ʃ ǡ{ƽ 4 Ȳ Ҩ.ć(a ʥ t*h,h,t)Ƥ z(ѝ ƹ,ѝ[9])Ǖ ǳ{ġ x0=v Ƃ,x1=Ӊ,x2=v ɋ,x3=v Ɋ ι ab ӏ(ƫ
(x0,x1)),cd ӏ(ƫ(x2,x3))ι amb ӏ(ǘ(x0,x1)),cmd Ҝ(ǘ(x2,x3),z)ĺ(lower)v ˉ е(ϑ),Ӊ=е(ϐ)ĺ(ʓ)v ɋ=е(Ќ),v Ɋ=е(Ϗ);}}ŝ{ƽ 2;Ҩ.ć(ʰ,h,0);Ҩ.ć(a ʥ h,h,1)Ǖ ǳ{ġ x0=v Ƃ,x1=Ӊ ĺ(lower)v ˉ е(ƫ(x0,x1))ĺ(ʓ)Ӊ=е(ǘ(x0,x1));}}}Ũ Ԉ{Ϳ Ԉ(Є):rest_(ĸ){}ċ ɿ č(ƿ){Ú à(n)+8;}Ĝ ś(ƿ){Ģ skip=(0-Λ(rest_ â)/4)%16 ĺ(skip Я(n)>rest_ Û)ǩ ǫ s=rest_ Ė(skip,n);rest_=rest_ Ė(skip Я(n))Ū s;}Ⱦ Ĝ rest_;};Ɯ Levels{Ό;ý lg;ȋ Ķ;Ĝ o5;Ĝ ȍ(ý s)Ù{Ú o5 Ė(Õ(s)ӌ(4*m),4*m);}Ĝ д(ɮ)Ù{Ú ȍ(1)Ė(i ӌ(m),m);}Ĝ column_transform(ȕ)Ù{Ú ȍ(lg-3)Ė((k-1)ӌ(m/4),m/4);}ċ Õ ĸ(Ό){Ú à(ȋ Ư(m))+Ԉ Ư(Õ(ĩ(m))ӌ(4*m));}Levels(Õ æ,Є,Ù Č*m9=Ƿ):m(æ),lg(ĩ(æ)),Ķ(ĸ Ȓ(ȋ Ư(æ)),æ,m9){Ԉ carve(ӿ(à(ȋ Ư(æ))));o5=carve ņ(Õ(lg)ӌ(4*m));}};í next_level(ҽ,Õ ď,Õ φ){Ģ Ơ=ď/2,top=φ Ԍ;Ȫ(a ü,a ʊ(Ơ),a ʊ(top));a[top]=a[top]?a[top]-1:Ͼ 1;a Г 1;ż(a ʷ 1,a ʊ(Ơ),0);}í first_level(Ƶ,ĝ g,Levels&ţ){Ģ m=ţ.m,h=m/2,k9=m/4 Ĩ ȋ&Ķ=ţ.Ķ Ĩ ĝ v=ȁ(3)Ȓ(h)ǫ t1=ţ.д(0),t2=ţ.д(1)Ȣ Ŧ=ʘ(ĥ(h,(ȸ+1)/2),1);ʍ Ɠ(;j+8<=Ŧ&&2*j ү<=ȸ Ӕ){Ҽ even_lanes(ɂ(õ ġ*>(g Ʒ 2*j)),ɂ(õ ġ*>(g Ʒ 2*j+8)));Š(t1 Ʒ j,Ӣ(ϕ(x)));}Ć(;j<Ŧ ų)t1 ˉ Ґ(g[2*j]);t1 Ŭ Ē ż
(t1 ʊ(Ŧ),t1 ʊ(h),0)ǫ q=ȁ(2)Ә Š(q Ʒ h+i,ϕ(even_lanes(Ō(t1 Ʒ 2*i),Ō(t1 Ʒ 2*i+8))))Ȇ(t1 Ȓ(h),0,t1)Ȇ(v,0,t2);Ȩ Ѐ ø(Ů 3 Į)į ƈ ȁ(3)Ʒ à(m)+i ӌ(h)Ĩ FirstLevelProducts ə Ɩ{&Ķ,t1 â,t2 â,Ƿ,{į Ŭ,į ƹ,į ǥ}}ø(ѿ m/16;k+=8)Ɩ.Ǜ(k)ø(Ņ x:į)Ɇ(Ĝ(x,h),Ķ,Ɉ{ѝ,x},Ж(ţ.lg-1),Ʃ);q Г 1;ż(q ʷ 1,q ʊ(k9),0)Ә{ġ v2=ϕ(even_lanes(Ō(v Ʒ 2*i),Ō(v Ʒ 2*i+8)));Š(q Ʒ 2*h+i,ń(ƫ(v2,Ō(į Ŭ+i)),kP));Š(q Ʒ 3*h+i,ϕ(Ō(į ǥ+i)));}ĵ(į ƹ,k9,q ʊ(4*h));}í first_level_transposed(Ƶ,Ù Levels&ţ,ĝ in,ɐ){Ģ m=ţ.m,h=m/2,k9=m/4 Ĩ ȋ&Ķ=ţ.Ķ ǌ ν=ѯ(ȕ){Ú in Ė(2*m+k*h,k9);};Ĝ tp Ѐ ø(ѿ 3 Ȟ){tp ѓ į Ė(k ӌ(h),h)Ȇ(ν(k),0,tp Ƭ);}Ɣ q1=ţ.д(0)â,*q2=ţ.д(1)â ǌ term=ѯ(ȕ,Ɣ q){Ú ͱ{&Ķ,tp Ƭ â,q};}ø(Õ r=2;r<4 ė r){Ù Ĝ ă=į Ė(r*m,m);Ɇ(ă,Ķ,CompositionSumBottom{{term(r-1,q1),term(r-2,q2)}},Ȍ ƹ[ţ.lg],Ʃ)Ĩ ĝ a5=ν(r)Ә{ɍ ǥ;spread(Ō(a5 ȝ),x);Š(ă Ʒ 2*i,ń(ƫ(Ō(ă Ʒ 2*i),x Ŭ),kP));Š(ă Ʒ 2*i+8,ń(ƫ(Ō(ă Ʒ 2*i+8),x ƹ),kP));}}}Ą<Õ K>Ĉ Ŗ(ί,Ù ɝ*Ǜ,ɐ,Π){ϗ<K>b9{}ø(ѿ K Ȟ)b9.ơ ѓ{{Ķ.ķ,ѝ,Ǜ Ƭ.b â},Ǜ Ƭ.ʰ};Ɇ(į,Ķ,b9,Ȍ ƹ[ĩ(ǲ)],ǁ);}í ʏ(ĝ a,Ö c,Ĝ t){Œ f(c)ø(Ů ư ƞ)Š(t ȝ,ń(ŭ(Ō(a ȝ),f),kP));}í third_last_level(Ƶ
,Ù Levels&ţ,Ù Ĝ(&c)ɛ){Ģ m=ţ.m,k9=m/4,e1=m/8 Ĩ ĝ q=ȁ(ţ.lg-3)ǫ n9=ȁ(ţ.lg-1);Ĝ ļ ɚ,tc Ҹ(ʶ 8 Ȟ){ļ ѓ n9 Ė((k-1)ӌ(e1+1),e1+1);tc ѓ ţ.column_transform(k);}Ԑ{ɍ ɚ;
#pragma GCC unroll 8
ԏ
#pragma GCC unroll 7
Ć(ʶ 8 Ȟ)Š(ļ Ƭ Ʒ r,k%2?Ӣ(x Ƭ):x Ƭ);}Ć(ʶ 8 Ȟ){ğ x=q[16*e1+k];ļ Ƭ[e1]=k%2&&x?Ͼ x:x;}Ć(ʶ 8 Ȟ)t.ı(ļ Ƭ,0,tc Ƭ);Ĝ ƒ Ѐ,n3 ɛ ǫ Ͽ=n9 Ė(7 ӌ(e1+1))ø(Ů 3 Į)ƒ ƈ Ͽ Ė(i ӌ(k9),k9)У n3 ѓ Ͽ Ė((k+2)ӌ(k9),k9);ʏ(tc[5],2,ƒ Ŭ),ʏ(tc Ѐ,ӓ,ƒ ƹ),ʏ(tc ɛ,ӓ,ƒ ǥ)Ĩ ɝ ù ƹ={{tc ƹ,tc ƹ}},χ ǥ={{tc ǥ,tc ǥ},{tc ƹ,ƒ ƹ}},г(ù,n3 ƹ);t.Ŗ(χ,n3 ǥ);t.Ŗ(o0,n3 Ѐ);ğ o9 ɛ={0,ӓ,2,ӓ}У{Ù ĝ i3=ļ[Ӷ]Ƥ w(o9 Ƭ)Ә{Ҽ Ō(n3 Ƭ ȝ);Š(c Ƭ ȝ,i<e1?ů(ƫ(x,ń(ŭ(Ō(i3 ȝ),w),kP))):x);}Ù ŉ ȣ=c Ƭ[e1]+ŉ(o9 Ƭ)*i3[e1];c Ƭ[e1]=Ö(ȣ Ұ);c Ƭ[k9]=c Ƭ Ŭ;c Ƭ Ŭ Ē ż(c Ƭ ʊ(k9+1),c Ƭ ƃ,0);}}í third_last_level_transposed(Ƶ,Ù Levels&ţ,Ù ĝ(&p)ɛ,Ĝ Ǡ,ɐ){Ģ m=ţ.m,k9=m/4,e1=m/8;Ĝ tp ɛ,r Ҹ(ǈ 4 ų){tp ˉ Ǡ Ė(j ӌ(k9),k9)Ȇ(p Ƃ,0,tp Ƃ);}Ć(Õ c=1;c<8 ė c){r[c]=Ǡ Ė((c+3)ӌ(k9),k9);ɝ Ǜ ɛ;ȡ Ɠ(ǈ 4&&2*j<c ų)Ǜ[þ++]={tp Ƃ,ţ.column_transform(c-2*j)}ĺ(þ<4)t.Ŗ(Ø span(Ǜ,þ),r[c],ƨ)ї Ŗ<4>(ţ.Ķ,Ǜ,r[c],ƨ);}Ć(ɮ=e1 ϼ k9 ƞ){ɍ ɚ;x Г Ō(p Ŭ ȝ);
#pragma GCC unroll 7
Ć(Õ c=1;c<8 ė c)x[c]=c%2?Ō(r[c]ȝ):ů(ƫ(Ō(r[c]ȝ),Ō(p[c/2]ȝ)));ʈ(x);
#pragma GCC unroll 8
Ć(ǈ 8 ų)Š(Ԁ 2*m ү*(i-e1+j),x Ƃ);}}í Ӟ(Ƶ,ĝ g,Levels&ţ){Ģ m=ţ.m ǫ v=ȁ(3)ʴ,q0=ȁ(0)ϟ;ż(q0 ü,q0 ƃ,0);Ȫ(g ʊ(ĥ<Õ>(ȸ,1)),g ʊ(ĥ(ȸ,m)),q0 ʷ 1)Ĩ ȋ&Ķ=ţ.Ķ;forward_with(q0,m,Ķ,LevelBottom{&Ķ,v â});Ɇ(v,Ķ,Ɉ{ѝ,v â},Ж(ţ.lg),ȉ);first_level(t,g,ţ)ø(ý s=2;s+3<ţ.lg ė s){Ģ ď=2*(m>>s),pad=Õ(ţ.lg-s-3)ǫ next=ȁ(s+1)ϟ;forward_pruned(ȁ(s),((Ǽ s)+1)Ԍ,pad,Ķ,LevelBottom{&Ķ,next â});inverse_pruned(next,pad-1,Ķ,Ɉ{ѝ,next â},Ж(ţ.lg+1));next_level(ȁ(s+1),ď/2,Õ(2)<<s);}Ĝ c ɛ У c ѓ ȁ(ţ.lg-2)Ė((k-1)ӌ(m/2),m/2);third_last_level(t,ţ,c)ǫ q=ȁ(ţ.lg-1)ʴ;ż(q ü,q ƃ,0)ø(Ů m/4 ƞ)Š(q ȝ,Ӣ(ϕ(Ō(c ǥ ȝ))));q[m/4]=Ґ(c ǥ[m/4])ǫ n3=ȁ(ţ.lg-1)Ė(à(m),m/2)У t.ı(c Ƭ);Ɗ(c ƹ,c ƹ,n3)ø(Ů m/2 ƞ)Š(q ȝ,ń(ƫ(Ō(q ȝ),Ō(n3 ȝ)),kP));ğ top=q[m/2]+q Ŭ;q[m/2]=top>=kP?top-kP:top;q Ŭ Ē t.ı(q);}}ò ý compose_log(ƿ){if(n<=ä Ʉ)Ú ʵ Ū ĩ(ä compose_length(n))+1;}Ί compose_scratch(ƿ){if(n<=ä Ʉ)Ú 0 Ū ä Levels::ĸ(ä compose_length(n));}ԃ outer_word(Ö x){Ú x;}ԃ outer_word(ǣ&x)Ш Ő Ф>Ĉ compose_imported(Ƶ,ԁ Ф>f,ĝ g,Ĝ h,Є,Ù Č*m9=Ƿ){Ə(ģ Ф,Ö>||ģ Ф,Ī>)Ȣ n=h Û ĺ(f ű||ȸ<=1){ż(h ü,h ƃ,0u)
ĺ(n&&!f ű)h Г outer_word(f Ŭ)а}if(n<=ä Ʉ){Ĺ(ģ Ф,Ö>)Ú ä compose_direct(f,g,h);Ö a[ä Ʉ]Ȣ æ Ч(Ǳ,n)ø(Ů æ Į)ө outer_word(f Ĕ)Ū ä compose_direct(ĝ(a,æ),g,h);}ä Levels ţ(ä compose_length(n),ĸ,m9);ä Ӟ(t,g,ţ)Ȣ m=ţ.m Ȋ lg=ţ.lg ǫ ȣ=ȁ(lg-1)ǫ p=ȣ Ė(à(m),m),pq=ȣ Ė(2 ӌ(m),m);{Ģ æ Ч(Ǳ,m);ż(p ü,p ƃ-ř(æ),0);ǚ to=p Ʒ(m-æ)Ĩ ä ĭ ˁ=ʞ;ʍ Ɠ(;j+8<=æ Ӕ){Ə(ƛ(Ф)==ƛ(Ö));ä ɍ;Ǵ(&x,f Ʒ æ-8-j,ƛ(x))Њ(ģ Ф,Ī>)x=ä ń(x,ä kP);Ψ(õ ä ҳ>(to+j),Ľ(x,ˁ));}Ć(;j<æ ų)to ˉ outer_word(f[æ-1-j]);}t.Ŷ(p,0,pq,ȣ ʴ,ƨ);{Ģ h=m/2,k9=m/4 ǫ p0=p Ė(h),p1=pq Ė(h),t0=ȣ Ȓ(h),t1=ȣ Ė(à(h),h)ǫ c=ȁ(lg-2)ǌ column=ѯ(ȕ){Ú c Ė((k-1)ӌ(h),h);}ǫ r1=p Ȓ(h),r2=pq Ȓ(h),r3=ȣ Ė(3 ӌ(m),h)Ȇ(p0,0,t0)Ȇ(p1,0,t1);Ɗ(t0,column(1),r1,ƨ);Ɗ(t0,column(2),r2,ƨ)Ĩ ɝ Ǜ ǥ={{t1,column(1)},{t0,column(3)}};t.Ŗ(Ǜ,r3,ƨ)ø(ɮ=k9 ϼ h ƞ)ä Š(r2 ȝ,ä ń(ä ƫ(ä Ō(p1 ȝ),ä Ō(r2 ȝ)),ϴ))Ĩ ĝ İ ɛ={p0 Ė(k9),r1 Ė(k9),r2 Ė(k9),r3 Ė(k9)};ä third_last_level_transposed(t,ţ,İ,ȁ(lg-2),ȁ(lg-3));}Ĝ in=ȁ(lg-3);ğ ŧ=ä Ȍ ƹ[lg+2]ø(ý s=lg-4;s>=2;--s){Ù Ĝ x=in Ė(2*m),į=ȁ(s==lg-4?lg-2:s+1)Ȣ pad=Õ(lg-s-3);ä forward_pruned
(x,2*m,pad-1,ţ.Ķ,ä ǂ{ţ.Ķ.ķ});ä inverse_pruned(į,pad,ţ.Ķ,ä ͱ{&ţ.Ķ,x â,ȁ(s)â},ŧ,ƨ);in=į;}ä first_level_transposed(t,ţ,in,ȁ(2));in=ȁ(2)ǫ į=ȁ(1)ǫ p0=in Ė(2*m,m),r=į ϟ Ȇ(p0 Ȓ(m/2),0,p0);ä Ɇ(r,ţ.Ķ,ä ͱ{&ţ.Ķ,p0 â,ȁ(0)â},ä Ȍ ƹ[ţ.lg+1],Ʃ);ȕ Ɠ(;k ү<=n;k+=16){ä ĭ a ǥ;ä spread(ä Ō(in Ʒ 3*m+k/2),a)ø(Ӄ<2 ė t)Ψ(õ ä ҳ>(h Ʒ k ϛ),ä ɸ(a Ѱ,ä Ō(r Ʒ k ϛ)));}Ć(;k<n Ȟ){ğ a=k%2?0:in[3*m+k/2];h ѓ a>=r Ƭ?a-r Ƭ:a+ϴ-r Ƭ;}}í compose(Ƶ,О,ĝ g,Ĝ h,Є){compose_imported(t,f,g,h,ĸ);}}ā ȗ{Ϥ{Ί Ҏ(ƿ){Ú ɬ(ʘ(n,512));}í projection_direct(ĝ g,ҽ){Ģ n=ư;Ö ǹ[Ʉ]={1},next[Ʉ]ø(ѿ n Ȟ){a ѓ ǹ[Ԕ]ø(Ů n Į){ŉ c Ɠ(ʍ=1;j<=i&&j<ȸ ų)c+=ŉ(g Ƃ)*ǹ[i-j]Ұ;next ƈ Ö(c Ұ);}ĵ(next,n,ǹ);}}Ǹ Ȗ(ɍ,ĭ y){ġ t=_mm256_castps_si256(_mm256_shuffle_ps(р(x),р(y),0xDD))Ū ҭ(t,0xD8);}έ kAdd,Ũ Op>Ĉ ʺ(ĝ b,І,п,ř Ų,ɐ,Ù Op&op){ʦ scalar=[&op](Ö x){Ú Ö(Ҙ(ɦ(op(ç(ý(x))))));}ǌ put=[&į](ʍ,Ö x){Ĺ(kAdd)į ˉ į Ƃ+x>=kP?į Ƃ+x-kP:į Ƃ+x ї į ˉ x;}ǌ at=[Ų](ʍ){Ú ř(2*j)+Ų;}ǌ ù=ѯ(ɮ){Ù ř d=ř(i)-Ų Ū ĥ(ǲ,d<=0?Õ(0):Õ(d+1)/2);}Ȣ begin=ù(lo),end=Ŋ(begin,ù(hi))Њ(!kAdd){ż(į ü,į ʊ(begin),0);ż(į ʊ(end),į ƃ
,0);}ʍ=begin ø(;j+8<=end&&Õ(at(j))ү<=ζ Ӕ){Ɣ ʝ=b Ʒ at(j);ɍ=op(even_lanes(Ž(ʝ),Ž(ʝ+8)))Њ(kAdd)x=ń(ƫ(Ž(Ԁ j),x),kP);ū(Ԁ j,x);}Ć(;j<end ų)put(j,scalar(b[Õ(at(j))]));}í add_into(ĝ x,ĝ y,ɐ){Ć(Ů ǲ ƞ)Š(į ȝ,ń(ƫ(Ō(x ȝ),Ō(y ȝ)),kP));}Ɯ ProjectionBottom{Ǐ Ƈ,Ƚ=Ś Ĩ ȋ*Ķ;Ɣ p;Ņ v,*w;Ț{ǂ{Ķ->ķ}(a,þ,ù)ø(Ů þ Ƽ 4)Ǜ(a+32*i,2*(Ў i));}ѱ Ǜ(Ɣ q,ȕ)Ù{ы va ɛ,vb ɛ,wa ɛ,wb ɛ;ȟ ə(q,p ү*k,ş.s,va,wa);ȟ ԍ(q+8,p ү*k+8,ş.s,vb,wb);ҩ(va,vb,ş.ć,v Ƨ k);ҩ(wa,wb,ş.ć,w Ƨ k);}ʙ Ŵ ȟ(Ɣ q,Ɣ p,Ù я&s,ĭ(&v)ɛ,ĭ(&w)ɛ){ĭ c ɚ,a ɚ,tc ɚ;ΐ(q,c),ΐ(p,a)Φ(c,s,2,tc);ӡ(c,tc,v);Ӡ<1>(a,c,tc,w);}};í inverse_lower(Һ,Ϩ,Ù Τ&w,Ȩ){Ć(Ů 8*h ƞ){ġ f0=Ō(a+i),f1=Ō(a+i Ƨ h),f2=Ō(a+i ү*h),f3=Ō(a+i+24*h);Š(є i,ʗ(ƫ(ʗ(ƫ(f0,f1)),ʗ(ƫ(f2,f3)))));Š(є i Ƨ h,Ӑ);}}í forward_lower(Ɣ in,Ϩ,Ù Τ&w,Ң){Ć(Ů 8*h ƞ){ġ u ӏ(Ō(in+i)),b=Ō(in+i Ƨ h),yb Ҝ(b,w.y),zb Ҝ(b,w.z);Š(a+i,ƫ(u,yb)),Š(a+i Ƨ h,ǘ(u,yb));Š(a+i ү*h,ƫ(u,zb)),Š(a+i+24*h,ǘ(u,zb));}}Ő ƭ>Ũ Doubling{Ⱥ Doubling(ҽ,Õ ŀ,ί,Ϧ):a_(ʰ),half_(ư/2),ʪ(ŀ),b_(Õ(ĩ(ŀ))-1),aligned_(ĩ(ŀ)%2 ε),ʻ(Ķ),bottom_(b9),ϯ(Ķ.ķ,ѝ,Ɉ{ѝ,ʰ}),forward_(Ķ.ķ,ѝ,b9){}
Ĉ run(Ö p8,Ö fix)Ù{ǚ copy=a_+half_ Ȣ nv=half_/8,Ѫ=ʘ(ʪ,16)ø(Õ c Ē c<nv;c+=Ѫ){Ć(ʍ=c/ʪ;j<(c+Ѫ)/ʪ ų){inverse_x(a_ Ƨ ʪ*j,j,copy Ƨ ʪ*j);forward_x(copy Ƨ ʪ*j,j,a_ Ƨ ʪ*j);}if(ʪ<16)bottom_(a_ Ƨ c,Ѫ/4,c/4);}inverse_y(copy,p8)ĺ(fix){Ć(Ů 4*ʪ ƞ)Š(copy+i,Ӣ(Ō(copy+i)));copy Г fix>=Ͼ copy Ŭ?fix-(Ͼ copy Ŭ):fix+copy Ŭ;}ɇ();}Ⱦ Ĉ inverse_x(Ң,ʍ,Ȩ)Ù{if(ʪ==4){ϯ.ϖ(a,4,j)ԕ(a,16,į);}м(aligned_){Ģ h=ʪ/4 Ȳ ϯ.ϖ(ϋ t*h,h,4*j+t);inverse_lower(a,h,Τ(ʻ.ô,j),į);}ŝ{Ģ h=ʪ/2;ϯ.ϖ(a,h,2*j);ϯ.ϖ(ϋ h,h,2*j+1)ø(Ů 8*h ƞ)Š(є i,ʗ(ƫ(Ō(a+i),Ō(a+i Ƨ h))));}}Ĉ forward_x(Ɣ in,ʍ,Ң)Ù{if(ʪ==4){ĵ(in,16,a);ʇ(a ү,16,0);}м(aligned_){Ģ h=ʪ/4;forward_lower(in,h,Τ(ʻ.ķ,j),a)ĺ(h==4)Ú bottom_(a,4,4*j)Ȳ forward_.ϖ(ϋ t*h,h,4*j+t);}ŝ{Ģ h=ʪ/2 ø(Ů 8*h ƞ){Ҽ Ō(in+i);Š(a+i,x),Š(a+i Ƨ h,x);}if(h==4)Ú;forward_.ϖ(a,h,2*j);forward_.ϖ(ϋ h,h,2*j+1);}}Ĉ above(Ң,Ԅ,ȕ)Ù{if(nv==ʪ)Ú ĺ(nv==2*ʪ){Œ x=ʮ(ʻ.ô,k)ø(Ů 4*ʪ ƞ){ġ u=Ō(a+i),v=Ō(a+i Ƨ ʪ);Š(a+i,ʗ(ƫ(u,v))),Š(a+i Ƨ ʪ,ŭ(ǘ(u,v),x));}Ú;}ƽ 4 Ȳ above(ϋ t*h,h,4*k+t);ǔ(a,h,b_,ʻ.ô,k);}Ĉ inverse_y(Ң,Ö ŧ)Ù{Ģ nv=half_/8,ҕ=Ǽ b_ Ĩ Ǝ
 zero=b_<=5;ҳ Ù v=ț(a)Ƥ s(ŧ)ǌ е Ѓ ι z0=đ ǡ{ƽ 4 Ȳ above(ϋ t*h,h,t)Ƥ z(ʻ.ô ƹ,ʻ.ô[9])Ǖ{ǳ{ġ x0=v Ƃ,x1=Ӊ,x2=v ɋ,x3=v Ɋ ι ab ӏ(ƫ(x0,x1)),cd ӏ(ƫ(x2,x3))ι amb ӏ(ǘ(x0,x1)),cmd Ҝ(ǘ(x2,x3),z);v ˉ е(ϑ),Ӊ=е(ϐ);v ɋ=е(Ќ),v Ɋ=е(Ϗ);}if(zero)ȳ v ɋ=v Ɋ=z0;}}ŝ{ƽ 2;above(a,h,0);above(ϋ h,h,1)Ǖ{ǳ{ġ x0=v Ƃ,x1=Ӊ;v ˉ е(ƫ(x0,x1)),Ӊ=е(ǘ(x0,x1));}if(zero)ȳ z0;}}}Ĉ ɇ()Ù{Ģ nv=half_/4,ҕ=Ǽ b_ Ĩ Pruned<ƭ>Ҩ(ʻ,bottom_,b_)ǡ{ƽ 4;ҳ Ù v=ț(a_)Ƥ z(ʻ.ķ ƹ,ʻ.ķ[9])Ǖ ǳ{Ҽ v[2*h+j],y Ҝ(v[3*h+j],z);v[2*h+j]=ƫ(x,y),v[3*h+j]=ǘ(x,y);}Ҩ.ı(a_ ү*h,h,2);Ҩ.ı(a_+24*h,h,3);}ŝ{Ҩ.ı(a_+4*nv,nv/2,1);}}Ņ a_;Õ half_,ʪ,b_;Ǝ aligned_ Ĩ ȋ&ʻ;ƭ bottom_;ǆ<Ɉ>ϯ;ǆ<ƭ>forward_;};í ӄ(Ƶ,ί,ĝ g,ƿ,Ĝ q,Ĝ p){Ģ m=Ѹ/4,h=m/2,k9=m/4,e=m-n,æ Ч(n,ȸ)ǌ ȭ=ѯ(ɍ)Ý{Ú x;}ǌ j0=ѯ(ɍ)Ý{Ú Ӣ(x);}ǌ e0=ѯ(ɍ)Ý{Ú ϕ(x);}ǌ Ґ=ѯ(ɍ)Ý{Ú Ӣ(ϕ(x));}ǌ ν=[h,k9](ҽ,ȕ){Ú a Ė(k*h,k9);}ĺ(æ>0){q Ŭ Ē Ȫ(g ʷ 1,g ʊ(æ),q ʷ 1);}forward_with(q ϟ,æ,Ķ,LevelBottom{&Ķ,q â});Ɇ(q ʴ,Ķ,Ɉ{ѝ,q â},Ж(ĩ(m)),ȉ)ǫ q2=p Ė(7*h,h),q1=q Ė(7*h,h),p1=q Ė(4*h,h)ԕ(q ü,h,q2 ü);ʺ ə(g,1,æ,0,q1,Ґ)ĺ(e%2)ʺ ə(g,1,æ,1-ř(e),p1,j0)ї ʺ ə(g,1,æ,1-ř(e
),p1,ȭ)ǫ tq2=q ʴ,tq1=q ч,tp1=p ʴ Ȇ(tq2 Ȓ(h),0,tq2)Ȇ(q1,0,tq1)Ȇ(p1,0,tp1);ǚ g1=p Ʒ 4*h;ǚ g2=p Ʒ 5*h;ǚ e12=p Ʒ 6*h;ǚ o1=q Ʒ 5*h;ǚ o2=q Ʒ 6*h Ĩ FirstLevelProducts ԍ Ɩ{&Ķ,tq1 â,tq2 â,tp1 â,{g1,g2,e12,o1,o2}}ø(ѿ m/16;k+=8)Ɩ.Ǜ(k);ğ ŧ=Ж(ĩ(h))ø(Ȩ:Ɩ.į)Ɇ(Ĝ(į,h),Ķ,Ɉ{ѝ,į},ŧ,Ʃ);ż(p ü,p ʊ(k9),0)ĺ(e%2&&(e-1)/2%2)p[(e-3)/4]=1;ʺ ə(p1,0,h,1,ν(p,1),ȭ)ԕ(o1,k9,p ʊ(2*h))ԕ(o2,k9,p ʊ(3*h))ĺ(e%2){Ģ d=(e-1)/2 Ĩ ř Ų=1-ř(d)Ĩ ĝ b1=q1,b2=q2 ĺ(d%2){ʺ ԍ(b1,0,h,Ų,ν(p,1),ȭ);ʺ ԍ(b2,0,h,Ų,ν(p,2),ȭ);}ŝ{ʺ ԍ(b1,0,h,Ų,ν(p,1),j0);ʺ ԍ(b2,0,h,Ų,ν(p,2),j0);}}ż(q ü,q ʊ(k9),0);q Г 1;ʺ ə(q1,0,h,0,ν(q,1),e0);ʺ ə(q2,0,h,0,ν(q,2),e0);add_into(ν(q,2),ĝ(g1,k9),ν(q,2))Ә Š(q Ʒ 3*h+i,ϕ(Ō(e12+i)))ԕ(g2,k9,q ʊ(4*h));}í projection_weighted_first(Ƶ,ί,ĝ g,ĝ ş,Ĝ q,Ĝ p,Ĝ Ǡ){Ģ m=Ѹ/4,h=m/2,k9=m/4,n=ş Û;ӄ(t,Ķ,g,n,q,p)ѵ x=Ǡ ϟ,y=Ǡ Ė(2*m,2*m),z=Ǡ Ė(4*m,2*m);ż(x ü,x ƃ,0u);ż(y ü,y ƃ,0u)ø(Ů n Į)x[m-1-i]=ş Ĕ ø(ɮ=1 ϼ ĥ(ȸ,n)Į)y ƈ i%2&&g Ĕ?Ͼ g Ĕ:g Ĕ Ȇ(y);t.Ŷ(x ʴ,0,z,y,Ʃ)ѵ p0=Ǡ Ȓ(h),p1=Ǡ Ė(h,h)ø(Ů h Į){ʦ Ĵ=z Ʊ;p1 ƈ Ĵ?Ͼ Ĵ:0;p0 ƈ 2*i+1<m&&m-2-2*i<n?ş[m-2-2*i]:0;}
ł tp0=Ǡ ч,tp1=Ǡ Ė(2*m,m)ѵ tq1=Ǡ Ė(3*m,m),tq2=Ǡ Ė(4*m,m),Ŏ=Ǡ Ė(5*m,m)ø(Ů h Į){ʦ a=q[7*h+i],b=p[7*h+i];tq1 ƈ i%2&&a?Ͼ a:a;tq2 ƈ i%2&&b?Ͼ b:b;}t.ı(p0,0,tp0)Ȇ(p1,0,tp1)Ȇ(tq1 Ȓ(h),0,tq1)Ȇ(tq2 Ȓ(h),0,tq2)ø(Ů k9 Į)p ƈ p0 Ʊ;Ɗ(tp0,tq1,Ŏ,Ʃ)ø(Ů k9 Į){ʦ Ĵ=Ŏ Ʊ+p1 Ʊ;p[h+i]=Ĵ>=kP?Ĵ-kP:Ĵ;}Ù ɝ Ǜ ǥ={{tp0,tq2},{tp1,tq1}};t.Ŗ(Ǜ,Ŏ,Ʃ)ø(Ů k9 Į)p[2*h+i]=Ŏ Ʊ;Ɗ(tp1,tq2,Ŏ,Ʃ)ø(Ů k9 Į)p[3*h+i]=Ŏ Ʊ;}Ί ԇ(Ό){Ģ e1=m/8 Ū Ŋ(7*Ԉ Ư(e1+1)Ƨ Ԉ Ư(e1),Ԉ Ư(m)ʳ Ԉ Ư(m/2));}í projection_last_levels(Ƶ,ί,Ĝ q,Ĝ p,Ĝ w,ҽ){Ģ m=Ѹ/4,e1=m/8,k9=m/4,h=m/2;Ĝ c ɚ,pc ɚ;{Ԉ carve(w)ø(ʶ 8 Ȟ)c ѓ carve ņ(e1+1)ø(ѿ 8 Ȟ)pc ѓ carve ņ(e1);}Ԑ{ɍ ɚ;
#pragma GCC unroll 8
ԏ
#pragma GCC unroll 7
Ć(ʶ 8 Ȟ)Š(c Ƭ Ʒ r,k%2?Ӣ(x Ƭ):x Ƭ);}Ć(ʶ 8 Ȟ){ğ x=q[16*e1+k];c Ƭ[e1]=k%2&&x?Ͼ x:x;}Ԑ{ɍ ɚ;
#pragma GCC unroll 8
Ć(Ů 8 Į)x ƈ Ō(p Ʒ 16*(r+i));ʈ(x);
#pragma GCC unroll 8
Ć(ѿ 8 Ȟ)Š(pc Ƭ Ʒ r,x Ƭ);}Ԉ carve_q(q),carve_p(p);Ĝ tc ɚ,tp[7],ƒ Ѐ,cn ɛ,pn ɛ ø(ʶ 8 Ȟ){tc ѓ carve_q ņ(k9)Ȇ(c Ƭ,0,tc Ƭ);}Ć(ѿ 7 Ȟ){tp ѓ carve_p ņ(k9)Ȇ(pc Ƭ,0,tp Ƭ);}Ć(ł&s:ƒ)s=carve_q ņ(k9);ʏ(tc[5],2,ƒ Ŭ),ʏ(tc Ѐ,ӓ,ƒ ƹ),ʏ(tc ɛ,ӓ,ƒ ǥ)У cn ѓ carve_q ņ(k9+1)ø(ł&s:pn)s=carve_p ņ(k9);{Ù ɝ ù ƹ={{tc ƹ,tc ƹ}},χ ǥ={{tc ǥ,tc ǥ},{tc ƹ,ƒ ƹ}},г(ù,cn ƹ Ȓ(k9));t.Ŗ(χ,cn ǥ Ȓ(k9));t.Ŗ(o0,cn Ѐ Ȓ(k9));}ğ o9 ɛ={0,ӓ,2,ӓ}У{Ù ĝ i3=c[Ӷ]Ƥ f(o9 Ƭ)ø(Ů e1 ƞ)Š(cn Ƭ ȝ,ů(ƫ(Ō(cn Ƭ ȝ),ń(ŭ(Ō(i3 ȝ),f),kP))))Ĩ ŉ ȣ=cn Ƭ[e1]+ŉ(o9 Ƭ)*i3[e1];cn Ƭ[e1]=Ö(ȣ Ұ);cn Ƭ[k9]=cn Ƭ Ŭ;cn Ƭ Ŭ Ē}{ɝ Ǜ[7]ø(ǈ 4 ų){Ć(Õ b=1;b<=2*j+1 ė b)Ǜ[b-1]={tp[2*j+1-b],tc[b]}Ĩ ԁ ɝ>ʂ(Ǜ,2*j+1)ĺ(j<2)t.Ŗ(ʂ,pn Ƃ);м(j==2)Ŗ<5>(Ķ,Ǜ,pn Ƃ,ȉ)ї Ŗ<7>(Ķ,Ǜ,pn Ƃ,ȉ);add_into(pn Ƃ Ȓ(e1),pc[2*j+1],pn Ƃ Ȓ(e1));}}Ԉ next_q(q),next_p(p),next_w(w);Ĝ tcn ɛ,tpn Ѐ У{tcn ѓ next_q ņ(h)Ȇ(cn Ƭ,0,tcn Ƭ);}Ć(ǈ 3 ų){tpn ˉ next_p ņ(h)Ȇ(pn Ƃ,0,tpn Ƃ);}Ù Ĝ n3=next_q ņ(h),j2=next_w ņ(m);Ɗ(tcn ƹ,tcn ƹ,n3)ø(Ů h ƞ){Ҽ i<k9?Ӣ(ϕ(Ō(cn ǥ ȝ))):đ;Š(j2 ȝ,ń(ƫ(x,Ō(n3 ȝ)),kP));}{ğ x=j2[k9]+Ґ(cn ǥ[k9]);j2[k9]=x>=kP?
x-kP:x;j2[h]=j2 Ŭ;j2 Ŭ Ē ż(j2 ʊ(h+1),j2 ƃ,0);}Ù Ĝ r1=next_w ņ(h),r3=next_w ņ(h);Ɗ(tpn Ŭ,tcn ƹ,r1);add_into(r1 Ȓ(k9),pn ƹ,r1 Ȓ(k9))Ĩ ɝ Ǜ Ѐ={{tpn ǥ,tcn ƹ},{tpn ƹ,tcn ǥ},{tpn Ŭ,tcn Ѐ}};t.Ŗ(Ǜ,r3);add_into(r3 Ȓ(k9),pn Ѐ,r3 Ȓ(k9))Ȇ(j2)ǫ Ŏ=next_q ņ(m);t.Ŷ(r1,0,Ŏ,j2);ȕ Ɠ(;k+8<=ĥ(ư,h);k+=8)ū(a Ʒ k,ń(ƫ(Ō(r3 Ʒ k),Ō(Ŏ Ʒ k)),kP))ø(;k<ư Ȟ){ğ x=(k<h?r3 Ƭ:0)+Ŏ Ƭ;a ѓ x>=kP?x-kP:x;}}}ò ý projection_log(ƿ){if(n<=ä Ʉ)Ú ʵ Ū ĩ(ä Ҏ(n));}Ί ʹ(ƿ){Ę ä Ԉ ĺ(n<=ä Ʉ)Ú 0 Ȣ m=ä Ҏ(n)Ū à(ä ȋ Ư(m))ʳ Ԉ Ư(4*m)+Ԉ Ư(ä ԇ(m));}í ɯ(Ƶ,ĝ g,ҽ,Є){ѻ n=ư ĺ(n<=Ʉ)Ú projection_direct(g,a)ȿ,p=carve ņ(4*m),w=carve ņ(ԇ(m));ӄ(t,Ķ,g,n,q,p)ц}í weighted_projection(Ƶ,ĝ g,ĝ ş,ҽ,Є){ѻ n=ư;Ƣ(n>Ʉ&&ş Û==n)ȿ,p=carve ņ(4*m),w=carve ņ(ԇ(m));projection_weighted_first(t,Ķ,g,ş,q,p,ĸ.ȣ(6*m))ц}}ā ȗ{ò ī ϭ(ǣ&x){Ù ī Ĵ=x.v Ĩ ī l5=(ú о>(ī(Ĵ*998244351u))*998244353u+Ĵ)>>32 Ū ĥ(l5,l5-998244353u);}í append_word(л,ī Ĵ){į η();į.back().v=Ĵ;}í native_compose(ǣ*f,Õ nf,ǣ*g,Õ ng,л,ƿ){nf Ч(nf,n);ng Ч(ng,n)Ȋ lg=compose_log(n)+(n>ä Ʉ)й(ɠ(nf)Я(ng)Я(n)Я(compose_scratch(n)))Ǭ a ф
(nf),b ф(ng),c к ø(Ů nf Į)ө ĥ(f Ĕ.v,f Ĕ.v-Ƀ)ø(Ů ng Į)b ƈ ϭ(g Ĕ);compose_imported(t,ĝ(a),b,c,Ĭ(compose_scratch(n)),&t);ž(į,c â,c Û);}í native_weighted_projection(ǣ*g,Õ ng,ǣ*ş,л,ƿ,ȡ){ng Ч(ng,n)Ȋ lg=projection_log(n)+1 Ȣ Ƅ=ʹ(n)Я(6*ä Ҏ(n))й(ɠ(ng)ʳ à(n)Я(Ƅ))Ǭ a ф(ng),b к,w к ø(Ů ng Į)ө i?ϭ(g Ĕ):0 ø(Ů n Į)w ƈ ĥ(ş Ĕ.v,ş Ĕ.v-Ƀ);weighted_projection(t,a,w,b,Ĭ(Ƅ));ž(į,b â,þ);}í native_projection(ǣ*g,Õ ng,л,ƿ){ng Ч(ng,n)Ȋ lg=projection_log(n)й(ɠ(ng)Я(n)Я(ʹ(n)))Ǭ a ф(ng),b к ø(Ů ng Į)ө ϭ(g Ĕ);ɯ(t,a,b,Ĭ(ʹ(n)));append_fps_words ə(į,b â,ζ);}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ compose(Ù Ȼ&j7,Ù Ȼ&ʎ,Ѡ){Ѽ;ƙ Н ó(j7 Û);Ƣ(ǀ>=0&&(ʎ ű||ʎ Ŭ Ү))ĺ(ǀ ε)Ú{};
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(ǀ>=64&&ǀ<=(1<<21)&&j7 Û>8&&ʎ Û>2&&j7[ĥ(j7 Û,Õ(ǀ))-1]!=ȓ&&ʎ ƹ!=ȓ&&ʎ ǥ!=ȓ ȏ){F ă;Ü ȗ::native_compose(j7 â,j7 Û,ʎ â,ʎ Û,ă,ǀ)ȧ}}
#endif
F f=j7.pre(ǀ),g=ʎ.pre(ǀ);ɞ(f);ɞ(g)ĺ(Ǳ<=1||ȸ<=1){Ѭ ĺ(!f ű)ă Г f Ŭ ȧ}ý ù=1 ǟ(g[ù]Ү)++ù ĺ(ó(Ǳ)>(ǀ-1)/Ԏ){f Ř((ǀ-1)/Ԏ);ɞ(f)ĺ(Ǳ<=1){Ѭ ĺ(!f ű)ă Г f Ŭ ȧ}}if(Ԏ==Џ){Ѭ;T ǹ=1 ø(ö ó(Ǳ)&&(Ǉ)i*ù<ǀ Į)ă[i*ù]=f Ĕ*ǹ,ǹ*=g[ù]ȧ}if(ù>1){Ӂ=ù ø(Ż Ԏ ϼ Џ&&ď>1 Į)if(g Ĕ!=ȓ)ď=Ø gcd(ď,i)ĺ(ď>1){F ʸ((ȸ-1)/ď+1)ø(κ ó(ʸ Û)Į)ʸ ƈ g[i Ԍ];ʸ=compose(f,ʸ,(ǀ-1)/ď+1);Ѭ ø(ö ó(ʸ Û)Į)ă[i Ԍ]=ʸ Ĕ ȧ}}if(ǀ<=32||Ǳ<=8){F ă ø(Ż(ó(Ǳ))-1;i и i){ă=F::Ź(ă,g).pre(ǀ);ă+=f Ĕ;}ă ӗ ȧ}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(Ű ě&&ǀ<=(1<<21)){F ă;Ü ȗ::native_compose(f â,Ǳ,g â,ȸ,ă,ǀ)ȧ}}
#endif
ϓ;Ƣ(æ<=(1<<22));Ü NTT plan(æ);Ħ ϒ>p(Ё),q(æ)ø(ö ó(Ǳ)Į){Ġ p ƈ f Ĕ.v ї p ƈ Ƹ in(f Ĕ Ȅ);}Ć(ö Џ Į){Ġ q ƈ g Ĕ.v ї q ƈ Ƹ in(g Ĕ Ȅ);}ϒ a=Ƹ one,b=Ƹ one;Ƹ compose(p â,q â,1,æ,a,b);Ѭ ø(ö ǀ Į){Ġ ă ƈ T ɪ(p Ĕ)ї ă ƈ T θ(Ƹ į(p Ĕ));}Ҥ}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā Ü ȗ{í native_reversion(ǣ*in,Õ ʭ,л,ƿ){ʭ Ч(ʭ,n)Ȋ lg=Ŋ(projection_log(n),ʃ power_log(Ԕ))Ȣ ɼ=Ŋ(ʹ(n),à(Ԕ)+ʃ ҫ(Ԕ))й(ɠ(ʭ)ʳ à(n)Я(ɼ))Ǭ f ф(ʭ),g к,a к,ĸ ф(ɼ)ø(Ů ʭ Į)f ƈ ϭ(in Ĕ);ɯ(t,f,a,ĸ Ȓ(ʹ(n)))ѵ h=ĸ Ȓ(Ԕ);ʃ ɗ(a Ė(1),1,h);Ø ˁ(h ü,h ƃ)ǌ e=ϴ-ã ǹ(Ö(Ԕ),ϴ-2)ǌ c=in ƹ ˆ;ʃ ǹ(t,h,e,ĥ(c.v,c.v-Ƀ),g Ė(1),ӿ(à(Ԕ),ʃ ҫ(Ԕ)));ƚ(į,n);į η(0);ž(į,g Ʒ 1,ȸ-1);}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ compositional_inverse(Ù Ȼ&f,Ѡ){Ѽ;ƙ Н ó(Ǳ);Ƣ(ǀ>=0&&(f ű||f Ŭ Ү))ĺ(ǀ ε)Ú{}ĺ(ǀ у)Ú F(1);Ƣ(Ǳ>=2&&f ƹ!=ȓ)ӹ(compositional_inverse Ӹ(ҹ(f),ǀ))ї{T w=f ƹ ˆ;ή=ʡ(ǀ,Ǳ);ý ù=2 ǟ(ù<î&&f[ù]Ү)++ù ĺ(ù==î){Ѭ;ă ƹ=w ȧ}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(ǀ>=64&&ǀ<=(1<<21)ȏ){F ă;Ü ȗ::native_reversion(f â,Ǳ,ă,ǀ)ȧ}
#endif
ϓ;Ƣ(æ<=(1<<21))Ĩ ǧ=ǀ-1 ѵ&ђ=ǅ inverses(æ);F h(n);{ȷ,*q=p+4*æ ԕ(Ȯ(f),î,q);ʇ(q+î,Ё-î,0u);t.geometric(q,î,T(-1).v,w.v)ѵ Ÿ=t.reversion_projection(p,q,æ);t.Ŏ(p+æ-n,p+æ-n,Ȯ(ђ)+æ-n,n)ø(Е)h ƈ T ɪ(p[æ-1-i]);ǅ Ζ(h,T(æ-1)*T ɪ(Ÿ));}h=h.log(n);ǅ Ζ(h,-ђ[æ-1]);h=h.exp(n);ǅ Ζ(h,w);Ѭ;Ȫ(h ü,h ƃ,ă ʷ 1)ȧ}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
Ӏ{ˬ Ɯ Ώ{Ѽ ͼ ƻ=џ ͼ O=Ǆ ͼ Word=ӆ ǅ Word;ċ ƻ evaluate(ҥ k5,Ù Ħ F>&ǉ,Ħ ƻ>&ƕ,ý æ,ý ŀ,F ˀ,Ǝ d2=Ś){ƻ ă(Ј);
#if defined(__x86_64__) || defined(__i386__)
Ġ{if(ŀ==8 ȏ){Ƞ o4=æ*ŀ ѵ&plan=ǅ Ɓ(Ŋ(1,o4/2));Ű CompositionWords ĸ ĺ(!d2)ĸ.reset(new(ɵ)ϒ[2*o4])ø(ý ȍ=1;ȍ<=æ;ȍ<<=1){Ƞ î=o4/ȍ,Ơ=î/2 ø(ý ʛ=ȍ;ʛ<2*ȍ ė ʛ){Ƞ Ų=(ʛ-ȍ)*î ĺ(Ų>=ó(Ј))ϵ ɓ ԋ=Ȯ(ˀ)+Ų ĺ(î==8){Ű evaluate_leaf8(Ȯ(ă)+Ų,ԋ,Ȯ(ǉ[ʛ]),ǉ[ʛ]Û,Ȯ(k5)+Ų,ʡ(8,Ј-Ų));}м(Ų+Ơ>=ó(Ј)){ĵ(ԋ+Ơ,Ơ,ԋ);}м(!ƕ Ӭ ű){ɓ a=d2?Ȯ(ƕ Ӭ):ĸ ӯ;ɓ b=d2?Ȯ(ƕ Ѳ):ĸ ӯ+o4 ĺ(!d2){ĵ(Ȯ(ƕ Ӭ),î,a)ԕ(Ȯ(ƕ Ѳ),î,b);}Ƹ Ą Ҭ(ԋ,î);Ű Ŏ(a,a,ԋ,î);Ű Ŏ(b,b,ԋ,î);Ƹ Ą fft ԍ(a,î);Ƹ Ą fft ԍ(b,î)Ĩ Word norm=Ǘ î;ҡ(ԋ,b+Ơ,Ơ,norm);ҡ(ԋ+Ơ,a+Ơ,Ơ,norm);}ŝ{Ť Word ω ɚ,l8 ɚ;Ƣ(î==16);Ű evaluate_middle8(ω,l8,ԋ,Ȯ(ǉ Ѳ),ǉ Ѳ Û,Ȯ(ǉ Ӭ),ǉ Ӭ Û)ԕ(ω,8,ԋ)ԕ(l8,8,ԋ+8);}}}Ҥ}}
#endif
ł dfs=ѯ(ł&&self,ý ʛ,ý l,ý r,F ԋ)->Ĉ{if(l*ŀ>=ó(Ј))Ú Ȋ î=(r-l)*ŀ,Ơ=î/2 ĺ(r-l у||î<=8){
#if defined(__x86_64__) || defined(__i386__)
Ġ{if(î==8 ȏ){Ƞ þ=ʡ(8,Ј-l*ŀ);Ű evaluate_leaf8(Ȯ(ă)+l*ŀ,Ȯ(ԋ),Ȯ(ǉ[ʛ]),ǉ[ʛ]Û,Ȯ(k5)+l*ŀ,þ)а}}
#endif
ƻ ӧ(î)ø(ö î Į)Ć(ý j Ē j+i<î&&j<ó(ǉ[ʛ]Û)ų)ӧ[î-1-i-j]+=ԋ Ĕ*ǉ[ʛ]Ƃ ø(Ż l*ŀ ϼ ʡ(r*ŀ,Ј)Į){T Ĵ Ɠ(ý j=î-1;j и j)Ĵ=Ĵ*k5 Ĕ+ӧ Ƃ;ă ƈ Ĵ;}Ú;}Ƞ ρ=(l+r)/2 ĺ(ρ*ŀ>=ó(Ј)){self(self,2*ʛ,l,ρ,F(ԋ ʷ Ơ,ԋ ƃ))а}F ω,l8 ĺ(!ƕ Ӭ ű){Ӗ(ú ƻ&>(ԋ));F a=d2?Ɔ(ƕ Ӭ):ƕ Ӭ;F b=d2?Ɔ(ƕ Ѳ):ƕ Ѳ;ǅ Ȉ(a,a,ԋ,î);ǅ Ȉ(b,b,ԋ,î);ǅ Ʋ(a);ǅ Ʋ(b)Ĩ Word norm=Ǘ î;ҡ(Ȯ(b),Ȯ(b)+Ơ,Ơ,norm);ҡ(Ȯ(a),Ȯ(a)+Ơ,Ơ,norm);b Ř(Ơ);a Ř(Ơ);ω=Ɔ(b);l8=Ɔ(a);}ŝ{ω Ř(Ơ);l8 Ř(Ơ);Ǝ done=Ś;
#if defined(__x86_64__) || defined(__i386__)
Ġ{if(î==16 ȏ){Ű evaluate_middle8(Ȯ(ω),Ȯ(l8),Ȯ(ԋ),Ȯ(ǉ Ѳ),ǉ Ѳ Û,Ȯ(ǉ Ӭ),ǉ Ӭ Û);done=Ƈ;}}
#endif
if(!done){F a=F::Ź(ԋ,ǉ Ѳ),b=F::Ź(ԋ,ǉ Ӭ)ԕ(a ʷ Ơ,Ơ,ω ü)ԕ(b ʷ Ơ,Ơ,l8 ü);}}self(self,2*ʛ,l,ρ,Ɔ(ω));self(self,2*ʛ+1,ρ,r,Ɔ(l8));};dfs(dfs,1,0,æ,Ɔ(ˀ))ȧ}ċ Ħ F>build(Ù џ&xs,ý&æ,ý ŀ,Ǝ l4,Ħ Ħ T>>*ƕ=Ƿ){Ƞ f5=(ó(Ԇ)+ŀ-1)/ŀ ǟ(æ<f5)æ<<=1;Ħ F>ǉ(Ё)ĺ(ƕ)ƕ->resize(Ё)ø(ö æ Į){if(i>=f5){ǉ[æ+i]={1};ӈ;}F p;p.reserve(ŀ+1);p Ӯ(Ӽ)ø(ý j=i*ŀ;j<ʡ((i+1)*ŀ,Ԇ)ų){Ƞ d=ó(p Û)-1 ĺ(l4){p Ӯ(0)ø(ң=d+1;k>=1;--k)p Ƭ-=xs Ƃ*p[k-1];}ŝ{p Ӯ(p.back())ø(ң=d;k>=1;--k)p ѓ p[k-1]-xs Ƃ*p Ƭ;p Ŭ*=-xs Ƃ;}}ǉ[æ+i]=Ɔ(p);}Ć(Ż æ-1;i>=1;--i){if(ǉ Ʊ Û у&&ǉ Ʊ Ŭ==Ӽ){ǉ ƈ ǉ ɔ;ӈ;}ý d9=31-__builtin_clz((ϒ)i);ή=(æ>>d9)*ŀ ĺ(ƕ&&î>=32){ł&a=(*ƕ)ɔ ѵ&b=(*ƕ)Ʊ ѵ extend_transform=ѯ(ƻ&a,ʧ p){if(a ű){a Ř(î);Ȫ(p ü,p ƃ,a ü);Ӗ(a);}ŝ{Ƞ Ơ=î/2;ƻ ȇ(Ơ)ԕ(p ü,ʡ(Ơ,p Û),ȇ ü)ĺ(ó(p Û)>Ơ)ȇ Ŭ-=p[Ơ];ǅ ɘ(ȇ,Ӽ,ǅ ntt_roots(î)[î/2]);Ӗ(ȇ);a ӭ(a ƃ,ȇ ü,ȇ ƃ);}};extend_transform(a,ǉ ɔ);extend_transform(b,ǉ Ʊ);џ c(î);ǅ Ȉ(c,a,b,î)ĺ(i>1)(*ƕ)ƈ c;ǅ Ʋ(c)Ĩ ǧ=(ý)(ǉ ɔ Û+ǉ Ʊ Û-1);ǉ Ĕ Ř(n)Ĩ T ђ=T θ(Ǘ î);Ġ ҡ(Ȯ(ǉ Ĕ),Ȯ(c),ĥ(n,î),ђ Ȅ)ї Ć(Ȕ ĥ(n,î)ų)ǉ Ĕ ˉ c Ƃ*ђ ĺ(n==î+1){Ù T Μ=ǉ ɔ.back()*ǉ Ʊ.back();ǉ Ĕ Ŭ-=Μ;ǉ Ĕ
[î]=Μ;}}ŝ{ǉ ƈ F::Ź(ǉ ɔ,ǉ Ʊ);}}Ú ǉ;}};}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
Ӏ{ā ˎ{Ę ʃ Č ͼ ʃ Half ͼ ʃ ć ͼ ʃ inverse_log ͼ ʃ ȵ ͼ ʃ Ǟ;ò ӂ ϴ=ʃ ϴ;Ũ Ě:public ʃ Ě{Ϳ Ě(Õ č):ʃ Ě(č){}Ĝ ś(ƿ){ł span=ʃ Ě::ś(n)ĺ(!zero_initialized())ż(span ü,span ƃ,0u)Ū span;}};Ϥ{Ę ā ʃ ë;}}ā ˎ{Ϥ{Ǹ add_mod(ɍ,ĭ y){Ú ń(ƫ(x,y),kP);}Ǹ subtract_mod(ɍ,ĭ y){Ú ń(ê(ƫ(x,ɟ),y),kP);}Ǹ to_montgomery(ɍ){Ú ń(ŭ(x,Ʈ(kR)),kP);}í forward_small(ҳ f,Ό,ȕ,М){if(m у)Ú Ȣ h=m/2 Ƥ s=ʮ(ķ,k)ʋ{ġ lo=f Ƃ,hi=ń(ŭ(f Ǻ,s),kP);f ˉ add_mod(lo,hi),f Ǻ=subtract_mod(lo,hi);}forward_small(f,h,Ӷ,ķ);forward_small(f+h,h,Ӷ+1,ķ);}í ҧ(ҳ f,Ό,ȕ,Ɣ ô){if(m у)Ú Ȣ h=m/2;ҧ(f,h,Ӷ,ô);ҧ(f+h,h,Ӷ+1,ô)Ƥ s=ʮ(ô,k)ʋ{ġ a=f Ƃ,b=f Ǻ;f ˉ add_mod(a,b),f Ǻ=ń(ŭ(ê(ƫ(a,ɟ),b),s),kP);}}í transpose8(ĭ(&r)ɚ){ĭ t ɚ,u Ҹ(ö 8 Ƽ 2)ͻ,t Υ Ȁ(r Ĕ,r Υ)ø(ö 8 Ƽ 4)Ć(ˋ 2 Ȟ)u[i ʳ k]=ɻ(t[i+k],t[i+k+2]),u[i ʳ k+1]ǿ(t[i+k],t[i+k+2])ɫ ѹ,з}ԃ montgomery_scalar(Ö x,Ö y){ӂ kInverseR=ã ǹ(kR,ӓ)Ū ʀ(ʀ(x,y),kInverseR);}ˮ ʩ(Һ,Ό,Ҡ,ȕ,Ņ c){ȱ,p=ɟ,r=ĕ(kR);ɍ[33],xo[33],y[33],yo[33];
#pragma GCC unroll 33
Ć(ɮ Ē i<=m Į)x ƈ Ō(ϋ i),xo ƈ ñ(x Ĕ,32);
#pragma GCC unroll 33
Ć(Ŀ<=k ų)y ˉ Ō(b Ƨ j),yo ˉ ñ(y Ƃ,32)Ю;
#pragma GCC unroll 65
Ć(Ӄ<=m+k ė t){н=đ,ȇ=đ Ȣ ù=t>k?t-k:0,ȣ Ч(t,m);
#pragma GCC unroll 17
Ć(ɮ=ù;i<=ȣ Į){Ŧ=ð(Ŧ,ß(x Ĕ,y[t-i]));ȇ=ð(ȇ,ß(xo Ĕ,yo[t-i]));}if(ȣ-ù>=12)Ŧ=ё(Ŧ),ȇ=ё(ȇ);Ĥ Š(c ϛ,ů(ǝ));}}í multiply_lanes_any(Һ,Ό,Ҡ,ȕ,Ņ c){if(m==k){switch(m){case 1:Ú ʩ(a,1,b,1,c)ϻ 2:Ú ʩ(a,2,b,2,c)ϻ 4:Ú ʩ(a,4,b,4,c)ϻ 8:Ú ʩ(a,8,b,8,c)ϻ 16:Ú ʩ(a,16,b,16,c);}}ʩ(a,m,b,k,c);}Ũ Stack{Ϳ Stack(Ĝ i6):base_(i6 â),ͽ{Ģ ӝ=à(n)ĺ(top_+ӝ>size_)ǩ;ǚ p=base_+top_;top_+=ӝ Ū p;}Ⱦ Ņ base_;Õ size_,top_ Ē};}Ũ Ȥ{Ⱥ ċ ɿ č(ý Δ){Ú à(2*ã ϙ(Ŋ(Δ,ʵ)));}Ȥ(Ě&ŋ,ý Δ):lg_max_(Δ){if(Δ<3||Δ>Č::Ҕ)ǩ Ȋ lg=Ŋ(Δ,ʵ)Ȣ n8=ã ϙ(lg),entries=(Ǽ lg)ș ø(ң=3;k<=Δ Ȟ)leaf_scale_ ѓ ã ǹ(Ö(1)<<(k-3),ä ӓ);}ý Δ()Ù{Ú lg_max_;}Ĉ ı(ĝ in,Õ ƍ,ɐ)Ù{Ɵ Ĩ Ɏ src=Ο ĺ(ǲ<64)Ú small(src,į,0);run(į,src,ǂ{Ɛ},0);}Ĉ ɇ(ĝ in,Õ ƍ,ɐ)Ù{Ɵ;ʐ(2*ǲ)Ĩ Ɏ src=Ο ĺ(ǲ<64)Ú small(src,į,1)Ĩ Ҟ}Ĉ ć(ĝ in,ɐ,Ö c=1)Ù{Ɵ;ʐ(ǲ)ĺ(ǲ<64){Ģ nv=ǲ/8;ϣ щ f ˉ Ō(in ʥ j);ҧ(f,nv,0,ō)Ƥ s(ŧ(ǲ,c))щ Š(į ʥ j,ń(ŭ(f Ƃ,s),kP))а}run(į,Ɏ(Ƿ,0,0),Ɉ{ō,in â},ŧ(ǲ,c));}Ĉ inverse_upper(ĝ in,ɐ,Ö c=1)Ù{Ɵ;ʐ(2*ǲ)Ȣ ӵ Ƥ s(ŧ(ǲ,c))ĺ(ǲ<64){ϣ щ f ˉ Ō(in ʥ j);ҧ(f,nv,1,ō)щ v ˉ ń(ŭ(f Ƃ,s),kP)а}Ù ǆ Ŝ(Ɛ,ō,Ɉ{ō,in â-ǲ})ǡ{ƽ 4 Ȳ ő(į ʥ t*h,h,4+t
)Ĩ Τ w(ō,1)ʋ{ϣ={v Ƃ,Ӊ,v ɋ,v Ɋ};τ(f,w)Ȳ v[j+t*h]=ń(ŭ(f Ѱ,s),kP);}Ú;}ƽ 2;ő(į â,h,2);ő(į ʥ h,h,3)Ƥ scale_z(ʀ(ŧ(ǲ,c),ō ƹ))ʋ{ġ u=v Ƃ,w=Ӊ;v ˉ ń(ŭ(ƫ(u,w),s),kP),Ӊ=ń(ŭ(ǘ(u,w),scale_z),kP);}}Ĉ leaf_products(Һ,Ҡ,ɐ)Ù{ѻ ȟ=ǲ/8;ɥ;Ε(Ļ Ŭ,Ō(a),Ϙ(Ɛ,0))ø(ә ȟ ė p){if(p+1<ȟ)Ε(Ļ[(p+1)&1],Ō(ϋ p+8),Ϙ(Ɛ,p+1));Š(į ʥ p,ń(ǽ(Ļ[p&1],b Ƨ p),kP));}}Ĉ standard_to_lanes(Ɣ in,Õ ď,ƿ,Ȩ)Ù{Ɵ ø(ә n/8 ė p){ĭ f Ҹ(Õ l Ē l<8 ė l)f[l]=Ō(in+l Ԍ Ƨ p);transpose8(f)Ƥ z=ʮ(Ɛ,p);ĭ lo ɛ,hi ɛ Ȳ{ġ zb Ҝ(f[t+4],z);lo Ѱ=ƫ(f Ѱ,zb),hi Ѱ=ǘ(f Ѱ,zb);}forward_h1(lo,Τ(Ɛ,2*p));forward_h1(hi,Τ(Ɛ,2*p+1))Ȳ Š(є 64*p ϛ,ů(lo Ѱ)),Š(є 64*p+32 ϛ,ů(hi Ѱ));}}Ĉ leaf_product_sums(Һ,Ҡ,Ɣ c,Ɣ d,ɐ)Ù{ѻ ȟ=ǲ/8;ɥ ǥ;Ε(Ļ Ŭ Ŭ,Ō(a),Ϙ(Ɛ,0));Ε(Ļ Ŭ ƹ,Ō(c),Ϙ(Ɛ,0))ø(ә ȟ ė p){if(p+1<ȟ){Œ w=Ϙ(Ɛ,p+1);Ε(Ļ[(p+1)&1]Ŭ,Ō(ϋ p+8),w);Ε(Ļ[(p+1)&1]ƹ,Ō(c Ƨ p+8),w);}ġ ab=ǽ(Ļ[p&1]Ŭ,b Ƨ p),cd=ǽ(Ļ[p&1]ƹ,d Ƨ p);Š(į ʥ p,ů(ϑ));}}Ŵ Ѥ(Һ,Ҡ,ɐ){Ɵ ø(Ů ǲ ƞ)Š(į ȝ,ń(Ŕ(Ō(a+i),Ō(b+i)),kP));}Ŵ pointwise_product_sums(Һ,Ҡ,Ɣ c,Ɣ d,ɐ){Ɵ;ȱ,p=ɟ ø(Ů ǲ ƞ){Ҽ Ō(a+i),y=Ō(b+i),z=Ō(c+i),w=Ō(d+i);н=ð(ß(x,y),ß(z,w));ĭ ȇ=ð(ɹ,ß(
ñ(z,32),ñ(w,32)));Ĥ Š(į ȝ,ń(ǝ,kP));}}Ⱦ Ö ŧ(ƿ,Ö c)Ù{ʦ lg=ĩ(n)ĺ(lg>Č::Ҕ)ǩ Ū ʀ(leaf_scale_[lg],c);}Ĉ ʐ(ƿ)Ù{if(!ϧ(n)||n<8||ĩ(n)>lg_max_)ǩ;}ä Ɏ source(ĝ in,Õ ƍ,ƿ)Ù{ʐ(n)ĺ(ƍ>n||in Û>n-ƍ)ǩ Ū ä Ɏ(in â,in Û,ƍ);}Ĉ small(Ù ä Ɏ&src,ɐ,Õ ŀ)Ù{ѻ nv=ǲ/8;ϣ щ f ˉ src(j);forward_small(f,nv,ŀ,Ɛ)щ Š(į ʥ j,f Ƃ);}Ϣ run(ҽ,Ù ä Ɏ&in,Ϧ,Ö ŧ)Ù{Ɵ Ĩ ǆ Ŝ(Ɛ,ō,b9)Ω{Ι(v,h,ȉ,ō,Ʈ(ŧ));}ŝ{ƽ 2;ʠ forward_top2(in,v,h)Ѵ Њ(ƭ::Ƚ)Ӥ(v,h,ȉ,ŧ);}}ý lg_max_;Ņ Ɛ,*ō;Ö leaf_scale_[Č::Ҕ+1]={};};Ɯ LaneLayout{ċ ɿ kWords=8;ċ ӂ kBase=32 ͼ ȼ=ä ĭ;Ɯ Node{ȼ ũ,Μ;};Ŵ Ź(Һ,Ό,Ҡ,ȕ,Ņ c){ä multiply_lanes_any(a,m,b,k,c);}ċ Õ î(Ö ũ){Ú ɬ(Ŋ<Ö>(ũ,1));}Ŵ Ŏ(Ù Ȥ&,Һ,Ҡ,ɐ){Ȥ::Ѥ(a,b,į);}ċ ȼ Μ(Ɣ c,ȼ ũ){Ù ȼ ȯ=ň(_mm256_slli_epi32(ũ,3),ɜ)Ū _mm256_i32gather_epi32(õ Ƞ*>(c),ȯ,4);}ċ ȼ ʂ(ȼ x,ȼ y){Ú ň(x,y);}ċ ȼ lead_product(ȼ x,ȼ y){Ú ä ń(ä Ŕ(x,y),ä kP);}ċ ȼ top(ȼ ũ,ȼ Μ,Õ î){Ú _mm256_and_si256(_mm256_cmpeq_epi32(ũ,ä ĕ(Ö(î))),Μ);}Ŵ ʫ(Ņ c,ȼ x){ä Š(c,ä subtract_mod(ä Ō(c),x));}Ŵ ƫ(Ņ c,ȼ x){ä Š(c,ä add_mod(ä Ō(c),x));}Ŵ put(Ņ c,ȼ x){ä Š(c,x);}};Ɯ ǖ{ċ ɿ kWords=1;ċ ӂ kBase Ē Ę ȼ=Ö;Ɯ Node
{ȼ ũ,Μ;};Ŵ Ź(Ɣ,Õ,Ɣ,Õ,Ņ){}ċ Õ î(Ö ũ){Ú ʘ(8,ɬ(ũ));}Ŵ Ŏ(Ù Ȥ&t,Һ,Ҡ,ɐ){t.leaf_products(a,b,į);}ċ ȼ Μ(Ɣ c,ȼ ũ){Ú c[ũ];}ċ ȼ ʂ(ȼ x,ȼ y){Ú x+y;}ċ ȼ lead_product(ȼ x,ȼ y){Ú ä montgomery_scalar(x,y);}ċ ȼ top(ȼ ũ,ȼ Μ,Õ î){Ú ũ==î?Μ:0;}Ŵ ʫ(Ņ c,ȼ x){*c=*c>=x?*c-x:*c+ä Ͼ x;}Ŵ ƫ(Ņ c,ȼ x){*c=*c+x>=ä kP?*c+x-ä kP:*c+x;}Ŵ put(Ņ c,ȼ x){*c=x;}};Ɯ DropTransforms{Ĉ ŕ()(Õ,Õ,ĝ)Ù{}};Ő Layout,Ũ Leaves,Ũ Keep=DropTransforms>Ũ Ώ{Ⱥ Ę ȼ=ӆ ƴ ȼ ͼ Node=ӆ ƴ Node;ċ è Ǝ kPlaces=requires(Keep k,ɮ){k.place(i,i,i,i);};Ɯ Root{Ĝ ļ;Node ʛ;Õ î;};ċ Õ Ƅ(ȡ,Õ o4){Ú à(þ+1)Ƨ à(ƴ kWords*(ƴ î(Ö(o4))+1))+4096;}Ώ(Ù Ȥ&t,Ù Leaves&ȟ,Є,Keep keep={}):t_(t),leaves_(ȟ),keep_(keep),ʄ(ĸ),Ȑ(ȟ.þ()),Ͱ(ʄ ņ(Ȑ+1)){Ͱ Ŭ Ɠ(ѿ Ȑ Ȟ)Ͱ[k+1]=Ͱ Ƭ+ȟ.ũ(k);}Root ˀ(){Ģ þ=Ȑ,î=ƴ î(ũ(0,þ));ɿ w=ƴ kWords;ǚ c=ʄ ņ(w*(î+1))ĺ(þ у||ũ(0,þ)<=ƴ kBase){ʇ(c,w*(î+1),0)Ū{{c,w*(î+1)},ļ(0,þ,c),î};}ǚ a=ʄ ņ(w*î)Ȣ ρ=split(0,þ);Ņ left_out=a,*right_out=c Њ(kPlaces)left_out=keep_.place(0,ρ,þ,w*î),right_out=left_out Я(w*î)Ĩ Node ω=build(0,ρ,left_out,î),l8=build(ρ,þ,right_out,î);ƴ Ŏ(t_,left_out
,right_out,{a,w*î})Њ(requires{keep_.ˀ(ĝ{});})keep_.ˀ(ĝ(a,w*î));t_.ć({a,w*î},{c,w*î})Ы ѷ put(c+w*î,top)Ū{{c,w*(î+1)},ʛ,î};}Õ split(І,п)Ù{ğ ƪ=Ͱ[lo]+(Ͱ[hi]-Ͱ[lo])/2;Õ ρ=Õ(Ø lower_bound(Ͱ+lo+1,Ͱ+hi-1,ƪ)-Ͱ)ĺ(ρ>lo+1&&Ͱ[ρ]>=ƪ&&ƪ-Ͱ[ρ-1]<Ͱ[ρ]-ƪ)--ρ Ū ρ;}Ö ũ(І,п)Ù{Ú Ͱ[hi]-Ͱ[lo];}Ⱦ Node ļ(І,п,Ņ c)Ù{if(hi-lo у)Ú leaves_.Ō(lo,c);ɿ w=ƴ kWords,kMax=ʘ(ƴ kBase,1);Ɯ Part{Ņ c;Õ ũ;};Part part[kMax];ћ i6 ǥ[w*2*kMax];ȡ=hi-lo;Ņ to=i6 Ŭ;ȼ ũ{}ø(ѿ þ Ȟ){Ù ȼ d=leaves_.Ō(lo+k,to).ũ;ũ=k?ƴ ʂ(ũ,d):d;part ѓ{to,leaves_.ũ(lo+k)};to+=w*(part Ƭ.ũ+1);}Ć(Õ ȍ=1;þ>1 ė ȍ){to=þ==2?c:i6[ȍ&1];Õ next Ɠ(ȕ Ē k+1<þ;k+=2){Ù Part&a=part Ƭ,&b=part[k+1];ƴ Ź(a.c,a.ũ,b.c,b.ũ,to);part[next++]={to,a.ũ+b.ũ};to+=w*(a.ũ+b.ũ+1);}if(þ%2){Ù Part&a=part[þ-1]ԕ(a.c,w*(a.ũ+1),to);part[next++]={to,a.ũ};}þ=next;}Ú{ũ,ƴ Μ(c,ũ)};}Node build(І,п,Ȩ,Õ ɖ){ɿ w=ƴ kWords ĺ(hi-lo у||Ѝ<=ƴ kBase){Ù Node ʛ=ļ(lo,hi,į);t_.ı({į,w*(Ѝ+1)},0,{į,w*ɖ})Ӓ}Ģ î=ƴ î(Ѝ),č=w*î,ѥ;Ң,*b,*c;Õ ρ Њ(kPlaces){ρ=split(lo,hi);a=keep_.place(lo,ρ,hi,č),b=a Я(č),c=ʄ ņ(č+w);}ŝ{a=ɖ>=2*î?є č:ʄ ņ(č
);b=c=ʄ ņ(č+w);ρ=split(lo,hi);}Ù Node ω=build(lo,ρ,a,î),l8=build(ρ,hi,b,î)Ы;ƴ Ŏ(t_,a,b,{į,č})ĺ(ɖ>î){t_.ć({į,č},{c,č})ѷ ʫ(c,top);t_.ɇ({c,č},0,{є č,č});ƴ ƫ(c,top);ƴ put(c+č,top)ø(Ό=2*î;m<ɖ;m*=2)t_.ɇ({c,č+w},0,{є w*m,w*m});}ʄ Ȏ Ӓ}Ù Ȥ&t_ Ĩ Leaves&leaves_;Keep keep_;ä Stack ʄ;Õ Ȑ;Ņ Ͱ;};í lane_columns(Ɣ c,ȡ,ğ(&æ)ɚ,ǚ(&į)ɚ){ѻ i5=*Ø max_element(æ,æ+8)ø(ǈ i5 Ӕ){ĭ r Ҹ(Ů 8 Į)r ƈ j+i<þ?Ō(c Ƨ(j+i)):đ;transpose8(r)ø(ý l Ē l<8 ė l)if(j<æ[l])ū(į[l]+j,r[l]);}}}ā ˎ{Ϥ{ĉ Ǹ ˍ(н,ĭ ȇ,Õ ơ){ȱ,p=ɟ,r=ĕ(kR)Ю ĺ(ơ>12)Ŧ=ё(Ŧ),ȇ=ё(ȇ);Ĥ Ú ů(ǝ);}Ą<Õ K,Õ C,Õ B>ˮ э(Ɣ w,Ɣ q,Ȩ){Ə(K<=16);ĭ wo[K+C],qo[K+1];
#pragma GCC unroll 32
Ć(Ů K+C Į)wo ƈ ñ(Ō(w Ƨ i),32);
#pragma GCC unroll 17
Ć(Ŀ<=K ų)qo ˉ ñ(Ō(q Ƨ j),32)ø(Õ t0 Ē t0<C;t0+=B){н[B],ȇ[B];
#pragma GCC unroll 8
Ć(Õ s Ē s<B ė s)Ŧ[s]=ȇ[s]=đ;
#pragma GCC unroll 17
Ć(Ŀ<=K ų){ġ y=Ō(q Ƨ j);
#pragma GCC unroll 8
Ć(Õ s Ē s<B&&t0+s<C ė s){Ŧ[s]=ð(Ŧ[s],ß(Ō(w Ƨ(K+t0+s-j)),y));ȇ[s]=ð(ȇ[s],ß(wo[K+t0+s-j],qo Ƃ));}}
#pragma GCC unroll 8
Ć(Õ s Ē s<B&&t0+s<C ė s)Š(į Ƨ(t0+s),ˍ(Ŧ[s],ȇ[s],K+1));}}Ɨ í э(Ɣ w,ȕ,Ɣ q,ȡ,Ȩ){Ć(Ӄ<þ ė t){н=đ,ȇ=đ ø(Ŀ<=k ų){Ҽ Ō(w Ƨ(k+t-j)),y=Ō(q Ƨ j)ɴ}Š(į ϛ,ˍ(Ŧ,ȇ,k+1));}}Ɯ ѣ{Ɣ k5;Õ n2;ȡ()Ù{Ú n2;}Ö ũ(Õ)Ù{Ú 1;}Ǔ Node Ō(ȕ,Ņ c)Ù{ġ Μ=to_montgomery(ê(ɟ,ä Ō(k5 Ƨ k)));Š(c,ĕ(kR));Š(c+8,Μ)Ū{ĕ(1),Μ};}};Ɯ LaneProducts{Ɣ c;Õ ď;Ö h4;ȡ()Ù{Ú 8;}Ö ũ(Õ)Ù{Ú h4;}ǖ::Node Ō(ȕ,Ȩ)Ù{ĵ(c+k Ԍ,h4+1,į)Ū{h4,į[h4]};}};Ũ Ύ{Ⱥ Ɯ Keeper{Ύ*Š;Ņ place(Õ,Õ ρ,Õ,Õ č)Ù{Ú Š->place(ρ,č);}Ĉ ˀ(ĝ Ɓ)Ù{Š->root_=Ɓ;}};Ĉ reset(Ĝ č,Ĝ ȯ){words_=č,used_=0,pair_=ȯ â;}Ņ place(Õ ρ,Õ č){Ģ ӝ=2 ӌ(č)ĺ(used_+ӝ>words_ Û)ǩ;pair_[ρ]=Ö(used_);used_+=ӝ Ū words_ Ʒ pair_[ρ];}Ɣ pair(Õ ρ)Ù{Ú words_ Ʒ pair_[ρ];}ĝ ˀ()Ù{Ú root_;}Ⱦ ĝ root_;Ĝ words_;Õ used_ Ē Ņ pair_=Ƿ;};Ő Layout,Ũ Tree>Õ kept_words(Ù Tree&ǉ,І,п){if(hi-lo у||ǉ.Ѝ<=ƴ kBase)Ú 0 Ȣ Χ,č=ƴ kWords*ƴ î(ǉ.Ѝ)Ū 2 ӌ(č)+kept_words<Layout>(ǉ,lo,ρ)+kept_words<Layout>(ǉ,ρ,hi);}Ą<Õ D>Ĉ product_level(Ɣ c7,Ȩ,Õ j3){Ć(Ů j3 Į)ʩ(c7 Ƨ(D+1)*(2*i),D,c7 Ƨ(D+1)*(2*i+1),D,į Ƨ(2*D+1)*i);}ɿ middle_block(Õ d){Ú d>=16?0:d==8?3:d==4?4:2;}Ą<Õ D>
Ĉ middle_level(Ɣ k3,Ɣ Ɩ,Ȩ,Õ j3){Ć(Ů j3 Į){Ɣ w=k3 ү*D*(i/2),*q=Ɩ Ƨ(D+1)*(i^1)Њ(middle_block(D)ε)э(w,D,q,D,į Ƨ D*i)ї э<D,D,middle_block(D)>(w,q,į Ƨ D*i);}}Ő F>Ĉ with_degree(Õ d,F&&f){switch(d){case 1 ѭ 1>()ϻ 2 ѭ 2>()ϻ 4 ѭ 4>()ϻ 8 ѭ 8>()ϻ 16 ѭ 16>();default:ǩ;}}Ũ BaseTree{protected:BaseTree(Ù ѣ&n2,Ņ Ƴ):slots_(n2),Ϭ(Ƴ){}Ņ Ӟ(І,ȡ,Ɣ*Ɩ){Ņ at=ю ø(ѿ þ Ȟ)slots_.Ō(lo+k,at ү*k);Ɩ Г at;at+=16*þ ø(Õ d=1,j=1;2*d<þ;d*=2,++j){with_degree(d,ѯ<Õ D>(){Ĺ(D<=Ǔ kBase/4)product_level<D>(Ɩ[j-1],at,þ/(2*D));});Ɩ ˉ at;at+=8*(2*d+1)*(þ/(2*d));}Ú at;}Ĉ descend_levels(І,ȡ,Ɣ w,Ɣ Ù*Ɩ,ǚ*Ǯ){Ɣ k3=w ø(Õ d=þ/2,j=ĩ(þ)-1;;d/=2,--j){ǚ į=d у?Ϭ Ƨ(lo-lo_):Ǯ[j&1];with_degree(d,ѯ<Õ D>(){middle_level<D>(k3,Ɩ Ƃ,į,þ/D);})ĺ(d у)Ú;k3=į;}}Ɣ build(Õ v,І,п){ǚ c=free_ ĺ(hi-lo у){slots_.Ō(lo,c);free_+=16 Ū c;}Ģ ρ=lo+(hi-lo)/2;ΰ[2*v]=build(2*v,lo,ρ);ΰ[2*v+1]=build(2*v+1,ρ,hi)ĺ(v у)Ú Ƿ;ǚ į=free_;multiply_lanes_any(ΰ[2*v],ρ-lo,ΰ[2*v+1],hi-ρ,į);free_+=8*(hi-lo+1)Ū į;}Ĉ descend(Õ v,І,п,Ɣ w){if(hi-lo у)Ú Š(Ϭ Ƨ(lo-lo_),Ō(w))ͷ э(w,hi-ρ,ΰ[2*v+1],ρ-lo
,ω);э(w,ρ-lo,ΰ[2*v],hi-ρ,l8);descend(2*v,lo,ρ,ω);descend(2*v+1,ρ,hi,l8);free_=ω;}ћ ю[8*320];Ɣ ΰ[64];Ņ free_=ю Ĩ ѣ&slots_;Ņ Ϭ;Õ lo_ Ē};Ũ BaseDescent:private BaseTree{Ⱥ BaseDescent(Ù ѣ&n2,Ņ Ƴ):BaseTree(n2,Ƴ){}Ĉ run(І,п,Ɣ w){Ģ þ=hi-lo ĺ(þ у)Ú Š(Ϭ Ƨ lo,Ō(w))ĺ(ϧ(þ)){Ɣ Ɩ[6];Ņ at=Ӟ(lo,þ,Ɩ);ǚ Ǯ ǥ={at,at Ƨ þ}Ū descend_levels(lo,þ,w,Ɩ,Ǯ);}Ӵ}};}Ũ ʨ{Ⱥ ċ Õ č(Ό){б,þ=m8/8,ţ=ʌ(þ)+1 Ȣ o3=Ϡ Ȣ kept_lanes=ţ*(32*þ+64*(þ/32+1))Ȣ kept_top=14 ӌ(o3)Ū Ȥ Ư(ɶ(m8))Я(m8)Я(LaneTree::Ƅ(þ,þ))Я(kept_lanes)Я(þ)Я(8*((þ+8)/8*8))Я(TopTree::Ƅ(8,m8))Я(kept_top)Я(8);}ċ Õ descend_words(Ό){б Ȣ h5=Ǔ î(Ö(m8/8)),o3=Ϡ Ū 2 ӌ(8*ʘ(8,h5))+5 ӌ(o3)+4 ӌ(8*h5)+4096;}ʨ(Ě&ŋ,ĝ k5):m8_((Ј+7)/8*8),Ȑ(m8_/8),t_(ŋ,ɶ(m8_)){Ɵ;ǚ k0 ф(m8_)â;Ȫ(k5 ü,k5 ƃ,k0);slots_={k0,Ȑ};lanes_.emplace(t_,slots_,Ĭ(LaneTree::Ƅ(Ȑ,Ȑ)),Ύ::Keeper{&ҍ});ҍ.reset(Ĭ(kept_words<LaneLayout>(*lanes_,0,Ȑ)),Ĭ(Ȑ))Ĩ LaneTree::Root h6=lanes_->ˀ()Ȣ ď=(Ȑ+8)/8*8;ǚ İ ф(8 Ԍ)â;ğ æ=Ö(Ȑ+1);ğ ȴ ɚ={æ,æ,æ,æ,æ,æ,æ,æ};ǚ į ɚ={İ,İ+ď,İ ʳ ď,İ+3 Ԍ,İ+4 Ԍ,İ+5 Ԍ,İ+6 Ԍ,İ+7 Ԍ};lane_columns(h6.ļ â,h6.î+1,ȴ,į);
lane_products_={İ,ď,Ö(Ȑ)};top_.emplace(t_,lane_products_,Ĭ(TopTree::Ƅ(8,m8_)),Ύ::Keeper{&Ѣ});Ѣ.reset(Ĭ(kept_words<ǖ>(*top_,0,8)),Ĭ(8));ΰ=top_->ˀ().ļ Ȓ(m8_+1);}ʨ(Ù ʨ&)=delete;ʨ&ŕ=(Ù ʨ&)=delete;Õ size()Ù{Ú m8_;}ĝ Ŏ()Ù{Ú ΰ;}ĝ product_transform()Ù{Ú Ѣ.ˀ();}Ĉ descend(ĝ w,Ĝ Ƴ,Ö c,Є){Ɵ;Stack Ǚ(ĸ)Ȣ h5=Ǔ î(Ö(Ȑ))Ĩ LaneState ˀ=lane_state(w,c,Ǚ);BaseDescent ƻ(slots_,Ƴ â)ĺ(Ȑ<=Ǔ kBase)Ú ƻ.run(0,Ȑ,ˀ.state Ƨ(h5-Ȑ));descend_lanes(0,Ȑ,ˀ.state,ˀ.u,Ǚ,ƻ);}protected:Ę LaneTree=Ώ<LaneLayout,ä ѣ,ä Ύ::Keeper>ͼ TopTree=Ώ<ǖ,ä LaneProducts,ä Ύ::Keeper>;ċ ý ɶ(Õ m8){Ģ т=8*Ǔ î(Ö(m8/8)),top=Ϡ Ū ĩ(Ŋ(т,top));}Ĉ halve(Ņ p,Õ č)Ù{ѻ h=č/2;t_.inverse_upper({p+h,h},{p+h,h},Ͼ 1);t_.ı({p+h,h},0,{p+h,h})ø(Ů h ƞ)Š(p+i,add_mod(Ō(p+i),Ō(p+h+i)));}Ĉ ļ(Ņ p,Õ č,Ö u)Ù{Ɵ ĺ(č==8)Ú t_.ć({p,8},{p,8},u)Ȣ h=č/2;ğ c=Ơ(u);t_.ć({p,h},{p,h},c);t_.inverse_upper({p+h,h},{p+h,h},Ͼ c)ø(Ů h ƞ){ġ a=Ō(p+i),b=Ō(p+h+i);Š(p+i,subtract_mod(a,b)),Š(p+h+i,add_mod(a,b));}}Ɯ LaneState{Ņ state;Ö u;};LaneState lane_state(ĝ w,Ö c,Ѿ){ѻ h5=Ǔ î(Ö(Ȑ)),ď=ʘ(8,h5)Ĩ Ǝ Ɓ
=Ȑ>Ǔ kBase;ǚ т=Л(8 Ԍ)Ȣ в ǚ İ=Л(8 Ԍ)ĺ(!Ɓ)ʇ(İ,8 Ԍ,0)Ȣ î=ǖ::î(Ö(m8_));ǚ ˀ=Л(î);t_.ı(w,î-m8_,{ˀ,î});Ö u=1;descend_top(0,8,ˀ,c,Ǚ,İ,h5,ď,Ɓ?&u:Ƿ)ĺ(Ɓ){t_.standard_to_lanes(İ,ď,h5,т);}ŝ{Ć(ǈ ď Ӕ){ĭ r Ҹ(Õ l Ē l<8 ė l)r[l]=Ō(İ+l Ԍ+j);transpose8(r)ø(Ů 8 Į)Š(т Ƨ(j+i),r Ĕ);}}Ǚ Ȏ Ū{т,u};}Ĉ descend_top(І,п,Ɣ s,Ö u,Ѿ,Ņ İ,Õ h5,Õ ď,Ņ i1){Ù TopTree&ǉ=*top_ Ȣ î=ǖ::î(ǉ.Ѝ),Χ ȶ{Ģ xlo=n1[side]Ŭ,xhi=n1[side]ƹ,ʎ=ǖ::î(ǉ.ũ(xlo,xhi))Ȣ в ǚ p=Л(î);t_.leaf_products(s,Ѣ.pair(ρ)+(1-side)ӌ(î),{p,î})ĺ(xhi-xlo у&&i1){halve(p,î)ԕ(p,h5,İ+xlo Ԍ);*i1=Ơ(u);}м(xhi-xlo у){ļ(p,î,u)ԕ(p+î-h5,h5,İ+xlo Ԍ);}м(2*ʎ==î){halve(p,î);descend_top(xlo,xhi,p,Ơ(u),Ǚ,İ,h5,ď,i1);}ŝ{ļ(p,î,u);ǚ q=Л(ʎ);t_.ı({p+î-ʎ,ʎ},0,{q,ʎ});descend_top(xlo,xhi,q,1,Ǚ,İ,h5,ď,i1);}Ǚ Ȏ;}}Ĉ descend_lanes(І,п,Ɣ s,Ö u,Ѿ,ä BaseDescent&ƻ){Ù LaneTree&ǉ=*lanes_ Ȣ č=8*Ǔ î(Ö(ǉ.Ѝ)),Χ ȶ{Ģ xlo=n1[side]Ŭ,xhi=n1[side]ƹ,ũ=ǉ.ũ(xlo,xhi)Ȣ ʎ=8*Ǔ î(Ö(ũ)),в ǚ p=Л(č);Ȥ::Ѥ(s,ҍ.pair(ρ)+(1-side)ӌ(č),{p,č})ĺ(xhi-xlo у||ũ<=Ǔ kBase){ļ(p,č,u);ƻ.run(xlo,xhi,p+č-8*ũ);}м(2*ʎ==č){halve(p,č);descend_lanes
(xlo,xhi,p,Ơ(u),Ǚ,ƻ);}ŝ{х descend_lanes(xlo,xhi,q,1,Ǚ,ƻ);}Ǚ Ȏ;}}ċ Ö Ơ(Ö u){Ú ʀ(u,(ä kP+1)/2);}Õ m8_,Ȑ;Ȥ t_;ä ѣ slots_{};ä LaneProducts lane_products_{};ä Ύ ҍ,Ѣ;Ø optional<LaneTree>lanes_;Ø optional<TopTree>top_;ĝ ΰ;};Ϥ{ò ɿ kDirectEvaluation=Ǽ 22;í ԅ(О,ĝ k5,Ĝ Ƴ){ɿ ɡ=4 Ƥ to_r(kR)ø(Ů Ј ƞ*ɡ){ћ x[8*ɡ]={},v[8*ɡ]Ȣ here Ч<Õ>(8*ɡ,Ј-i)ԕ(k5 ʊ(i),here,x);ĭ xm[ɡ],acc[ɡ]ø(ϩ)xm[c]=ń(ŭ(Ō(x Ƨ c),to_r),kP),acc[c]=đ ø(ʍ=Ǳ;j-->0;){ġ fj=ĕ(f Ƃ)ø(ϩ)acc[c]=ń(ƫ(Ŕ(acc[c],xm[c]),fj),2*kP);}Ć(ϩ)Š(v Ƨ c,ń(acc[c],kP))ԕ(v,here,Ƴ ʊ(i));}}Ɯ Division{Õ big,k,î;Division(ƿ,Õ m8):big(Ŋ(n,m8)),k((big+1)/2),î(ʘ(64,ɬ(Ŋ(Ӷ-1,big)))){}ý log()Ù{Ú Ŋ(inverse_log(k),ĩ(î));}Õ Ƅ()Ù{Ú à(big)Я(k)Я(ȵ(k))+3 ӌ(î);}};í divide(Ƶ,Ù Division&s,О,ĝ q,ĝ qt,Ĝ d,Є){Ģ big=s.big,k=s.k,ɼ=big-k,n=s.î;Stack Ǚ(ĸ)ǫ g{Л(big),big},h{Л(k),k};ʇ(g ü,big-Ǳ,0)Ӎ(f ü,f ƃ,g ʊ(big-Ǳ));ć(t,q,h,{Л(ȵ(k)),ȵ(k)})ǫ a{Л(n),n},b{Л(n),n},th{Л(n),n}ǌ lower=[n](Õ æ){Ú æ<=n/2?Ʃ:ȉ;}Ȇ(g Ȓ(k),0,a)Ȇ(h,0,th);Ɗ(a,th,a,lower(k))ԕ(a ü,k,d ü)Ȇ(d Ȓ(k),0,a)ĺ(qt Û!=n){Ģ ơ Ч(Ѹ,n)ԕ(q ü,ơ,b
 ü)ĺ(Ѹ>n)b Г Ö((ŉ(b Ŭ)+q[n])Ұ)Ȇ(b Ȓ(ơ),0,b);qt=b;}Ɗ(qt,a,a,k>=n/2?ƨ:ȉ)ø(Ů ɼ Į)b ƈ g[k+i]>=a[k+i]?g[k+i]-a[k+i]:g[k+i]+Ͼ a[k+i];t.Ŷ(b Ȓ(ɼ),0,a,th,lower(ɼ))ԕ(a ü,ɼ,d ʊ(k));}}Ϥ{Ί evaluate_tree_words(ƿ,Ό){б Ĩ Division ȴ(n,m8)Ū ʨ Ư(m)+Č Ư(ȴ.log())Я(ȴ.big)Я(Ŋ(ȴ.Ƅ(),ʨ::descend_words(m)))Я(m8);}í evaluate_tree(Ě&ŋ,О,ĝ k5,Ĝ Ƴ){Ģ m=Ј;ʨ ǉ(ŋ,k5)Ȣ m8=ǉ Û Ĩ Division ȴ(Ǳ,m8)Ĩ Č t(ŋ,ȴ.log())ǫ d ф(ȴ.big)ǫ ĸ ф(Ŋ(ȴ.Ƅ(),ʨ::descend_words(m)));divide(t,ȴ,f,ǉ.Ŏ(),ǉ.product_transform(),d,ĸ)Ĩ Ǝ ӣ=m8==m&&Λ(Ƴ â)%32 ε ǫ all=ӣ?Ƴ:Ĭ(m8);ǉ.descend(d Ė(ȴ.big-m8,m8),all,kR,ĸ)ĺ(!ӣ)ĵ(all ü,m,Ƴ ü);}}Ί evaluate_words(ƿ,Ό){Ú n*m<=ä kDirectEvaluation?0:ä evaluate_tree_words(n,m);}í evaluate(Ě&ŋ,О,ĝ k5,Ĝ Ƴ){if(Ǳ*Ј<=ä kDirectEvaluation)Ú ä ԅ(f,k5,Ƴ);ä evaluate_tree(ŋ,f,k5,Ƴ);}}ā ˎ{Ϥ{Ǹ ё(ĭ s){Ú ð(ß(ñ(s,32),ĕ(kR)),ĳ(s,đ,0xAA));}Ą<Õ K>ĉ Ǹ cross_sum(ġ(&Ŧ)[K],ġ(&ȇ)[K],Õ ơ){Ĺ(K у){Ú ˍ(Ŧ Ŭ,ȇ Ŭ,ơ);}ŝ{if(ơ<=17)Ú ˍ(ð(Ŧ Ŭ,Ŧ ƹ),ð(ȇ Ŭ,ȇ ƹ),ơ)Ū ˍ(ð(ё(Ŧ Ŭ),ё(Ŧ ƹ)),ð(ё(ȇ Ŭ),ё(ȇ ƹ)),0);}}Ɨ í Ҍ(Һ,ƿ,Ҡ,ȕ,Ɣ c,Ɣ d,Ȩ){Ć(Ӄ<n+k ė t){н ǥ={},ȇ ǥ={}Ȣ ù ǥ
={t>k?t-k:0,t>n?t-n:0},ȣ ǥ={ĥ(t,Ԕ),ĥ(t,k-1)}ø(ɮ=ù Ŭ;i<=ȣ Ŭ Į){Ҽ Ō(ϋ i),y=Ō(d Ƨ(t-i));Ŧ Г ð(Ŧ Ŭ,ß(x,y));ȇ Г ð(ȇ Ŭ,ɹ);}Ć(ɮ=ù ƹ;i<=ȣ ƹ Į){Ҽ Ō(b Ƨ i),y=Ō(c Ƨ(t-i));Ŧ ƹ=ð(Ŧ ƹ,ß(x,y));ȇ ƹ=ð(ȇ ƹ,ɹ);}Š(į ϛ,cross_sum(Ŧ,ȇ,(ȣ Ŭ+1-ù Ŭ)+(ȣ ƹ+1-ù ƹ)));}}Ą<Õ D,Õ B>ˮ Ҍ(Һ,Ҡ,Ɣ c,Ɣ d,Ȩ){ɿ kPad=B-1,kSums=2*D>17?2:1;ɍ[D ʳ kPad],xo[D ʳ kPad],y[D ʳ kPad],yo[D ʳ kPad],co[D+1],dq[D+1]ø(Ů kPad Į)x ƈ xo ƈ y ƈ yo ƈ x[D+kPad+i]=xo[D+kPad+i]=y[D+kPad+i]=yo[D+kPad+i]=đ;
#pragma GCC unroll 16
Ć(Ů D Į){x[kPad+i]=Ō(ϋ i),xo[kPad+i]=ñ(x[kPad+i],32);y[kPad+i]=Ō(b Ƨ i),yo[kPad+i]=ñ(y[kPad+i],32);}
#pragma GCC unroll 17
Ć(Ŀ<=D ų)co ˉ ñ(Ō(c Ƨ j),32),dq ˉ ñ(Ō(d Ƨ j),32)ø(Õ t0 Ē t0<2*D;t0+=B){н[B][kSums]={},ȇ[B][kSums]={}Ȣ ù=t0+1>D?t0+1-D:0,ȣ Ч(D,t0+B-1)ø(ʍ=ù;j<=ȣ ų){ġ qd=Ō(d Ƨ j),qc=Ō(c Ƨ j);
#pragma GCC unroll 8
Ć(Õ s Ē s<B ė s){Ģ i=kPad+t0+s-j;Ŧ[s]Г ð(Ŧ[s]Ŭ,ß(x Ĕ,qd));ȇ[s]Г ð(ȇ[s]Ŭ,ß(xo Ĕ,dq Ƃ));Ŧ[s][kSums-1]=ð(Ŧ[s][kSums-1],ß(y Ĕ,qc));ȇ[s][kSums-1]=ð(ȇ[s][kSums-1],ß(yo Ĕ,co Ƃ));}}
#pragma GCC unroll 8
Ć(Õ s Ē s<B ė s){Ģ t=t0+s,ơ=2*(ĥ(t,D-1)+1-(t>D?t-D:0))ĺ(t<2*D)Š(į ϛ,cross_sum(Ŧ[s],ȇ[s],ơ));}}}Ą<Õ D>ˮ Ҍ(Һ,Ҡ,Ɣ c,Ɣ d,Ȩ){Ə(2*D<=17);ɍ[D],xo[D],y[D],yo[D],p[D+1],po[D+1],q[D+1],qo[D+1];
#pragma GCC unroll 8
Ć(Ů D Į){x ƈ Ō(ϋ i),xo ƈ ñ(x Ĕ,32);y ƈ Ō(b Ƨ i),yo ƈ ñ(y Ĕ,32);}
#pragma GCC unroll 9
Ć(Ŀ<=D ų){p ˉ Ō(c Ƨ j),po ˉ ñ(p Ƃ,32);q ˉ Ō(d Ƨ j),qo ˉ ñ(q Ƃ,32);}
#pragma GCC unroll 16
Ć(Ӄ<2*D ė t){Ģ ù=t>D?t-D:0,ȣ Ч(t,D-1);н=đ,ȇ=đ;
#pragma GCC unroll 8
Ć(ɮ=ù;i<=ȣ Į){Ŧ=ð(Ŧ,ð(ß(x Ĕ,q[t-i]),ß(y Ĕ,p[t-i])));ȇ=ð(ȇ,ð(ß(xo Ĕ,qo[t-i]),ß(yo Ĕ,po[t-i])));}Š(į ϛ,ˍ(Ŧ,ȇ,2*(ȣ+1-ù)));}}Ą<Õ D>Ĉ cross_level(Ɣ Ͽ,Ɣ Ɩ,Ȩ,Õ j3){Ć(Ů j3 Į){Ɣ r=Ͽ Ƨ D*(2*i),*q=Ɩ Ƨ(D+1)*(2*i)Њ(D<=8)Ҍ<D>(r,r Ƨ D,q,q Ƨ(D+1),є 16*D*i)ї Ҍ<D,2>(r,r Ƨ D,q,q Ƨ(D+1),є 16*D*i);}}Ǹ inverse_montgomery(ɍ){ĭ ă=ĕ(kR),ǹ=x ø(Ö e=ӓ;e;e>>=1){if(e&1)ă=Ŕ(ă,ǹ);ǹ=Ŕ(ǹ,ǹ);}Ҥ}Ũ ҋ:private BaseTree{Ⱥ ҋ(Ù ѣ&n2,Ɣ y,Ό):BaseTree(n2,ϳ),y_(y),m_(m){}Ɣ run(І,п,Ɣ w){Ģ þ=hi-lo;lo_=lo ĺ(þ у){Š(Ϭ,Ō(w));ş(1)Ū Ϭ;}if(ϧ(þ))Ú run_levels(þ,w);Ӵ ş(þ);ascend(1,lo,hi,sums_)Ū sums_;}Ⱦ Ɣ run_levels(ȡ,Ɣ w){Ɣ Ɩ[6];Ņ at=Ӟ(lo_,þ,Ɩ);ǚ buffers ǥ={at,at Ƨ þ};descend_levels(lo_,þ,w,Ɩ,buffers);ş(þ);Ɣ Ͽ=Ϭ ø(Õ d=1,j Ē d<þ;d*=2,++j){ǚ į=2*d==þ?sums_:buffers[j&1];with_degree(d,ѯ<Õ D>(){cross_level<D>(Ͽ,Ɩ Ƃ,į,þ/(2*D));});Ͽ=į;}Ú sums_;}Ĉ ascend(Õ v,І,п,Ȩ){if(hi-lo у)Ú Š(į,Ō(Ϭ Ƨ(lo-lo_)))ͷ ascend(2*v,lo,ρ,ω);ascend(2*v+1,ρ,hi,l8);Ҍ(ω,ρ-lo,l8,hi-ρ,ΰ[2*v],ΰ[2*v+1],į);free_=ω;}Ĉ ş(ȡ){ɿ ɡ=4 ι iota=ɜ,one=ĕ(kR)Ȣ f5=(þ+ɡ-1)/ɡ;ĭ ʜ[kMaxSlots]ø(Õ g Ē
 g<f5 ė g){
#pragma GCC unroll 4
Ć(ϩ){Ģ k=ɡ*g+c,ù=8*(lo_+k);ɍ=one ĺ(k<þ){x=Ō(Ϭ Ƨ k)ĺ(Ў 8>m_){x=_mm256_blendv_epi8(one,x,σ(ĕ(Ö(m_-ù)),iota));Š(Ϭ Ƨ k,x);}}ʜ ѓ g?Ŕ(ʜ[k-ɡ],x):x;}}ġ*Ù ȣ=ʜ+ɡ*(f5-1)ι p01=Ŕ(ȣ Ŭ,ȣ ƹ),p23=Ŕ(ȣ ǥ,ȣ Ѐ)ι ć=inverse_montgomery(Ŕ(p01,p23))ι i01=Ŕ(ć,p23),i23=Ŕ(ć,p01);ĭ e[ɡ]={Ŕ(i01,ȣ ƹ),Ŕ(i01,ȣ Ŭ),Ŕ(i23,ȣ Ѐ),Ŕ(i23,ȣ ǥ)}ø(Õ g=f5;g-->0;){
#pragma GCC unroll 4
Ć(ϩ){Ģ k=ɡ*g+c ĺ(k>=þ)ӈ ι x=Ō(Ϭ Ƨ k),l4=g?Ŕ(e[c],ʜ[k-ɡ]):e[c];e[c]=Ŕ(e[c],x);Š(Ϭ Ƨ k,ů(Ŕ(l4,Ō(y_ Ƨ(lo_+k)))));}}}ċ ɿ kMaxSlots=Ǔ kBase;ћ ϳ[8*kMaxSlots];ћ sums_[8*kMaxSlots];Ɣ y_;Õ m_;};}Ũ ϫ:public ʨ{Ⱥ ϫ(Ě&ŋ,ĝ k5):ʨ(ŋ,k5),m_(Ј){}ċ Õ Ƅ(Ό){б,h5=Ǔ î(Ö(m8/8))Ȣ ď=ʘ(8,h5),o3=Ϡ Ū descend_words(m)+10 ӌ(8 Ԍ)+6 ӌ(o3)+4096;}Ĉ Ҋ(ĝ w,ĝ y,Ĝ r,Є){Ɵ;Stack Ǚ(ĸ)Ȣ h5=Ǔ î(Ö(Ȑ)),ď=ʘ(8,h5)Ĩ LaneState ˀ=lane_state(w,kR,Ǚ);ҋ ƻ(slots_,y â,m_);ǚ Ͽ=Л(8 Ԍ)ĺ(Ȑ<=Ǔ kBase){ĵ(ƻ.run(0,Ȑ,ˀ.state Ƨ(h5-Ȑ)),8*Ȑ,Ͽ);}ŝ{т(0,Ȑ,ˀ.state,ˀ.u,Ǚ,ƻ,Ͽ,h5);t_.ć({Ͽ,8*h5},{Ͽ,8*h5});}ǚ İ=ˀ.state ø(ǈ Ȑ Ӕ){ĭ v Ҹ(Ů 8 Į)v ƈ Ō(Ͽ Ƨ(j+i));transpose8(v)ø(Õ l Ē l<8 ė l)Š(İ+l Ԍ+j,v[l]);}Ģ î=ǖ::î(Ö(m8_));top(0,8,İ,ď,Ǚ,r â,î);t_.ć(r Ȓ(î),r Ȓ(î));}Ⱦ Ĉ т(І,п,Ɣ s,Ö u,Ѿ,ä ҋ&ƻ,Ȩ,Õ ɖ){Ɵ Ĩ LaneTree&ǉ=*lanes_ Ȣ î=Ǔ î(Ö(ǉ.Ѝ)),č=8*î Ȣ Χ,в Ɣ Ù n7 ǥ={ҍ.pair(ρ),ҍ.pair(ρ)Я(č)};ǚ Ͽ ǥ={ɖ>=2*î?є č:į,Л(č)}ȶ{Ģ xlo=n1[side]Ŭ,xhi=n1[side]ƹ,ũ=ǉ.ũ(xlo,xhi)Ȣ ʎ=8*Ǔ î(Ö(ũ)),g5=Ǚ.mark();ǚ p=Л(č);Ȥ::Ѥ(s,n7[1-side],{p,č})ĺ(xhi-xlo у||ũ<=Ǔ kBase){ļ(p,č,u);Ɣ Ù ʂ=ƻ.run(xlo,xhi,p+č-
8*ũ);t_.ı({ʂ,8*ũ},0,{Ͽ[side],č});}м(2*ʎ==č){halve(p,č);т(xlo,xhi,p,Ơ(u),Ǚ,ƻ,Ͽ[side],î);}ŝ{х т(xlo,xhi,q,1,Ǚ,ƻ,Ͽ[side],î);}Ǚ.release(g5);}Ȥ::pointwise_product_sums(Ͽ Ŭ,n7 ƹ,Ͽ ƹ,n7 Ŭ,{į,č});double_to(į,č,ɖ/î,Ǚ);Ǚ Ȏ;}Ĉ top(І,п,Ɣ İ,Õ ď,Ѿ,Ȩ,Õ ɖ){Ù TopTree&ǉ=*top_ ĺ(hi-lo у)Ú t_.ı({İ+lo Ԍ,Ȑ},0,{į,ɖ})Ȣ î=ǖ::î(ǉ.Ѝ),Χ,в Ɣ Ù n7 ǥ={Ѣ.pair(ρ),Ѣ.pair(ρ)Я(î)};ǚ Ͽ ǥ={ɖ>=2*î?є î:į,Л(î)};top(lo,ρ,İ,ď,Ǚ,Ͽ Ŭ,î);top(ρ,hi,İ,ď,Ǚ,Ͽ ƹ,î);t_.leaf_product_sums(Ͽ Ŭ,n7 ƹ,Ͽ ƹ,n7 Ŭ,{į,î});double_to(į,î,ɖ/î,Ǚ);Ǚ Ȏ;}Ĉ double_to(Ȩ,Õ č,Õ Ÿ,Ѿ)Ù{if(Ÿ у)Ú;ǚ c=Л(č);t_.ć({į,č},{c,č})ø(ƿ=č;n<Ÿ*č;n*=2)t_.ɇ({c,č},0,{є n,n});}Õ m_;};Ί interpolate_words(Ό){б Ĩ ä Division ȴ(m,m8)Ū ʨ Ư(m)+Č Ư(ȴ.log())Я(Ŋ(m+1,m8))Я(Ϡ)Я(Ŋ(ȴ.Ƅ(),ϫ::Ƅ(m)));}í Ҋ(Ě&ŋ,ĝ k5,ĝ Ƴ,Ĝ c){Ģ m=Ј;ϫ ǉ(ŋ,k5)Ȣ m8=ǉ Û,î=Ϡ ǫ y ф(Ŋ(m+1,m8))Ӎ(ǉ.Ŏ()ü,ǉ.Ŏ()ʊ(m+1),y ü);Ǟ(y Ȓ(m+1),y ʴ)Ĩ ä Division ȴ(m,m8)Ĩ Č t(ŋ,ȴ.log())ǫ Ļ ф(î)ǫ ĸ ф(Ŋ(ȴ.Ƅ(),ϫ::Ƅ(m)));ä divide(t,ȴ,y ʴ,ǉ.Ŏ(),ǉ.product_transform(),Ļ Ȓ(m8),ĸ);Ȫ(Ƴ ü,Ƴ ƃ,y ü);y[m]Ē ǉ.Ҋ(Ļ Ȓ(m8),y Ȓ(m8),Ļ,ĸ)Ӎ(Ļ ü,Ļ ʊ(m),c ü);}}ā ˎ
{í import_words(ˢ in,ɐ){Ɵ;ɮ Ɠ(ʲ in Û ƞ){Ҽ Ž(õ Ɣ>(in ȝ));Š(į ȝ,ů(Ŕ(x,ĕ(1))));}Ć(ϼ in Û Į)į ƈ in Ĕ Ȅ;}í import_montgomery(ˢ in,ɐ){Ć(Ů in Û Į){Ù ī x=Ө;į ƈ ĥ(x,x-Ƀ);}}ò ƶ export_words(ĝ in){ƶ į;ž(į,in â,in Û)Ū į;}ò ƶ native_evaluate(ˢ f,ˢ k5){Ě ŋ(evaluate_words(Ǳ,Ј)Я(Ǳ)ʳ à(Ј))ѵ a ф(Ǳ),x ф(Ј),y ф(Ј);import_montgomery(f,a);import_words(k5,x);evaluate(ŋ,a,x,y)Ū export_words(y);}ò ƶ native_interpolate(ˢ k5,ˢ Ƴ){Ģ n=Ј й(interpolate_words(n)+3 ӌ(n))ѵ x к,y к,c к;import_words(k5,x);import_montgomery(Ƴ,y);Ҋ(ŋ,x,y,c)Ū export_words(c);}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ Ҋ(Ù џ&xs,Ù џ&ys){Ѽ ͼ ƻ=џ;ƙ ӹ(Ҋ Ӹ(ҹ(xs),ҹ(ys)));Ƣ(Ԇ==ys Û)Ĩ ǧ=ó(Ԇ)ĺ(n ε)Ú{};
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(n>=64&&n ɱ ȏ)Ú F(Ü ˎ::native_interpolate(xs,ys));}
#endif
è ý ŀ=8 Э Ȋ î=æ*ŀ;F ˀ(î)ø(Е)ˀ[î-n+i]=T(n-i)*ǉ ƹ Ĕ ѵ flip=ѯ{Ć(κ Ё Į){Ø ˁ(ǉ Ĕ ü,ǉ Ĕ ƃ);ƻ&a=ƕ Ĕ ĺ(a ű)ӈ Ȋ î=ó(ư),d=ó(ǉ Ĕ Û)-1 ø(ý j=2;j<î;j<<=1)Ø ˁ(a ʷ j,a ü ʳ j)ĺ(d==î/2){Ć(ý j=î/2;j<î ų)a ˉ-a Ƃ;}ŝ{ʦ&ķ=ǅ ntt_roots(î);ƻ phase(î,Ӽ)ø(ý Ơ=1;Ơ<î;Ơ<<=1){Ù T ɰ=ķ[Ơ].pow(d)ø(Ȕ Ơ ų)phase[Ơ+j]=phase Ƃ*ɰ;}Ć(Ȕ î ų)a Ƃ*=phase Ƃ;}}};ˀ=ˀ.Α(ǉ ƹ,î);ƻ den=Ü Ώ<T>::evaluate(xs,ǉ,ƕ,æ,ŀ,Ɔ(ˀ));flip();Ħ F>Ĵ(Ё);ƻ ş(n);ǅ batch_inverse(ş,den);ǅ Ȉ(ş,ş,ys,n)ø(ý i Ē i*ŀ<n Į){Ƞ d Ч(ŀ,n-i*ŀ);F&f=Ĵ[æ+i];f Ř(d)Ĩ F&q=ǉ[æ+i];
#if defined(__x86_64__) || defined(__i386__)
Ġ{if(d==8 ȏ){Ű interpolate_leaf8(Ȯ(f),Ȯ(q),Ȯ(xs)+i*ŀ,Ȯ(ş)+i*ŀ);ӈ;}}
#endif
Ć(ý j=i*ŀ;j<i*ŀ+d ų){T ʑ=1;f[d-1]+=ş Ƃ ø(ң=d-2;k и k){ʑ=q[k+1]+xs Ƃ*ʑ;f Ƭ+=ş Ƃ*ʑ;}}}Ć(Ż æ-1;i>=1;--i){if(Ĵ Ʊ ű){Ĵ ƈ Ɔ(Ĵ ɔ);ӈ;}Ƞ î=ó(ƕ ɔ Û)ĺ(î&&ó(ƕ Ʊ Û)==î){ƻ a=Ɔ(Ĵ ɔ),b=Ɔ(Ĵ Ʊ);a Ř(î);b Ř(î);Ӗ(a);Ӗ(b);Ġ Ű җ(Ȯ(a),Ȯ(b),Ȯ(ƕ ɔ),Ȯ(ƕ Ʊ),î)ї Ć(Ȕ î ų)a ˉ a Ƃ*ƕ Ʊ Ƃ+b Ƃ*ƕ ɔ Ƃ;ǅ intt(a);a Ř(ǉ Ĕ Û-1);Ĵ ƈ Ɔ(a);}ŝ{Ĵ ƈ Ĵ ɔ*ǉ Ʊ+Ĵ Ʊ*ǉ ɔ;}}Ú Ɔ(Ĵ ƹ);}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ T linear_recurrence(Ù џ&g4,Ù џ&ļ,Ǉ k){Ѽ;ƙ;Ƣ(k>=0&&g4 Û==ļ Û)Ȋ d=ó(g4 Û)ĺ(k<d)Ú g4 Ƭ;ý с=d ѵ zero=[](T x){Ġ Ú x Ү ї Ú x Ȅ ε;}ǟ(с&&zero(ļ[с-1]))--с ĺ(с ε)Ú 0;ƀ{ӆ ǅ W a(с),c(с)ø(ö с Į){ө Ü Ī θ(g4[d-с+i]Ȅ);c ƈ Ü Ī θ(ļ Ĕ Ȅ);}Ú T θ(linear_recurrence Ӹ(a,c,k-d+с)Ȅ);}if(с у)Ú g4[d-1]*ļ Ŭ.pow(k-d+1);F q(с+1);q Г 1 ø(ö с Į)q Υ=-ļ Ĕ;F p=F::Ź(F(g4 ƃ-с,g4 ƃ),q).pre(с)Ū bostan_mori(Ɔ(p),Ɔ(q),k-d+с);}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
Ӏ{
#if defined(__x86_64__) || defined(__i386__)
Ý í ԅ(Ҿ ˠ,Ù Ҿ f,ƿ,Ù Ҿ k5,Ό){Ę I=ę I8 ø(Ů m Ƽ 32){Ť ę U Η[32]={},į[32]Ȣ þ Ч<Õ>(32,m-i)ĺ(þ<8){Ć(ѿ þ Ȟ){Ī Ĵ=0,x=һ(k5[i+k])ø(ʍ=n;j-->0;)Ĵ=Ĵ*x+һ(f Ƃ);ˠ[i+k]=Ĵ.v;}ϵ}Ù Ҿ xs=k5+i ĺ(þ<32){ĵ(xs,þ,Η);xs=Η;}I x0=ę œ(xs),x1=ę œ(xs+8);I x2=ę œ(xs ү),x3=ę œ(xs+24);I a0=đ,a1=a0,a2=a0,a3=a0 ø(ʍ=n;j-->0;){I c ʖ(f Ƃ);a0=ę ɾ(ę ɏ(a0,x0),c);a1=ę ɾ(ę ɏ(a1,x1),c);a2=ę ɾ(ę ɏ(a2,x2),c);a3=ę ɾ(ę ɏ(a3,x3),c);}Ҿ ǁ=þ==32?Ҷ:į;ę ŗ(ǁ,a0);ę ŗ(ǁ+8,a1);ę ŗ(ǁ ү,a2);ę ŗ(ǁ+24,a3)ĺ(þ<32)ĵ(į,þ,Ҷ);}}
#endif
}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ џ multipoint_eval(Ù Ȼ&f,Ù џ&xs){Ѽ ͼ W=Ď Ӹ ͼ ƻ=џ;ƙ ӹ(W(multipoint_eval Ӹ(ҹ(f),ҹ(xs))))ĺ(xs ű)Ú{}ĺ(Ǳ<=32||Ԇ<=32){
#if defined(__x86_64__) || defined(__i386__)
Ġ{if(((Ԇ>=8&&Ǳ>=8&&(Ԇ>=32||Ǳ>=32))||(Ԇ>=4096&&Ǳ>=4))ȏ){ƻ r(Ԇ);Ü ԅ(Ȯ(r),Ȯ(f),Ǳ,Ȯ(xs),Ԇ)Έ}}
#endif
ƻ r;r.reserve(Ԇ)ø(T x:xs)r Ӯ(f.eval(x))Έ}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(Ǳ*Ԇ>(Ǽ 22)&&Ŋ(Ǳ,Ԇ)ɱ ȏ)Ú Ü ˎ::native_evaluate(f,xs);}
#endif
è ý ŀ=8;ή=ŀ ǟ(î<(ý)Ŋ(Ǳ,Ԇ))î<<=1;Ƣ(î ɱ)Э;F ˀ=f;ˀ Ř(î);Ø ˁ(ˀ ü,ˀ ƃ);ˀ=ˀ.Α(ǉ ƹ,î)ĺ(æ*ŀ<î)ˀ.erase(ˀ ü,ˀ ƃ-æ*ŀ)Ū Ü Ώ<T>::evaluate(xs,ǉ,ƕ,æ,ŀ,Ɔ(ˀ),Ƈ);}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
Ӏ{Ɯ Ȭ{Ę T=Ī ͼ F=Ȼ ͼ O=Ǆ;ċ è ý Ӝ=64;Ɯ ǋ{ǜ F,4>a{F{1},F{},F{},F{1}};mutable ǜ F,4>Ǥ;ý ƍ Ē Ǝ ȭ=Ƈ;};ċ F high(ʧ a,ң){Ú k>=ѡ?F{}:F(a ʷ k,a ƃ);}ċ F ё(ʧ a,ǧ){if(ѡ<=n){F ă=a;ă Ř(n)ȧ}F ă(n)ø(ö ѡ Į)ă[i&(Ԕ)]+=ɨ ȧ}ċ ʧ Ǩ(Ù ǋ&š,ý ȯ,ǧ){F&Ǥ=š.Ǥ[ȯ]ĺ(Ǥ ű){Ǥ=ё(Ή[ȯ],n);Ӗ(Ǥ);}while(ý(Ǥ Û)<n){Ƞ Ơ=Ǥ Û;F ȇ(Ơ)Ĩ F&p=Ή[ȯ]ø(ö ý(p Û)Į)ȇ[i&(Ơ-1)]+=(i&Ơ)?-p Ĕ:p Ĕ;ǅ ɘ(ȇ,Ӽ,ǅ ntt_roots(2*Ơ)[Ơ]);Ӗ(ȇ);Ǥ ӭ(Ǥ ƃ,ȇ ü,ȇ ƃ);}Ú Ǥ;}ċ F ҁ(ʧ a,ʧ b,ʧ c,ʧ d,ǧ,F*Ǥ=Ƿ){F ă(a ü,a ʷ n);ę җ(Ȯ(ă),Ȯ(c),Ȯ(d),Ȯ(b),n)ĺ(Ǥ)*Ǥ=ă;ǅ intt(ă),ɞ(ă)ȧ}ċ ǋ multiply_column(Ù ǋ&a,Ù ǋ&b){Қ,ă.ȭ=Ś ĺ(ă.ƍ<=64){Ć(ъ){ă.a ɔ=a.a ɔ*b.a Ŭ+a.a Ʊ*b.a ǥ;ɞ(ă.a ɔ);}Ҥ}ǧ=1 ǟ(n<=ă.ƍ)n<<=1;ǜ Ù F*,4>x{},y{}ɫ{x ƈ&Ǩ(a,i,n)ĺ(!(i&1))y ƈ&Ǩ(b,i,n);}Ć(ъ)ă.a ɔ=ҁ(*x ɔ,*y Ŭ,*x Ʊ,*y ǥ,n,&ă.Ǥ ɔ)ȧ}Ŵ step_column(ǋ&š,ʧ q){Ć(ł&Ǥ:š.Ǥ)Ǥ.clear();F next=Ή Ŭ-q*Ή ǥ;ɞ(next);Ή Г Ɔ(Ή ǥ),Ή ǥ=Ɔ(next);š.ƍ+=Ŋ(0,ý(Ѹ)-1),š.ȭ=Ś;}ċ ǋ Ź(Ù ǋ&a,Ù ǋ&b,Ǝ e3=Ś){Қ,ă.ȭ=Ś ĺ(e3)ă.a Ѐ.clear()Ȋ φ=e3?1:2 ĺ(ă.ƍ<=64){Ć(ö φ Į)Ć(Ϟ){ă.a Й=a.a ɔ*b.a Ƃ+a.a Ʊ*b.a[2+j];ɞ(ă.a Й);}Ҥ}ǧ=1 ǟ(n<=ă.ƍ)n<<=1;ǜ Ù F*,4>x{},y{}ɫ{if(i<2*φ)x ƈ&Ǩ
(a,i,n);y ƈ&Ǩ(b,i,n);}Ć(ö φ Į)Ć(Ϟ)ă.a Й=ҁ(*x ɔ,*y Ƃ,*x Ʊ,*y[2+j],n,&ă.Ǥ Й)ȧ}Ŵ ɰ(ǋ&š,ʧ q){Ć(ł&Ǥ:š.Ǥ)Ǥ.clear();F c=Ή Ŭ-q*Ή ǥ,d=Ή ƹ-q*Ή Ѐ;ɞ(c),ɞ(d);Ή Г Ɔ(Ή ǥ),Ή ƹ=Ɔ(Ή Ѐ);Ή ǥ=Ɔ(c),Ή Ѐ=Ɔ(d);š.ƍ+=Ŋ(0,ý(Ѹ)-1),š.ȭ=Ś;}Ŵ apply(Ù ǋ&š,F&a,F&b){if(š.ȭ)Ú ĺ(ư<=128){F c=Ή Ŭ*a+Ή ƹ*b;b=Ή ǥ*a+Ή Ѐ*b,a=Ɔ(c);ɞ(a),ɞ(b)а}ý æ=ѡ-š.ƍ,n=1;Ƣ(æ>0)ǟ(n<æ)n<<=1;F x=ё(a,n),y=ё(b,n);Ӗ(x),Ӗ(y);ǜ Ù F*,4>m ɫ m ƈ&Ǩ(š,i,n);a=ҁ(*m Ŭ,x,*m ƹ,y,n);b=ҁ(*m ǥ,x,*m Ѐ,y,n);Ƣ(ѡ==æ&&ζ<ư);}ċ F apply_first(Ù ǋ&š,ʧ a,ʧ b){if(š.ȭ)Ú a Ȋ æ=ѡ-š.ƍ ĺ(æ у){T Ĵ=(Ή Ŭ ű||a ű?ȓ:Ή Ŭ Ŭ*a Ŭ)+(Ή ƹ ű||b ű?ȓ:Ή ƹ Ŭ*b Ŭ)Ū Ĵ Ү?F{}:F{Ĵ};}if(ư<=128){F ă=Ή Ŭ*a+Ή ƹ*b;ɞ(ă)ȧ}ǧ=1 ǟ(n<æ)n<<=1;F x=ё(a,n),y=ё(b,n);Ӗ(x),Ӗ(y)Ū ҁ(Ǩ(š,0,n),x,Ǩ(š,1,n),y,n);}Ŵ җ(T*į,Ù T*a,Ù T*b,ǧ,T x,T y,T z){
#if defined(__x86_64__) || defined(__i386__)
if(n>=8&&ę ě){combine8(į,a,b,n,x Ȅ,y Ȅ,z Ȅ)а}
#endif
Ć(Е)į ƈ x*ɨ+y*b[i-1]+z*ϸ;}
#if defined(__x86_64__) || defined(__i386__)
ˌ combine8(T*į,Ù T*a,Ù T*b,ǧ,ī x,ī y,ī z){Ę I=ę I8;I vx ʖ(x),qx ʖ(ę ť(x));I vy ʖ(y),qy ʖ(ę ť(y));I vz ʖ(z),qz ʖ(ę ť(z));К{I u=ę Ʀ(ę œ(õ Ù ī*>(a+i)),vx,qx);I v=ę Ʀ(ę œ(õ Ù ī*>(b+i-1)),vy,qy);I w=ę Ʀ(ę œ(õ Ù ī*>(b+i)),vz,qz);ę ŗ(õ ī*>(є i),ę ɾ(u,ę ɾ(v,w)));}T tx=T θ(x),ty=T θ(y),tz=T θ(z)ø(Ȝ)į ƈ tx*ɨ+ty*b[i-1]+tz*ϸ;}
#endif
ċ ǋ ӣ(ʧ a,ʧ b,ý ƪ=-1){è ý δ=2*Ӝ+8;Ƣ(ư<=2*Ӝ+1);Ť T k6 Ѐ[δ]={},φ Ѐ ǥ[δ]={};T*c=k6 Ŭ+1,*d=k6 ƹ+1,*e=k6 ǥ+1;T*ν Ѐ ǥ;ý ȴ Ѐ ǥ={{1,0},{0,1},{0,0}}ø(ö 3 Į)Ć(Ϟ)ν Ĕ ˉ φ Ĕ Ƃ+1;Ȫ(a ü,a ƃ,c);Ȫ(b ü,b ƃ,d);ν Ŭ Ŭ Г ν ƹ ƹ Г 1 Ĩ ǧ=ư ĺ(ƪ<0)ƪ=n/2;ý cd=Ԕ,dd=ý(ζ)-1;ý e4=2*ƪ-(Ԕ);T m5=1,m6=1 ǟ(dd>=ƪ){Ƞ d7=cd-dd;e4+=d7 Ȋ ʗ=Ŋ(0,e4)Ĩ T a9=d[dd];T m7=m5 ĺ(d7 у){Ù T x=a9*a9,y=-a9*c[cd]Ĩ T z=c[cd]*d[dd-1]-a9*c[cd-1];җ(e+ʗ,c+ʗ,d+ʗ,dd-ʗ,x,y,z);m7*=x ø(Ϟ){Ƞ î=Ŋ(ȴ Ŭ Ƃ,ȴ ƹ Ƃ?ȴ ƹ Ƃ+1:0);җ(ј,ν Ŭ Ƃ,ν ƹ Ƃ,î,x,y,z);ż(ј+î,ј+Ŋ(î,ȴ ǥ Ƃ),ȓ);ȴ ǥ ˉ î;}}ŝ{Ȫ(c+ʗ,c+cd+1,e+ʗ)ø(Ϟ){ĵ(ν Ŭ Ƃ,ȴ Ŭ Ƃ,ј);ż(ј+ȴ Ŭ Ƃ,ј+Ŋ(ȴ Ŭ Ƃ,ȴ ǥ Ƃ),ȓ);ȴ ǥ ˉ ȴ Ŭ Ƃ;}Ć(ý ƍ=d7;ƍ и ƍ){Ù T Ÿ=e[dd+ƍ]ø(Ż ʗ;i<=dd+ƍ Į)e ƈ a9*e Ĕ-(i>=ƍ?Ÿ*d[i-ƍ]:ȓ)ø(Ϟ){Ƞ î=Ŋ(ȴ ǥ Ƃ,ȴ ƹ Ƃ?ȴ ƹ Ƃ+ƍ:0)ø(ö î Į)ј ƈ a9*ј Ĕ-(i>=ƍ?Ÿ*ν ƹ Ƃ[i-ƍ]:ȓ);ȴ ǥ ˉ î;}m7*=a9;}}ь(c,d),ь(d,e)ø(Ϟ){ь(ν Ŭ Ƃ,ν ƹ Ƃ),ь(ν ƹ Ƃ,ј);ь(ȴ Ŭ Ƃ,ȴ ƹ Ƃ),ь(ȴ ƹ Ƃ,ȴ ǥ Ƃ);}m5=m6,m6=m7;cd=dd,dd=cd-1 ǟ(dd>=ƪ&&d[dd]Ү)--dd;}ǋ ă;ă.ƍ=Ԕ-cd,ă.ȭ=ă.ƍ ε;T ć=(m5*m6)ˆ;T e2 ǥ={ć*m6,ć*m5}ø(ъ)Ć(Ϟ){ă.a Й=F(ν Ĕ Ƃ,ν Ĕ Ƃ+ȴ Ĕ Ƃ);ǅ Ζ(ă.a Й,e2 Ĕ);ɞ(ă.a Й);
}Ҥ}ċ ǋ jump(F a,F b,ң,Ǝ e3=Ś){Ù ǧ=ѡ-1,ƪ=n-k;Ƣ(ư>ζ&&0<=k&&k<=n)ĺ(ý(ζ)<=ƪ)Ú ǋ{}ĺ(n>Ӷ)Ú jump(high(a,n-Ӷ),high(b,n-Ӷ),k,e3)ĺ(k<=Ӝ)Ú ӣ(a,b,ƪ);ý ù=1 ǟ(2*ù<k)ù*=2 Ȋ cut=Ŋ(0,n-2*ù);ǋ ω=jump(high(a,cut),high(b,cut),ù)ĺ(ω.ȭ){ł[q,r]=a.divmod(b);ɰ(ω,q),a=Ɔ(b),b=Ɔ(r);}ŝ apply(ω,a,b)ĺ(ý(ζ)<=ƪ)Ú ω Ȋ χ=ѡ-1-ƪ Ȋ skip=Ŋ(0,ѡ-1-2*χ)Ū Ź(jump(high(a,skip),high(b,skip),χ,e3),ω,e3);}ċ ǋ Ơ(F a,F b){Ƣ(ư>ζ);ý m=ѡ/2;ǋ ă ĺ(ý(ζ)<=m)Ú ă ĺ(ư<=2*Ӝ)Ú ӣ(a,b);ă=Ơ(high(a,m),high(b,m));apply(ă,a,b)ĺ(ý(ζ)<=m)Ú ă ѵ[q,r]=a.divmod(b);ɰ(ă,q),a=Ɔ(b),b=Ɔ(r)ĺ(ý(ζ)<=m)Ҥ ң=2*m+1-ѡ Ū Ź(Ơ(high(a,k),high(b,k)),ă);}};}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
ā Ü ē{ā ӛ{Ɵ ͼ ƺ::ё ͼ ƺ::ʫ ͼ ƺ::ґ ͼ ƺ::kLongDivision;Ũ Stack{Ϳ Stack(Ĝ i6):base_(i6 â),ͽ{Ģ ӝ=à(n)ĺ(ӝ>size_-top_)throw Ø bad_alloc();ǚ p=base_+top_;top_+=ӝ Ū p;}Ⱦ Ņ base_;Õ size_,top_ Ē};ԃ Ҁ(Ö x,Ö y){Ú Ö(ŉ(x)*y Ұ);}ԃ mod_sub(Ö x,Ö y){Ú x>=y?x-y:x+Ͼ y;}ԃ mod_negate(Ö x){Ú x?Ͼ x:0;}ԃ mod_inverse(Ö x){Ú ã ǹ(x,ӓ);}í combine3(Ɣ u,Ɣ v,Ȩ,ř ʝ,ř to,Ö x,Ö y,Ö z){ġ xs=ĕ(x),ys=ĕ(y),zs=ĕ(z)ø(ř j=ʝ&~ř(7);j<to Ӕ){ġ a=Ž(u+j),b=Ž(v+j-1),c=Ž(v+j)ι Ŧ=ð(ð(ß(a,xs),ß(b,ys)),ß(c,zs))ι ȇ=ð(ð(ß(ñ(a,32),xs),ß(ñ(b,32),ys)),ß(ñ(c,32),zs));ū(є j,ń(ґ(Ŧ,ȇ),kP));}}ˮ fill_pair(М,Ɣ x0,Ɣ x1,Õ g,ʅ(&Ļ)ǥ ɛ){Ģ at=32*g ι a0 ɛ={Ō(x0+at),Ō(x0+at+8),Ō(x0+at ү),Ō(x0+at+24)}ι a1 ɛ={Ō(x1+at),Ō(x1+at+8),Ō(x1+at ү),Ō(x1+at+24)};ʿ(Ļ Ŭ,a0,ķ,g);ʿ(Ļ ƹ,a1,ķ,g);}Ą<Õ K>í Ɩ(М,Ɣ x0,Ɣ x1,Ɣ Ù(&y_in)[K]ǥ,ǚ(&j6)[K],ƿ){Ɣ y[K]ǥ;Ȩ[K]ø(ѿ K Ȟ)y Ƭ Г y_in Ƭ Ŭ,y Ƭ ƹ=y_in Ƭ ƹ,į ѓ j6 Ƭ Ƥ μ(kR);ɥ ǥ ɛ;fill_pair(ķ,x0,x1,0,Ļ Ŭ)ø(Õ g Ē g<n/32 ė g){if(g+1<n/32)fill_pair(ķ,x0,x1,g+1,Ļ[(g+1)&1])Ĩ ʅ(&w)ǥ ɛ=Ļ[g&1]Ȣ at=32*g;
#pragma GCC unroll 4
Κ Ć(ѿ K Ȟ){ġ s ӏ(ƫ(ǽ(w Ŭ Ѱ,y Ƭ Ŭ+at ϛ),ǽ(w ƹ Ѱ,y Ƭ ƹ+at ϛ)));Š(į Ƭ+at ϛ,ń(ŭ(s,μ),kP));}}}Ɯ ǋ{Ņ ʮ ɛ;Õ æ ɛ;Ö at0 ɛ;Ņ Ɓ ɛ;Õ î ɛ;Õ room,progress;};Ũ HalfGcd{Ⱥ ċ è ř kDirect=64;ċ ý ɶ(Õ i9){Ú Ŋ(ʵ,ý(ʌ(ʘ(i9,2)-1)));}ċ Õ Ƅ(Õ i9){Ú 48*(Ǽ ɶ(i9))+4096;}ċ Õ č(Õ i9){Ú Č Ư(ɶ(i9))Я(Ƅ(i9));}HalfGcd(Ě&ŋ,Õ i9,ř ӣ=kDirect):Ǫ(ŋ,ɶ(i9)),ʄ(Ĭ(Ƅ(i9))),direct_(ӣ){}ǋ š(ȕ,Õ room){ǋ r ɫ{r.ʮ ƈ ś(k+1)â;r.Ɓ ƈ room?ś(room)â:Ƿ;r.î Ĕ Ē}r.room=room Έ}Ĉ jump(Һ,ř n,Ҡ,ř m,ř k,ǋ&į,ī ӝ=0xF){Ù ř ƪ=n-k ĺ(m<ƪ)Ú ȭ(į)ĺ(n>Ӷ){Ù ř s=n-Ӷ Ū jump(a+s,n-s,b+s,m-s,k,į,ӝ);}if(k<=direct_)Ú euclid(a,n,b,m,k,į)Ȣ ѥ,room=ʘ(64,ɬ(Õ(k)))Ĩ ř k1=first_jump(k),s1=Ŋ<ř>(0,n-2*k1);ǋ r1=š(Õ(k1),room);jump(a+s1,n-s1,b+s1,m-s1,k1,r1);ř c4 Ē Ɣ c=Ƿ,*d=Ƿ ĺ(r1 ʱ ε){r1=š(Õ(n-m),room);ǚ ɼ=ś(Õ(n+1))â;divide_step(a,n,b,m,r1,ɼ);c=b,d=ɼ,c4=m;}ŝ{c4=n-ř(r1 ʱ)Ĩ ř s2=Ŋ<ř>(0,2*ƪ-c4);ǚ cs=ś(Õ(c4+1))â,*Ù ds=ś(Õ(c4+1))â;apply(a,n,b,m,r1,c4,s2,cs,ds);c=cs,d=ds;}ř ϔ=c4-1 ǟ(ϔ>=ƪ&&d[ϔ]ε)--ϔ ĺ(ϔ<ƪ){copy(r1,į)Ū ʄ Ȏ;}Ù ř k2=c4-ƪ,s2=Ŋ<ř>(0,c4-2*k2);ǋ r2=š(Õ(k2),room);jump(c+s2,c4-s2
,d+s2,ϔ-s2,k2,r2,(ӝ&3?3u:0u)|(ӝ&12?12u:0u));җ(r2,r1,į,ӝ);ʄ Ȏ;}Ⱦ ċ ř first_jump(ř k){Ģ u=Õ(k)Ū ř(ϧ(u)?u/2:Ø bit_floor(u));}Ĝ ś(ƿ){Ú{ʄ ņ(n),n};}Ņ take_padded(ƿ){ǚ p=ʄ ņ(n ү);ʇ(p,n ү,0)Ū p+8;}Ŵ ȭ(ǋ&į){į.ʮ Ŭ Г į.ʮ Ѐ Г 1;į.æ Г į.æ Ѐ=1;į.æ ƹ=į.æ ǥ Ē į Ӌ Г į Ӌ Ѐ=1,į Ӌ ƹ=į Ӌ ǥ Ɠ(ł&l:į.î)l Ē į ʱ Ē}Ŵ copy(Ù ǋ&ʝ,ǋ&to){Т{ĵ(ʝ.ʮ Ĕ,ʝ.æ Ĕ,to.ʮ Ĕ);to.æ ƈ ʝ.æ Ĕ;to Ӌ ƈ ʝ Ӌ Ĕ;to.î Ĕ Ē}to ʱ=ʝ ʱ;}Ĉ euclid(Һ,ř n,Ҡ,ř m,ř k,ǋ&į){Ģ ѥ Ĩ ř ƪ=n-k;ř e4=n-Ӷ,lo=Ŋ<ř>(0,e4);Ņ p Ѐ ø(ł&x:p)x=take_padded(Õ(n+1));Ȫ(a+lo,a+n+1,p Ŭ+lo);Ȫ(b+lo,b+m+1,p ƹ+lo)Ȣ cap=Õ(k)+8;Ņ ν Ѐ ǥ;Õ m2 Ѐ ǥ={{1,0},{0,1},{0,0}}ø(ł&r:ν)Ć(ł&e:r){e=take_padded(cap);ʇ(e,cap,0);}ν Ŭ Ŭ Г 1,ν ƹ ƹ Г 1;Ņ c=p Ŭ,*d=p ƹ,*e=p ǥ;ř c4=n,ϔ=m ǟ(ϔ>=ƪ){Ù ř d7=c4-ϔ;ğ a9=d[ϔ];e4+=d7;lo=Ŋ<ř>(0,e4)ĺ(d7 у){ğ beta=c[c4];ğ e6=mod_sub(Ҁ(a9,c[c4-1]),Ҁ(beta,d[ϔ-1]));ğ b0=Ҁ(a9,kR);ğ x=Ҁ(a9,b0),y=Ͼ Ҁ(beta,b0),z=mod_negate(Ҁ(e6,kR));combine3(c,d,e,lo,ϔ,x,y,z)ø(ъ){Ģ sc=m2 Ŭ Ĕ,sd=m2 ƹ Ĕ Ȣ se=Ŋ(sc,sd?sd+1:0);combine3(ν Ŭ Ĕ,ν ƹ Ĕ,ҵ,0,ř(se),x,y,z)ĺ(m2 ǥ Ĕ>se)ż(ҵ+se,ҵ+m2 ǥ Ĕ,0);m2 ǥ ƈ se
;}}ŝ{pseudo_divide(c,c4,d,ϔ,e,lo,a9,ν,m2);}ǚ old=c;c=d,d=e,e=old ø(ъ){ь(ν Ŭ Ĕ,ν ƹ Ĕ);ь(ν ƹ Ĕ,ҵ);ь(m2 Ŭ Ĕ,m2 ƹ Ĕ);ь(m2 ƹ Ĕ,m2 ǥ Ĕ);}c4=ϔ;ϔ=c4-1 ǟ(ϔ>=ƪ&&d[ϔ]ε)--ϔ;}Ć(ъ)Ć(Ϟ){ĵ(ν Ĕ Ƃ,m2 Ĕ Ƃ,į.ʮ Й);į.æ Й=m2 Ĕ Ƃ;į Ӌ Й=m2 Ĕ Ƃ?ν Ĕ Ƃ Ŭ:0;į.î Й Ē}į ʱ=Õ(n-c4);ʄ Ȏ;}Ŵ pseudo_divide(Ɣ c,ř c4,Ɣ d,ř ϔ,Ņ e,ř lo,Ö a9,Ņ(&ν)Ѐ ǥ,Õ(&m2)Ѐ ǥ){Ȫ(c+lo,c+c4+1,e+lo)ø(ъ){ĵ(ν Ŭ Ĕ,m2 Ŭ Ĕ,ҵ)ĺ(m2 ǥ Ĕ>m2 Ŭ Ĕ)ż(ҵ+m2 Ŭ Ĕ,ҵ+m2 ǥ Ĕ,0);m2 ǥ ƈ m2 Ŭ Ĕ;}Ć(ř t=c4-ϔ;t и t){ğ i7=mod_negate(e[ϔ+t])ø(ř j=lo;j<=ϔ+t ų){ğ dj=j-t>=0?d[j-t]:0;e ˉ Ö((ŉ(a9)*e Ƃ+ŉ(i7)*dj)Ұ);}Ć(ъ){ǚ re=ҵ;Ɣ Ù rd=ν ƹ Ĕ Ȣ sd=m2 ƹ Ĕ,ƍ=Õ(t)Ȣ se=Ŋ(m2 ǥ Ĕ,sd?sd+ƍ:0)ø(ǈ se ų){ğ dj=j>=ƍ?rd[j-ƍ]:0;re ˉ Ö((ŉ(a9)*re Ƃ+ŉ(i7)*dj)Ұ);}m2 ǥ ƈ se;}}}Ĉ divide_step(Һ,ř n,Ҡ,ř m,ǋ&r,Ņ ɼ){Ģ d7=Õ(n-m);Ņ q=r.ʮ Ѐ ĺ((d7+1)*Õ(m+1)<=kLongDivision){Ȫ(a,a+n+1,ɼ);ğ Μ=mod_inverse(b[m])ø(ɮ=d7+1;i-->0;){ğ qi=Ҁ(ɼ[Õ(m)+i],Μ);q ƈ qi ø(ř t Ē t<=m ė t)ɼ[i+Õ(t)]=mod_sub(ɼ[i+Õ(t)],Ҁ(qi,b Ѱ));}}ŝ{Ù О(a,Õ(n+1)),g(b,Õ(m+1))й(divide_words(Ǳ,ȸ));divide(ŋ,f,g,Ĝ(q,d7+1),Ĝ(ɼ,Õ(m)));ż(ɼ+m,ɼ+n+1,0);}Ć(ɮ Ē i<=
d7 Į)q ƈ mod_negate(q Ĕ);r.ʮ ƹ Г r.ʮ ǥ Г 1;r.æ Г 0,r.æ ƹ=r.æ ǥ=1,r.æ Ѐ=d7+1;r Ӌ Г 0,r Ӌ ƹ=r Ӌ ǥ=1,r Ӌ Ѐ=q Ŭ ø(ł&l:r.î)l Ē r ʱ=d7;}Ĉ transform_of(Ɣ c,Õ æ,ɐ)Ù{Ģ ǯ=ǲ ĺ(æ<=ǯ)Ú Ǫ.ı(ĝ(c,æ),0,į);ё(ĝ(c,æ),į);Ǫ.ı(į);}Ĉ extend(ǋ&r,ý i,Õ ǯ)Ù{if(r.î Ĕ==ǯ)Ú ǫ t(r.Ɓ Ĕ,ǯ)Ȣ Ơ=ǯ/2,æ=r.æ Ĕ ĺ(r.î Ĕ==Ơ&&æ<=Ơ+1){Ù Ĝ ʓ=t Ė(Ơ)ĺ(æ<=Ơ){Ǫ.ɇ(ĝ(r.ʮ Ĕ,æ),0,ʓ);}ŝ{ĵ(r.ʮ Ĕ,Ơ,ʓ â);ʓ Г mod_sub(ʓ Ŭ,r.ʮ Ĕ[Ơ]);Ǫ.ɇ(ʓ,0,ʓ);}}ŝ{transform_of(r.ʮ Ĕ,æ,t);}r.î ƈ ǯ;}Ĉ apply(Һ,ř n,Ҡ,ř m,ǋ&r1,ř c4,ř s2,Ņ c,Ņ d){Ģ ѥ Ȣ ǯ=ʘ(64,ɬ(Õ(n+1)/2))ɫ extend(r1,i,ǯ)ǫ ta ӻ,tb ӻ;transform_of(a,Õ(n+1),ta);transform_of(b,Õ(m+1),tb)ǫ pc ӻ,pd ӻ;apply_products(r1,ta,tb,pc,pd)Ĩ ř l=ř(ǯ),mixed=c4-l ĺ(mixed<0){Ȫ(pc ʷ s2,pc ʷ c4+1,c+s2);Ȫ(pd ʷ s2,pd ʷ c4+1,d+s2)Ū ʄ Ȏ;}Ģ ʗ=Õ(mixed+1)ǫ lc ӻ,ld ӻ Ȣ ѕ=ʘ(64,ɬ(2*ʗ-1))ĺ(ѕ<ǯ){ǋ r=r1 ɫ{r.Ɓ ƈ ś(ѕ)â;Ǫ.ı(ĝ(r1.ʮ Ĕ,ĥ(ʗ,r1.æ Ĕ)),0,Ĝ(r.Ɓ Ĕ,ѕ));}Ù Ĝ sa=ta Ȓ(ѕ),sb=tb Ȓ(ѕ);Ǫ.ı(ĝ(a,ʗ),0,sa);Ǫ.ı(ĝ(b,ĥ(ʗ,Õ(m+1))),0,sb);apply_products(r,sa,sb,lc Ȓ(ѕ),ld Ȓ(ѕ));}ŝ{Ǫ.ı(ĝ(a,ʗ),0,ta);Ǫ.ı(ĝ(b,ĥ(ʗ,Õ(m+1))),0,tb);apply_products(r1,ta,tb
,lc,ld);lc Г Ö((ŉ(r1 Ӌ Ŭ)*a Ŭ+ŉ(r1 Ӌ ƹ)*b Ŭ)Ұ);ld Г Ö((ŉ(r1 Ӌ ǥ)*a Ŭ+ŉ(r1 Ӌ Ѐ)*b Ŭ)Ұ);}Ù ř ù Ч(s2,mixed+1);Ȫ(lc ʷ ù,lc ʷ mixed+1,c+ù);Ȫ(ld ʷ ù,ld ʷ mixed+1,d+ù)Ĩ ř ʝ=Ŋ(s2,mixed+1);Ȫ(pc ʷ ʝ,pc ʷ l,c+ʝ);Ȫ(pd ʷ ʝ,pd ʷ l,d+ʝ);ʫ(pc â,lc â,c+l,Õ(mixed+1));ʫ(pd â,ld â,d+l,Õ(mixed+1));ʄ Ȏ;}Ĉ apply_products(Ù ǋ&r1,ĝ ta,ĝ tb,Ĝ c,Ĝ d)Ù{Ģ ǯ=c Û ǌ t=ѯ(ý i){Ú ĝ(r1.Ɓ Ĕ,ǯ);}Ĩ ɝ pc ǥ={{t(0),ta},{t(1),tb}},pd ǥ={{t(2),ta},{t(3),tb}};Ǫ.Ŗ(pc,c);Ǫ.Ŗ(pd,d);}Ĉ җ(ǋ&r2,ǋ&r1,ǋ&į,ī ӝ){Ģ ѥ Ȣ p=r2 ʱ+r1 ʱ,ǯ=ʘ(64,ɬ(p))ø(ъ)if(ӝ&(3u<<2*i))extend(r2,2*i,ǯ),extend(r2,2*i+1,ǯ)ø(Ϟ)if(ӝ&(5u<<j))extend(r1,j,ǯ),extend(r1,2+j,ǯ)ǫ buffer ӻ Ĩ Ǝ keep=į.room>=ǯ;Ņ t ɛ ɫ t ƈ keep?į.Ɓ Ĕ:ś(ǯ)â ø(ъ){Ɣ x0=r2.Ɓ ɔ,*x1=r2.Ɓ Ʊ;Ɣ Ù y ǥ ǥ={{r1.Ɓ Ŭ,r1.Ɓ ǥ},{r1.Ɓ ƹ,r1.Ɓ Ѐ}}Ĩ ī ν=ӝ>>2*i&3 ĺ(ν==3){ǚ to ǥ={t ɔ,t Ʊ};Ɩ<2>(Ǫ.ķ(),x0,x1,y,to,ǯ);}м(ν){Ƞ j=ν у?0:1;Ɣ Ù yj ƹ ǥ={{y Ƃ Ŭ,y Ƃ ƹ}};ǚ to ƹ={t Й};Ɩ<1>(Ǫ.ķ(),x0,x1,yj,to,ǯ);}}Ć(ъ)Ć(Ϟ){Ƞ ij=2*i+j;į.î[ij]Ē ğ x0=Ö((ŉ(r2 Ӌ ɔ)*r1 Ӌ Ƃ+ŉ(r2 Ӌ Ʊ)*r1 Ӌ[2+j])Ұ);į Ӌ[ij]=x0 ĺ(!(ӝ>>ij&1))ӈ ĺ(keep)į.î[ij]=ǯ;Ǫ.ć(
ĝ(t[ij],ǯ),buffer);ǚ e=į.ʮ[ij]ԕ(buffer â,ĥ(p+1,ǯ),e)ĺ(p==ǯ)e[ǯ]=mod_sub(e Ŭ,x0),e Г x0;Õ æ=p+1 ǟ(æ>0&&e[æ-1]ε)--æ;į.æ[ij]=æ;}į ʱ=p;ʄ Ȏ;}Č Ǫ;Stack ʄ;ř direct_;};}Ί inverse_mod_words(ƿ,Ό){Ģ d=m-1 Ū divide_words(n,m)Я(d)Я(З(n,m))+ӛ::HalfGcd Ư(ʘ(d,1));}ò ř inverse_mod(Ě&ŋ,О,ĝ g,Ĝ h){Ę ā ӛ Ĩ ř d=ř(ȸ)-1 ĺ(d ε)Ú 0 ǫ b ф(Õ(d))Ȣ rs=remainder_size(Ǳ,ȸ);divide(ŋ,f,g,Ĭ(З(Ǳ,ȸ)),b Ȓ(rs));ř m=ř(rs)-1 ǟ(m>=0&&b[Õ(m)]ε)--m ĺ(m<0)Ú-1;HalfGcd gcd(ŋ,Õ(d));ǋ r=gcd.š(Õ(d),0);gcd.jump(g â,d,b â,m,d,r,0x2)ĺ(ř(r ʱ)!=d)Ú-1;ğ c=Ö((ŉ(r Ӌ Ŭ)*g Ŭ+ŉ(r Ӌ ƹ)*b Ŭ)Ұ);ğ ŧ=mod_inverse(c);Õ t=r.æ ƹ ǟ(t>0&&r.ʮ ƹ[t-1]ε)--t ø(Ů t Į)h ƈ Ҁ(r.ʮ ƹ Ĕ,ŧ)Ū ř(t);}}ā Ü ē{ò Ǝ native_inv_mod(ǣ*a,ƿ,ǣ*b,Ό,л){Ě ŋ(inverse_mod_words(n,m)Я(n)Я(m)Я(m))ѵ f к,g ф(m),h Θ ǌ æ=inverse_mod(ŋ,f,g,h)ĺ(æ<0)Ú Ś;ƚ(į,Õ(æ));ƺ::append_words(į,h â,Õ(æ),932051910)Ū Ƈ;}Ő Sink>í native_gcd_matrix(ǣ*a,ƿ,ǣ*b,Ό,ī ӝ,Sink sink){Ě ŋ(ӛ::HalfGcd Ư(Ԕ)Я(n)Я(m))ѵ f к,g Θ;ӛ::HalfGcd gcd(ŋ,Ԕ)ѵ š=gcd.š(Ԕ,0);gcd.jump(f â,ř(n)-1,g â,ř(m)-1,ř(n)-1,š,ӝ);sink(š);}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
Ӏ{ò Ȭ::ǋ polynomial_gcd_jump(Ù Ȭ::F&a,Ù Ȭ::F&b){
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(ư>=129&&ư<=(1u<<23)&&ę ě){Ȭ::ǋ į;ʃ native_gcd_matrix(ʰ,ư,b â,ζ,3,[&į](Ù ʃ ӛ::ǋ&š){į.ƍ=ý(š ʱ),į.ȭ=š ʱ ε ø(Ϟ){į.a Ƃ.clear();į.a Ƃ.reserve(š.æ Ƃ);ʃ ƺ::append_words(į.a Ƃ,š.ʮ Ƃ,š.æ Ƃ,ã kR);}})Ū į;}
#endif
Ú Ȭ::jump(a,b,ѡ-1,Ƈ);}}ˬ Ɯ PolynomialExtendedGCD{Ȼ gcd,x,y;};ˬ Ȼ polynomial_gcd(Ȼ a,Ȼ b){Ę H=Ü Ȭ;ƙ ӹ(polynomial_gcd(ҹ(a),ҹ(b)))ї{ɞ(a),ɞ(b)ĺ(ư<ζ)a.swap(b)ĺ(!b ű&&ư==ζ){ł r=a.divmod(b).χ;a=Ɔ(b),b=Ɔ(r);}if(!b ű){ł š=Ü polynomial_gcd_jump(a,b)ĺ(š.ƍ==ѡ-1)Ú{Ӽ};a=H::apply_first(š,a,b);}if(!a ű)a/=a.back()Ū a;}}ˬ PolynomialExtendedGCD<T>polynomial_extended_gcd(Ȼ a,Ȼ b){Ę H=Ü Ȭ;ƙ;ƀ{ł ă=polynomial_extended_gcd(ҹ(a),ҹ(b))Ū{ǅ ƾ(ă.gcd),ǅ ƾ(ă.x),ǅ ƾ(ă.y)};}ŝ{ɞ(a),ɞ(b)ĺ(a ű&&b ű)Ú{};ӆ H::ǋ š ǟ(!b ű&&ư<=ζ){ł[q,r]=a.divmod(b);H::ɰ(š,q),a=Ɔ(b),b=Ɔ(r);}if(!b ű){ł part=Ü polynomial_gcd_jump(a,b);a=H::apply_first(part,a,b);š=H::Ź(part,š,Ƈ);}T ć=a.back()ˆ;a*=ć,Ή Ŭ*=ć,Ή ƹ*=ć Ū{Ɔ(a),Ɔ(Ή Ŭ),Ɔ(Ή ƹ)};}}ˬ Ø optional<Ď<T>>polynomial_inv_mod(Ȼ a,Ȼ i8){Ü Ǆ::trim(i8);Ƣ(!i8 ű)ĺ(i8 Û у)Ú Ȼ{}ͼ H=Ü Ȭ;ƙ;ƀ{ł ă=polynomial_inv_mod(ҹ(a),ҹ(i8))ĺ(!ă)Ú Ø nullopt Ū ǅ ƾ(*ă);}ŝ{
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(i8 Û>=129&&i8 Û<=(1u<<23)&&ư<=(1u<<23)ȏ){Ȼ į ĺ(!Ü ʃ native_inv_mod(ʰ,ư,i8 â,i8 Û,į))Ú Ø nullopt Ū į;}
#endif
if(ư>=i8 Û)a=a.divmod(i8).χ;ɞ(a)ĺ(a ű)Ú Ø nullopt ѵ š=H::jump(i8,a,ý(i8 Û)-1,Ƈ)ĺ(š.ƍ!=ý(i8 Û)-1)Ú Ø nullopt;T ˏ=(Ή Ŭ ű?ȓ:Ή Ŭ Ŭ*i8 Ŭ)+(Ή ƹ ű?ȓ:Ή ƹ Ŭ*a Ŭ);Ή ƹ/=ˏ Ū Ɔ(Ή ƹ);}}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ ɯ(Ù Ȼ&f,Ù Ȼ&ş,Ѡ){Ѽ;ƙ Н ó(ş Û);Ƣ(ǀ>=0&&ǀ<=(1<<22)&&ş Û<=(1U<<21))ĺ(ǀ ε)Ú{}ĺ(ǀ у)Ú F{ş ű?ȓ:ş Ŭ};ƀ{ӆ ǅ W ʎ(ĥ(Ǳ,ş Û))ø(ö ó(ʎ Û)Į)ʎ ƈ Ü Ī θ(f Ĕ Ȅ)Ū ǅ ƾ(ɯ Ӹ(ʎ,ҹ(ş),ǀ));}ŝ{ǧ=ó(ş Û)ǟ(n&&ş[Ԕ]Ү)--n ĺ(n ε)Ú F(ǀ);ή=ʡ(n,Ǳ),ù=1 ǟ(ù<î&&f[ù]Ү)++ù;T c=f ű?ȓ:f Ŭ ĺ(ù>=î){Ѭ;T Ĵ=ş Ŭ ø(ł&x:ă)x=Ĵ,Ĵ*=c ȧ}if(ù>1){Ӂ=ù ø(Ż Ԏ ϼ î&&ď>1 Į)if(f Ĕ!=ȓ)ď=Ø gcd(ď,i)ĺ(ď>1){F ʸ((î-1)/ď+1),w((Ԕ)/ď+1)ø(ö ó(ʸ Û)Į)ʸ ƈ f[i Ԍ]ø(ö ó(w Û)Į)w ƈ ş[i Ԍ]Ū ɯ(ʸ,w,ǀ);}}Ԃ Ч(ǀ,(Ԕ)/Ԏ);F ă;ý ȣ=î-1 ǟ(f[ȣ]Ү)--ȣ ĺ(ù==ȣ){ă Ř(þ);T Ĵ=1 ø(ö þ Į)ă ƈ ş[i*ù]*Ĵ,Ĵ*=f[ù];}м(n<=32||þ<=8){ă Ř(þ);F h(f ü,f ʷ ȣ+1),ǹ{Ӽ};h Ŭ Ɠ(ö þ Į){T Ĵ Ɠ(Ȕ ó(ǹ Û)ų)Ĵ+=ǹ Ƃ*ş Ƃ;ă ƈ Ĵ ĺ(i+1<þ){ǹ=F::Ź(ǹ,h)ĺ(ó(ǹ Û)>n)ǹ Ř(n);}}}ŝ
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(n<=(1<<21)ȏ){Ü ȗ::native_weighted_projection(f â,î,ş â,ă,n,þ);}ŝ
#endif
{ă Ř(þ);ý æ=1 ǟ(æ<n)æ<<=1;Ƣ(æ<=(1<<21));ȷ,*q=p+4*æ ԕ(Ȯ(ş),n,p);ʇ(p+n,æ-n,0u);q Ŭ Ɠ(Ż 1;i<=ȣ Į)q ƈ(-f Ĕ).v;ʇ(q+ȣ+1,Ё-ȣ-1,0u);Ǝ unit=n==æ&&ş[Ԕ]==Ӽ&&Ø all_of(ş ü,ş ʷ Ԕ,[](T x){Ú x Ү;})ѵ Ÿ=t.reversion_projection(p,q,æ,unit?Ƿ:p);t.ŧ(p,þ,t.į(Ÿ))ø(ö þ Į)ă ƈ T ɪ(p Ĕ);}if(c Ү){ă ӗ ȧ}F b(ǀ);T Ԓ=1 ø(κ ǀ Į)Ԓ*=i;T g9=Ԓ ˆ ø(Ż ǀ-1;i и i){b ƈ g9 ĺ(i<þ)ă Ĕ*=g9;g9*=i;}ǅ ɘ(b,Ӽ,c);ă=F::Ź(ă,b);ă ӗ;Ԓ=1 ø(ö ǀ Į)ă Ĕ*=Ԓ,Ԓ*=i+1 ȧ}}ˬ Ȼ ɯ(Ù Ȼ&f,ǧ,Ѡ){Ƣ(n>=0&&n<(1<<21));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
Ġ{if(n>=64&&Ǳ>2&&f Ŭ Ү&&f ƹ!=ȓ&&f ǥ!=ȓ ȏ){Ƞ þ=ǀ<0?n+1:ǀ;Ƣ(þ>=0&&þ<=(1<<22))ĺ(þ ε)Ú{};Ȼ ă;Ü ȗ::native_projection(f â,Ǳ,ă,n+1);ă Ř(þ)ȧ}}
#endif
Ȼ ş(n+1);ş[n]=1 Ū ɯ(f,ş,ǀ);}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
Ň
#pragma GCC optimize("O3")
Ӏ{ā taylor_factorials{ò ӂ kStep=1024;ò è ł kTable=[]{ǜ Ö,513>n8{1};ŉ Ĵ=1 ø(ɮ=1 ϼ n8 Û Į){Ć(Ö j=1;j<=kStep ų)Ĵ=Ĵ*((i-1)*kStep+j)%998244353;n8 ƈ Ö(Ĵ);}Ú n8;}();}ā taylor_detail{ā ntt=ʉ::ntt ȅ ƫ ȅ ĕ ȅ ǘ ȅ kP ȅ kR ȅ Đ ȅ ǹ ȅ ń ȅ υ ͼ ʉ::Subtrees ȅ ĭ ͼ ʃ ä Ō ͼ ʃ ä ʗ ͼ ʃ ä Š;ԃ ć(Ö x){Ú ǹ(x,ӓ);}ԃ mont(Ö x){Ú Đ(x,kR);}Ǹ Ȗ(ɍ){Ú ʤ;}ĉ Ǹ Ŕ(ĭ a,ĭ a4,ĭ b,ĭ b4){ġ ni=ĕ(ntt::ƥ kNI),p=ɟ;н=ß(a,b),ȇ=ß(a4,b4);Ĥ Ú ǝ;}ˮ ʈ(ĭ(&r)ɚ){ĭ t ɚ,u ɚ;
#pragma GCC unroll 4
Ć(ö 8 Ƽ 2){ͻ;t Υ Ȁ(r Ĕ,r Υ);}
#pragma GCC unroll 2
Ć(ö 8 Ƽ 4){u ƈ ɻ(t Ĕ,t[i+2]);u Υ ǿ(t Ĕ,t[i+2]);u[i+2]=ɻ(t Υ,t[i+3]);u[i+3]ǿ(t Υ,t[i+3]);}
#pragma GCC unroll 4
Т{ѹ;з}}è ý ȫ=4,ɤ=8*ȫ;Ő F>Ĉ fill(ĭ(&x)[ȫ],F f){ћ Ƴ[ɤ]ԑ Ƴ[s]=f(s)ø(ʚ ȫ ė v)x ӑ=Ō(Ƴ Ƨ v);}Ę Block=ĭ[ȫ]ɚ;Ɯ Chain{ɍ[ȫ],ƻ[ȫ],b6[ȫ],u,ɰ;Ő X,Ũ Base>Chain(X g4,Base b7,Ö n6):u(đ),ɰ(ĕ(n6)){fill(x,g4);fill(ƻ,b7)ø(ʚ ȫ ė v)b6 ӑ=Ȗ(ƻ ӑ);}ĉ Ĉ advance(Block&ŀ,ý t){
#pragma GCC unroll 4
Ć(ʚ ȫ ė v){ŀ ӑ Ѱ=x ӑ;x ӑ=Ŕ(x ӑ,Ȗ(x ӑ),ƫ(ƻ ӑ,u),ƫ(b6 ӑ,u));}u=ń(ƫ(u,ɰ),kP);}};Ũ TaylorShift{Ϳ TaylorShift(ƿ):n_(n){lg_=Ŋ(10,ý(ʌ(2*Ԕ)));lg_+=lg_%2;chunk_=((n+ɤ-1)/ɤ+15)/16*16 ĺ(chunk_/16%2 ε)chunk_+=16 Ȣ ǯ=î(),Ķ=ntt::ä ϙ(lg_)Ȣ č=2*ǯ+3*kPadding ʳ Ķ;ϳ=ɀ(č)ĺ(!ϳ.get_deleter().region)ʇ(ϳ ӯ,č,0u);b_=ϳ ӯ+kPadding;a_=b_+ǯ+kPadding;Ɛ=a_+ǯ+kPadding;ō=Ɛ+Ķ;}Ņ ļ(){Ú a_;}Ɣ ƍ(Ö c){Ę ā ntt::ë Ȣ ǯ=î(),q=ǯ/64;build_table(Ɛ,ǯ/16,kRoots Ŭ);build_table(ō,ǯ/16,kRoots ƹ);lane_factorials();ş(c)ѵ*av=ț(a_)ѵ*bv=ț(b_);ʉ::forward_radix8(av,q,Ɛ);ʉ::forward_radix8(bv,q,Ɛ)ø(Ӄ<8 ė t){Ö&x=b_[8*q*t];x=(x Ұ+(t<4?1:Ͼ 1))Ұ;}Ù Subtrees ѧ(Ɛ,ō)Ȳ ѧ.ϖ(av+t*q,bv+t*q,q,t);ntt::ƥ inverse_identity(av,q,ō)ø(Õ t=4;t<8 ė t)ѧ.ϖ(av+t*q,bv+t*q,q,t);ntt::ƥ ć(av+4*q,q,ō+υ(1),ō+υ(2));ǁ()Ū b_;}Ⱦ ċ ɿ kPadding=1040;Õ î()Ù{Ú Ǽ lg_;}Õ start(ý s)Ù{Ú s*chunk_;}Ĉ lane_factorials(){Ę taylor_factorials::kStep ǌ Ԋ=[this](ý s){Ú Ö(start(s+1)/kStep*kStep);};ɍ[ȫ],m[ȫ],þ[ȫ];fill(x,ѯ(ý s){Ú mont(taylor_factorials::kTable[Ԋ(s)/kStep]);});fill(m,ѯ(ý s){Ú mont(
Ԋ(s)+1);});fill(þ,ѯ(ý s){Ú Ö(start(s+1)-Ԋ(s));})ι unit=ĕ(kR)ø(Ö j Ē j<kStep-1 ų){ġ ɰ=ĕ(j);
#pragma GCC unroll 4
Ć(ʚ ȫ ė v){ġ Ÿ=_mm256_blendv_epi8(unit,m ӑ,σ(þ ӑ,ɰ));x ӑ=Ŕ(x ӑ,Ȗ(x ӑ),Ÿ,Ȗ(Ÿ));m ӑ=ń(ƫ(m ӑ,unit),kP);}}ћ h7[ɤ]ø(ʚ ȫ ė v)Š(h7 Ƨ v,x ӑ);ğ o6=ć(kR);Ӛ Г 1 ԑ Ӛ[s+1]=Đ(h7[s]Ұ,o6);Ö f=1 ø(ý s Ē s<=ɤ ė s)Ϫ[s]=f,f=Đ(f,Ӛ[s]);f=ć(f)ø(ý s=ɤ;s и s)Ϫ[s]=Đ(Ϫ[s],f),f=Đ(f,Ӛ[s]);}Ĉ ş(Ö c){ğ c3=ǹ(c,Ö(chunk_)),c6=mont(ć(c));ǜ Ö,ɤ>c5 ԑ c5[s]=s?Đ(c5[s-1],c3):c3;Chain ʭ([this](ý s){Ú mont(Ӛ[s]);},[this](ý s){Ú mont(Ö(start(s)+1));},kR);Chain kernel(ѯ(ý s){Ú Đ(c5[s],Ϫ[s+1]);},ѯ(ý s){Ú Đ(Ö(start(s+1)),c6);},Ͼ c6)Ȣ ď=chunk_;Block ı ǥ,b5 ǥ;
#pragma GCC unroll 8
С ʭ.advance(ı Ŭ,t),kernel.advance(b5 Ŭ,t)ø(ʍ=0,ԋ Ē j<ď Ӕ,ԋ^=1){ǚ a=a_+j;ǚ e=b_+î()/2-ď+j;
#pragma GCC unroll 8
С{ʭ.advance(ı[ԋ^1],t);kernel.advance(b5[ԋ^1],t)Ȋ v=ȫ*t/8,ù=ȫ*t%8 ĺ(ù ε)ʈ(ı[ԋ]ӑ),ʈ(b5[ԋ]ӑ);
#pragma GCC unroll 4
Ć(ý l=ù;l<Ў ȫ ė l){Ģ at=(8*v+l)Ԍ ι f=Ō(a+at),x=ı[ԋ]ӑ[l];Š(a+at,ń(Ŕ(f,Ȗ(f),x,Ȗ(x)),kP));Š(e-at,ń(b5[ԋ]ӑ[l],kP));}}}}Ĉ ǁ(){ğ ŧ=Đ(mont(ć(Ö(î()/8))),kR);Chain chain(ѯ(ý s){Ú Đ(Đ(Ϫ[s+1],Ö(start(s+1))),ŧ);},[this](ý s){Ú mont(Ö(start(s+1)-1));},Ͼ kR)Ȣ ď=chunk_,Ơ=î()/2;Block ŀ ǥ;
#pragma GCC unroll 8
С chain.advance(ŀ Ŭ,7-t)ø(ʍ=0,ԋ Ē j<ď Ӕ,ԋ^=1){Ɣ Ù u=a_+ď-8-j;ǚ į=b_+ď-8-j;
#pragma GCC unroll 8
С{chain.advance(ŀ[ԋ^1],7-t)Ȋ v=ȫ*t/8,ù=ȫ*t%8 ĺ(ù ε)ʈ(ŀ[ԋ]ӑ);
#pragma GCC unroll 4
Ć(ý l=ù;l<Ў ȫ ė l){Ģ at=(8*v+l)Ԍ ι g ӏ(ǘ(Ō(u+at),Ō(u+at+Ơ))),z=ŀ[ԋ]ӑ[l];Š(є at,ń(Ŕ(g,Ȗ(g),z,Ȗ(z)),kP));}}}}Õ n_,chunk_;ý lg_;Ņ a_,*b_,*Ɛ,*ō;FpsWords ϳ;ǜ Ö,ɤ+1>Ӛ,Ϫ;};í ƍ(ƶ&f,Ī c){TaylorShift t(Ǳ);ɮ Ɠ(ʲ Ǳ ƞ){ʦ x=ʃ ä Ž(õ Ù ī*>(f ȝ));Š(t.ļ()+i,ń(x,kP));}Ć(ϼ Ǳ Į)t.ļ()ƈ ĥ(f Ĕ.v,f Ĕ.v-kP)ǌ*į=t.ƍ(c Ȅ);i Ɠ(ʲ Ǳ ƞ)ʃ ä ū(õ ī*>(f ȝ),Ō(є i))ø(ϼ Ǳ Į)f ƈ һ(į Ĕ);}}}
#pragma GCC pop_options
#endif
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
ˬ Ȼ taylor_shift(Ȼ f,Ø type_identity_t<T>c){Ѽ;ƙ ӹ(taylor_shift Ӹ(ҹ(f),Ü Ī θ(c Ȅ)))ї{Ù ǧ=ó(Ǳ)ĺ(n<=1||c Ү)Ú f;Ƣ(ī(n)<ī(Ҳ));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
if(n>=256&&n<=(1<<19)ȏ){Ü taylor_detail::ƍ(f,c)Ū f;}
#endif
F a(n),b(n),ş;Ü Ӧ(Ȯ(a),Ȯ(b),n);Ű Ŏ(Ȯ(a),Ȯ(a),Ȯ(f),n);Ø ˁ(a ü,a ƃ);ş=b;ǅ ɘ(b,Ӽ,c);F prod=F::Ź(a,b);Ø ˁ(prod ü,prod ʷ n);Ű Ŏ(Ȯ(f),Ȯ(prod),Ȯ(ş),n)Ū f;}}
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
#undef Ý
#undef ß
#undef à
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
#undef Ɗ
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
#undef Ɨ
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
#undef ǎ
#undef Ǐ
#undef Ǒ
#undef ǒ
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
#undef ϝ
#undef Ϟ
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
#endif
