from pathlib import Path
import shutil
W=Path('/tmp/fps-borrow-round3')
for name,source in [('profile_before',Path('/tmp/fps-evaluation-round3/base')),('profile_color',W/'borrow32_color')]:
 C=W/name;shutil.copytree(source,C,dirs_exist_ok=True);p=C/'components/fps_convolution.hpp';s=p.read_text().replace('#include <vector>','#include <vector>\n#include <time.h>\n#include <cstdio>')
 marker='template <class Plan>\ninline std::vector<Mint> run('
 start=s.index(marker)
 if name=='profile_before':
  head=s[:start];tail=s[start:];tail=tail.replace('Plan plan(a.size(), b.size());','const auto stamp = [] { timespec t; clock_gettime(CLOCK_MONOTONIC_RAW, &t); return 1000000000ULL * t.tv_sec + t.tv_nsec; };\n\tconst auto p0 = stamp();\n\tPlan plan(a.size(), b.size());\n\tconst auto p1 = stamp();',1)
  tail=tail.replace('const auto *p = plan.multiply();','const auto p2 = stamp();\n\tconst auto *p = plan.multiply();\n\tconst auto p3 = stamp();',1)
  tail=tail.replace('append_fps_words<true>(result, p, size);','append_fps_words<true>(result, p, size);\n\tconst auto p4 = stamp();\n\tstd::fprintf(stderr, "plan=%llu copy=%llu ntt=%llu export=%llu\\n",p1-p0,p2-p1,p3-p2,p4-p3);',1)
  s=head+tail
 else:
  a=s.index('inline std::vector<Mint> run_borrowed(');b=s.index(marker)
  chunk=s[a:b];chunk=chunk.replace('const std::size_t len =','const auto stamp = [] { timespec t; clock_gettime(CLOCK_MONOTONIC_RAW, &t); return 1000000000ULL * t.tv_sec + t.tv_nsec; };\n\tconst auto p0 = stamp();\n\tconst std::size_t len =',1)
  chunk=chunk.replace('result.resize(words);','const auto p1 = stamp();\n\tresult.resize(words);\n\tconst auto p2 = stamp();',1)
  chunk=chunk.replace('copy_input_mint(a, to);','const auto p3 = stamp();\n\tcopy_input_mint(a, to);',1)
  chunk=chunk.replace('plan.multiply();','const auto p4 = stamp();\n\tplan.multiply();\n\tconst auto p5 = stamp();',1)
  chunk=chunk.replace('result.resize(size);','result.resize(size);\n\tconst auto p6 = stamp();\n\tstd::fprintf(stderr, "allocate=%llu resize=%llu plan=%llu copy=%llu ntt=%llu export=%llu\\n",p1-p0,p2-p1,p3-p2,p4-p3,p5-p4,p6-p5);',1)
  s=s[:a]+chunk+s[b:]
 p.write_text(s);print(name)
