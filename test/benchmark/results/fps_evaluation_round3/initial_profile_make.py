from pathlib import Path
import shutil
W=Path('/tmp/fps-evaluation-round3');C=W/'profile_base';shutil.copytree(W/'base',C,dirs_exist_ok=True)
p=C/'components/fps_taylor_blocked.hpp';s=p.read_text();s=s.replace('#include <array>','#include <array>\n#include <time.h>\n#include <cstdio>')
s=s.replace('const std::size_t len = length(), q = len / 64;', '''const auto stamp = [] { timespec t; clock_gettime(CLOCK_MONOTONIC_RAW, &t); return 1000000000ULL * t.tv_sec + t.tv_nsec; };
		const auto profile0 = stamp();
		const std::size_t len = length(), q = len / 64;''')
s=s.replace('\t\tlane_factorials();','\t\tconst auto profile1 = stamp();\n\t\tlane_factorials();')
s=s.replace('\t\tweights(c);','\t\tconst auto profile2 = stamp();\n\t\tweights(c);\n\t\tconst auto profile3 = stamp();')
s=s.replace('\t\toutput();','''		const auto profile4 = stamp();
		output();
		const auto profile5 = stamp();
		std::fprintf(stderr, "tables=%llu factorial=%llu weights=%llu ntt=%llu output=%llu\\n", profile1-profile0,profile2-profile1,profile3-profile2,profile4-profile3,profile5-profile4);''')
p.write_text(s)
r=Path('/tmp/fps-reference/polynomial_taylor_shift/409534.cpp').read_text();lo=r.index('class TaylorShift {');s=r[lo:];
s=s.replace('const std::size_t len = length(), q = len / 64;', '''const auto stamp = [] { timespec t; clock_gettime(CLOCK_MONOTONIC_RAW, &t); return 1000000000ULL * t.tv_sec + t.tv_nsec; };
        const auto profile0 = stamp();
        const std::size_t len = length(), q = len / 64;''',1)
s=s.replace('        lane_factorials();','        const auto profile1 = stamp();\n        lane_factorials();',1).replace('        weights(c);','        const auto profile2 = stamp();\n        weights(c);\n        const auto profile3 = stamp();',1)
s=s.replace('        output();','''        const auto profile4 = stamp();
        output();
        const auto profile5 = stamp();
        std::fprintf(stderr, "tables=%llu factorial=%llu weights=%llu ntt=%llu output=%llu\\n",profile1-profile0,profile2-profile1,profile3-profile2,profile4-profile3,profile5-profile4);''',1)
(W/'profile_reference.cpp').write_text('#include <time.h>\n#include <cstdio>\n'+r[:lo]+s)
