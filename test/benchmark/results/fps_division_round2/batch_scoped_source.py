from pathlib import Path
import shutil
W=Path('/tmp/fps-division-round2');B=Path('/tmp/cp-fps-round2-base');C=W/'scoped_candidate'
if C.exists():shutil.rmtree(C)
shutil.copytree(B,C)
rel='components/fps_division_blocked.hpp';base=(B/rel).read_text();wide=(W/'candidate'/rel).read_text();start=base.index('inline void append_words(');end=base.index('\ninline void subtract(',start);old=base[start:end].strip();a=wide.index('inline void append_words(');b=wide.index('\ninline void subtract(',a);large=wide[a:b];tail=large.index('\tfor (; i + 8 <= n; i += 8)');large=large[:tail]+'\tif (i < n) append_words(out, values + i, n - i, factor);\n}\n';large=large.replace('inline void append_words(','inline void append_division_words(',1).replace('\tconst Factor scale(factor);','\tstatic_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);\n\tconst Factor scale(factor);',1)
s=base[:start]+old+'\n\n'+large+base[end:];s=s.replace('division_detail::append_words(quotient,','division_detail::append_division_words(quotient,').replace('division_detail::append_words(remainder,','division_detail::append_division_words(remainder,').replace('\t\tappend_words(out, block.data(),','\t\tappend_division_words(out, block.data(),');(C/rel).write_text(s)
print('prepared scoped 64-batch candidate',flush=True)
