from pathlib import Path
import shutil
w=Path('/workspace/cp-fps-refactor-kernels-study');p=w/'shared_safe';p.mkdir(exist_ok=True)
for d in ['components','fps','math','convolution']:shutil.copytree(w/'shared_product'/d,p/d,dirs_exist_ok=True)
f=p/'components/fps_blocked.hpp';s=f.read_text()
s=s.replace('inline std::size_t table_words(int lg)','inline constexpr std::size_t table_words(int lg)')
old='''struct ProductBottom {
\tstatic constexpr bool kForward = true, kInverse = true;
\tconst std::uint32_t* roots;
\tconst std::uint32_t* inverse_roots;
\tconst std::uint32_t* b;
'''
assert old in s;s=s.replace(old,old+'\tstd::size_t origin = 0;\n')
s=s.replace('b + 8 * (4 * g + t)', 'b + 8 * (4 * g + t - origin)')
s=s.replace('terms[0].product.b + 8 * (4 * g + t)', 'terms[0].product.b + 8 * (4 * g + t - terms[0].product.origin)')
s=s.replace('terms[k].product.b + 8 * (4 * g + t)', 'terms[k].product.b + 8 * (4 * g + t - terms[k].product.origin)')
# The preceding generic replacement also matches the suffix of member expressions.
s=s.replace('terms[0].product.b + 8 * (4 * g + t - origin)', 'terms[0].product.b + 8 * (4 * g + t - terms[0].product.origin)')
s=s.replace('terms[k].product.b + 8 * (4 * g + t - origin)', 'terms[k].product.b + 8 * (4 * g + t - terms[k].product.origin)')
s=s.replace('product.b + 32 * (first + j)', 'product.b + 8 * (4 * (first + j) - product.origin)')
s=s.replace('term.product.b + leaf)', 'term.product.b + (leaf - 8 * term.product.origin))')
f.write_text(s)
f=p/'components/fps_convolution.hpp';s=f.read_text()
s=s.replace('reinterpret_cast<std::uint32_t*>(b) - 8 * k * nv', 'reinterpret_cast<std::uint32_t*>(b), k * nv')
s=s.replace('static int log_length(std::size_t n, std::size_t m) { return std::max(6, int(std::bit_width(n + m - 2))); }','''static int log_length(std::size_t n, std::size_t m) {
\t\tif (n == 0 || m == 0 || n > (1u << 23) || m > (1u << 23) || n + m - 1 > (1u << 23)) std::abort();
\t\treturn std::max(6, int(std::bit_width(n + m - 2)));
\t}''')
f.write_text(s)
print('shared_safe',len(s),(p/'components/fps_blocked.hpp').stat().st_size)
