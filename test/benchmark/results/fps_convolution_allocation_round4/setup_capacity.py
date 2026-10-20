from pathlib import Path
import shutil
W=Path('/workspace/cp-fps-round4/borrow');H='components/fps_convolution.hpp';base=(W/'before'/H).read_text()
for name,extra in [('capacity_1m',1<<18),('capacity_2m',1<<19)]:
 C=W/name;shutil.copytree(W/'before',C)
 old='\tprepare_fps_output(result, words);'
 new='''	if (words < (1u << 17)) prepare_fps_output(result, words);
	else {
		try { result.reserve(words + EXTRA); }
		catch (const std::bad_alloc&) { result.reserve(words); }
		const auto first = reinterpret_cast<std::uintptr_t>(result.data());
		const auto owned_end = first + result.capacity() * sizeof(Mint), used_end = first + words * sizeof(Mint);
		[[maybe_unused]] const auto advise = [first](std::uintptr_t end, std::uintptr_t alignment, int advice) {
			const auto lo = (first + alignment - 1) & ~(alignment - 1), hi = end & ~(alignment - 1);
			if (lo < hi) static_cast<void>(madvise(reinterpret_cast<void*>(lo), hi - lo, advice));
		};
#if defined(MADV_HUGEPAGE)
		advise(owned_end, 2u * 1024 * 1024, MADV_HUGEPAGE);
#endif
#if defined(MADV_POPULATE_WRITE)
		advise(used_end, 4096, MADV_POPULATE_WRITE);
#endif
	}'''.replace('EXTRA',str(extra))
 assert old in base;(C/H).write_text(base.replace(old,new))
print('capacity variants prepared')
