exec(open('/workspace/cp-fps-refactor-kernels-study/prepare.py').read().split('variants={}')[0])
s=base_conv
for marker in ['inline void bottom_first(','inline void bottom_last(','[[gnu::always_inline]] inline void first(','[[gnu::always_inline]] inline void last(']:
 s=replace_function(s,marker,'')
def insert_after(source,marker,suffix):
 return replace_function(source,marker,get_function(source,marker)+suffix)
def get_function(source,marker):
 start=source.index(marker); first=source.index('{',start);depth=0
 for match in TOKEN.finditer(source,first):
  if match[0]=='{':depth+=1
  elif match[0]=='}':
   depth-=1
   if not depth:return source[start:match.end()]
 raise ValueError(marker)
s=insert_after(s,'inline void bottom_both(','''
inline void bottom_first(const void* a, const void* b, void* buf, const void* x, const void* y, const void* w) {
\talignas(64) static const std::uint32_t zero[96]{};
\talignas(64) std::uint32_t discarded[32];
\tbottom_both(a, b, buf, x, y, w, discarded, zero, x, y);
}
inline void bottom_last(void* out, const void* cur, const void* ix, const void* iy) {
\talignas(64) static const std::uint32_t zero[32]{};
\talignas(64) std::uint32_t discarded[96];
\tbottom_both(zero, zero, discarded, ix, iy, zero, out, cur, ix, iy);
}
''')
s=insert_after(s,'[[gnu::always_inline]] inline void both(','''
[[gnu::always_inline]] inline void first(const void* a, const void* b, void* buf, const void* x, const void* y) {
\talignas(64) static const std::uint32_t zero[192]{};
\talignas(64) std::uint32_t discarded[64];
\tboth(a, b, buf, x, y, discarded, zero, x, y);
}
[[gnu::always_inline]] inline void last(void* out, const void* cur, const void* ix, const void* iy) {
\talignas(64) static const std::uint32_t zero[64]{};
\talignas(64) std::uint32_t discarded[192];
\tboth(zero, zero, discarded, ix, iy, out, cur, ix, iy);
}
''')
for name,block in [('endpoint',base_block),('endpoint_bottom',(out/'loop_bottom/components/fps_blocked.hpp').read_text())]:
 p=out/name;p.mkdir(exist_ok=True)
 for directory in ['components','fps','math','convolution']:
  shutil.copytree(root/directory,p/directory,dirs_exist_ok=True)
 (p/'components/fps_convolution.hpp').write_text(s)
 (p/'components/fps_blocked.hpp').write_text(block)
 print(name,len(s),len(block),'save',len(base_conv)+len(base_block)-len(s)-len(block))
