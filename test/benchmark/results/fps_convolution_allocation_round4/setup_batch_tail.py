from pathlib import Path
import shutil,re
W=Path('/workspace/cp-fps-round4/borrow');H='components/fps_convolution.hpp';base=(W/'before'/H).read_text()
C=W/'batch_construct';shutil.copytree(W/'before',C);s=base.replace('\tresult.assign(words, Mint::word(0));\n','')
old='\tMint* const to = result.data() + offset;'
new='''	result.resize(offset);
	std::array<Mint, 64> batch;
	const auto p = _mm256_set1_epi32(Mint::mod());
	std::size_t i = 0;
	for (; i + batch.size() <= a.size(); i += batch.size()) {
		for (std::size_t j = 0; j < batch.size(); j += 8) {
			__m256i x;
			std::memcpy(&x, a.data() + i + j, sizeof(x));
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			std::memcpy(static_cast<void*>(batch.data() + j), &x, sizeof(x));
		}
		result.insert(result.end(), batch.begin(), batch.end());
	}
	for (; i < a.size(); ++i) result.emplace_back(Mint::word(std::min(a[i].v, a[i].v - Mint::mod())));
	result.resize(words);
'''+old
assert old in s;s=s.replace(old,new).replace('\tcopy_input_mint(a, to);\n','');(C/H).write_text(s)
C=W/'tail_store';shutil.copytree(W/'before',C);s=base
start=s.index('inline void inverse_top(Vec* a, std::size_t q, const Vec* w) {');end=s.index('\n}\n',start)+2
old=s[start:end];new=old.replace('inline void inverse_top(Vec* a, std::size_t q, const Vec* w) {','inline void inverse_top_to(Vec* a, std::size_t q, const Vec* w, void* target, std::size_t output_q, std::size_t count) {')
new=new.replace('Vec* const end = a + q;','Vec* const end = a + count;\n\tchar* out = static_cast<char*>(target);\n\tchar* other = out + 128 * output_q;')
# Every vector input read precedes every output store in the asm body.
def store(m):
 line=m[0].replace('vmovdqa','vmovdqu').replace('%[a]','%[out]').replace('%[b]','%[other]').replace('%[h3]','%[oh3]').replace('%[h]','%[oh]')
 return line
new=re.sub(r'"vmovdqa %%ymm\d+, [^\n]*',store,new)
new=new.replace('"add $32, %[b]\\n\\t"','"add $32, %[b]\\n\\t"\n\t\t"add $32, %[out]\\n\\t"\n\t\t"add $32, %[other]\\n\\t"')
new=new.replace(': [a] "+r"(a), [b] "+r"(b)',': [a] "+r"(a), [b] "+r"(b), [out] "+r"(out), [other] "+r"(other)')
new=new.replace(': [end] "r"(end), [h] "r"(32 * q), [h3] "r"(96 * q),',': [end] "r"(end), [h] "r"(32 * q), [h3] "r"(96 * q), [oh] "r"(32 * output_q), [oh3] "r"(96 * output_q),')
s=s[:end]+'\n\n'+new+s[end:]
at=s.index('class Product {');left=s[:at];right=s[at:]
right=right.replace('const std::uint32_t* multiply() {','const std::uint32_t* multiply(Mint* output = nullptr) {',1)
old='\t\ttop_kernels::inverse_top(a, q, w);';new='''		if (output && reinterpret_cast<void*>(output) != reinterpret_cast<void*>(a)) {
			alignas(32) Vec heads[8];
			for (std::size_t group = 0; group < 8; ++group) heads[group] = a[group * q];
			if (q > 1) top_kernels::inverse_top_to(a + 1, q, w, output + 8, q, q - 1);
			top_kernels::inverse_top_to(heads, 1, w, output, q, 1);
		} else top_kernels::inverse_top(a, q, w);'''
assert old in right;right=right.replace(old,new,1);s=left+right
s=s.replace('\tplan.multiply();','\tplan.multiply(result.data());',1).replace('\tif (offset) std::memmove(static_cast<void*>(result.data()), to, size * sizeof(Mint));\n','')
(C/H).write_text(s)
print('batch and tail candidates prepared')
