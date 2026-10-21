exec(open('/workspace/cp-fps-refactor-kernels-study/prepare.py').read().split('variants={}')[0])
start=base_conv.index('inline void copy_input(')
end=base_conv.index('inline std::vector<Mint> multiply(',start)
helpers=base_conv[start:end]
start=base_conv.index('inline void forward_radix8(')
radix=base_conv[start:base_conv.index('\n}\n',start)+3]
text='''#ifndef CP_COMPONENTS_FPS_CONVOLUTION_HPP
#define CP_COMPONENTS_FPS_CONVOLUTION_HPP
#include "fps_blocked.hpp"
#include "fps_output.hpp"
#include "fps_convolution_unbalanced.hpp"
#include "fps_convolution_constant.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\\"avx2\\")")
namespace fps_detail::multiply_detail {
namespace ntt {
namespace kernels = blocked_ntt::kernels;
namespace detail = blocked_ntt::detail;
class Convolution {
\tstd::size_t n_, m_, len_;
\tblocked_poly::Arena arena_;
\tblocked_poly::Transform transform_;
\tstd::uint32_t *a_, *b_;
\tstatic int log_length(std::size_t n, std::size_t m) { return std::max(6, int(std::bit_width(n + m - 2))); }
public:
\tConvolution(std::size_t n, std::size_t m) : Convolution(n, m, nullptr) {}
\tConvolution(std::size_t n, std::size_t m, std::uint32_t* borrowed)
\t\t: n_(n), m_(m), len_(std::size_t(1) << log_length(n, m)),
\t\t  arena_(blocked_poly::Transform::words(log_length(n, m)) + (borrowed ? 1 : 2) * blocked_poly::Arena::footprint(len_)),
\t\t  transform_(arena_, log_length(n, m)), a_(borrowed ? borrowed : arena_.take(len_).data()), b_(arena_.take(len_).data()) {
\t\tif (n == 0 || m == 0 || log_length(n, m) > 23) std::abort();
\t}
\tstatic bool fits(std::size_t n, std::size_t m) {
\t\tconst int lg = log_length(n, m);
\t\treturn lg % 2 == 0 && lg >= 10 && 2 * std::max(n, m) <= std::size_t(1) << lg;
\t}
\tstd::uint32_t* a() { return a_; }
\tstd::uint32_t* b() { return b_; }
\tconst std::uint32_t* multiply() {
\t\tfor (std::size_t i = 0; i < m_; ++i) b_[i] = Mint::word(b_[i]).val();
\t\ttransform_.forward(std::span<const std::uint32_t>(b_, m_), 0, std::span<std::uint32_t>(b_, len_));
\t\ttransform_.cyclic_product(std::span<const std::uint32_t>(a_, n_), 0,
\t\t\tstd::span<std::uint32_t>(a_, len_), std::span<const std::uint32_t>(b_, len_));
\t\treturn a_;
\t}
};
}
using Product = ntt::Convolution;
using ntt::detail::Vec;
using ntt::detail::Factor;
using ntt::detail::add;
using ntt::detail::diff;
using ntt::detail::kP;
inline Vec times(Vec x, const Factor& f) { return ntt::detail::multiply(x, f); }
inline Vec diff_canonical(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, ntt::detail::broadcast(kP)), y); }
class Subtrees {
\tconst std::uint32_t *roots_, *inverse_;
public:
\tSubtrees(const std::uint32_t* r, const std::uint32_t* ir) : roots_(r), inverse_(ir) {}
\tvoid visit(Vec* a, Vec* b, std::size_t nv, std::size_t k) const {
\t\tusing namespace blocked_poly::detail;
\t\tRecursion<ForwardBottom>(roots_, inverse_, ForwardBottom{roots_}).visit(reinterpret_cast<std::uint32_t*>(b), nv, k);
\t\tProductBottom bottom{roots_, inverse_, reinterpret_cast<std::uint32_t*>(b) - 8 * k * nv};
\t\tRecursion<ProductBottom>(roots_, inverse_, bottom).visit(reinterpret_cast<std::uint32_t*>(a), nv, k);
\t}
};
'''+radix+helpers+'''
inline std::vector<Mint> multiply(const std::vector<Mint>& a, const std::vector<Mint>& b) {
\tassert(!a.empty() && !b.empty() && a.size() + b.size() - 1 <= (1 << 23));
\tif (constant::fits(a, b)) return constant::multiply(a, b);
\tif (unbalanced::fits(a.size(), b.size())) return unbalanced::multiply(a, b);
\treturn run_borrowed_as<ntt::Convolution>(a, b);
}
}
#pragma GCC pop_options
#endif
#endif
'''
p=out/'shared_product';p.mkdir(exist_ok=True)
for directory in ['components','fps','math','convolution']:
 shutil.copytree(out/'all_top'/directory,p/directory,dirs_exist_ok=True)
(p/'components/fps_convolution.hpp').write_text(text)
print(len(text),'conv saved',len(base_conv)-len(text))
