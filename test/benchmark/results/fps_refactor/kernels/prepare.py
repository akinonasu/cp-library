from pathlib import Path
import shutil,sys
root=Path('/workspace/cp-library'); out=Path('/workspace/cp-fps-refactor-kernels-study')
sys.path.insert(0,str(root/'tools'))
from compact_fps import TOKEN

def replace_function(source, marker, replacement):
 start=source.index(marker); first=source.index('{',start); depth=0
 for match in TOKEN.finditer(source,first):
  if match[0]=='{': depth+=1
  elif match[0]=='}':
   depth-=1
   if not depth:
    return source[:start]+replacement+source[match.end():]
 raise ValueError(marker)
base_conv=(root/'components/fps_convolution.hpp').read_text()
base_block=(root/'components/fps_blocked.hpp').read_text()
variants={}
conv=replace_function(base_conv,'inline void bottom_both(','''inline void bottom_both(const void* a, const void* b, void* buf, const void* x, const void* y, const void* w, void* out, const void* cur, const void* ix, const void* iy) {
\tbottom_first(a, b, buf, x, y, w);
\tbottom_last(out, cur, ix, iy);
}''')
conv=replace_function(conv,'[[gnu::always_inline]] inline void both(','''[[gnu::always_inline]] inline void both(const void* a, const void* b, void* buf, const void* x, const void* y, void* out, const void* cur, const void* ix, const void* iy) {
\tfirst(a, b, buf, x, y);
\tlast(out, cur, ix, iy);
}''')
variants['split_both']=(conv,base_block)
block=replace_function(base_block,'[[gnu::noinline]] inline void forward_bottom(','')
block=replace_function(block,'[[gnu::noinline]] inline void inverse_bottom(','')
a='''\t\tif (count % 8 == 0)
\t\t\treturn kernels::forward_bottom(reinterpret_cast<Vec*>(a), count, roots + slot(first), roots + slot(2 * first));
'''
b='''\t\tif (count % 8 == 0)
\t\t\treturn kernels::inverse_bottom(reinterpret_cast<Vec*>(a), count, inverse_roots + slot(first),
\t\t\t\t\t\t\t\t\t\tinverse_roots + slot(2 * first), reinterpret_cast<const Vec*>(from));
'''
assert a in block
block=block.replace(a,'')
# Match continuation indentation from the actual source.
import re
block,n=re.subn(r'\t\tif \(count % 8 == 0\)\n\t\t\treturn kernels::inverse_bottom\(.*?\);\n','',block,flags=re.S)
assert n==1
variants['loop_bottom']=(base_conv,block)
variants['both_bottom']=(conv,block)
for name,inverse in [('forward_h4',False),('inverse_h4',True)]:
 block=replace_function(block,'[[gnu::noinline]] inline void '+name+'(',f'''[[gnu::noinline]] inline void {name}(Vec* a, std::size_t count, const std::uint32_t* table, std::size_t k) {{
\tfor (std::size_t j = 0; j < count; ++j)
\t\tblocked_ntt::kernels::{'inverse' if inverse else 'forward'}(a + 16 * j, 4, table + blocked_ntt::detail::slot(k + j), table + blocked_ntt::detail::slot(2 * (k + j)));
}}''')
variants['all_h4']=(conv,block)
for name in ['forward_top8_lower','forward_top8_upper']:
 block=replace_function(block,'[[gnu::noinline]] inline void '+name+'(','')
block=replace_function(block,'inline bool forward_top8(','''inline bool forward_top8(const Source&, std::uint32_t*, std::size_t, const std::uint32_t*) { return false; }''')
variants['all_top']=(conv,block)
for name,(c,b) in [('baseline',(base_conv,base_block)),*variants.items()]:
 p=out/name
 p.mkdir(exist_ok=True)
 for directory in ['components','fps','math','convolution']:
  shutil.copytree(root/directory,p/directory,dirs_exist_ok=True)
 (p/'components/fps_convolution.hpp').write_text(c)
 (p/'components/fps_blocked.hpp').write_text(b)
 print(name,len(c),len(b),'save',len(base_conv)+len(base_block)-len(c)-len(b))
