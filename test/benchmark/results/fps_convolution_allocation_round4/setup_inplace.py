from pathlib import Path
import shutil
W=Path('/workspace/cp-fps-round4/borrow');C=W/'range_inplace';shutil.copytree(W/'before',C);H=C/'components/fps_convolution.hpp';s=H.read_text().replace('\tresult.assign(words, Mint::word(0));\n','')
old='\tMint* const to = result.data() + offset;'
new='''	result.resize(offset);
	result.insert(result.end(), a.begin(), a.end());
	Mint* const to = result.data() + offset;
	const auto p = _mm256_set1_epi32(Mint::mod());
	std::size_t i = 0;
	for (; i + 8 <= a.size(); i += 8) {
		__m256i x;
		std::memcpy(&x, to + i, sizeof(x));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		std::memcpy(static_cast<void*>(to + i), &x, sizeof(x));
	}
	for (; i < a.size(); ++i) to[i] = Mint::word(std::min(to[i].v, to[i].v - Mint::mod()));
	result.resize(words);'''
assert old in s;s=s.replace(old,new).replace('\tcopy_input_mint(a, to);\n','');H.write_text(s)
