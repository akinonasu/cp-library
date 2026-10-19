from pathlib import Path
import shutil
W=Path('/workspace/cp-fps-round4/borrow');header='components/fps_convolution.hpp';base=(W/'before'/header).read_text()
for name in ['range_transform','reserve_only','huge_only']:
 C=W/name
 if C.exists():raise ValueError(name+' exists')
 shutil.copytree(W/'before',C);s=base
 if name=='range_transform':
  s=s.replace('#include <vector>','#include <vector>\n#include <ranges>')
  s=s.replace('\tresult.assign(words, Mint::word(0));\n','')
  old='\tMint* const to = result.data() + offset;'
  new='''	result.resize(offset);
	const auto input = a | std::views::transform([](Mint x) {
		return Mint::word(std::min(x.v, x.v - Mint::mod()));
	});
	result.insert(result.end(), input.begin(), input.end());
	result.resize(words);
'''+old
  assert old in s;s=s.replace(old,new).replace('\tcopy_input_mint(a, to);\n','')
 elif name=='reserve_only':
  s=s.replace('\tprepare_fps_output(result, words);','\tresult.reserve(words);')
 else:
  s=s.replace('\tprepare_fps_output(result, words);','''	result.reserve(words);
#if defined(MADV_HUGEPAGE)
	const auto first = reinterpret_cast<std::uintptr_t>(result.data()), end = first + words * sizeof(Mint);
	constexpr std::uintptr_t huge = 2u * 1024 * 1024;
	const auto lo = (first + huge - 1) & ~(huge - 1), hi = end & ~(huge - 1);
	if (lo < hi) static_cast<void>(madvise(reinterpret_cast<void*>(lo), hi - lo, MADV_HUGEPAGE));
#endif''')
 (C/header).write_text(s)
print('prepared more')
