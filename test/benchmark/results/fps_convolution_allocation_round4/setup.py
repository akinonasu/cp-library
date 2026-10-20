from pathlib import Path
import io,shutil,subprocess,tarfile
W=Path('/workspace/cp-fps-round4/borrow');R=Path('/workspace/cp-library');B=W/'before';commit='7521ea8703a362aaf3ccd370188fccab86050848'
if not B.exists():
 data=subprocess.check_output(['git','archive',commit,'fps','components','math','convolution','test'],cwd=R)
 with tarfile.open(fileobj=io.BytesIO(data)) as t:t.extractall(B,filter='data')
header='components/fps_convolution.hpp';base=(B/header).read_text()
for name in ['range_import','resize_control']:
 C=W/name
 if C.exists():raise ValueError(name+' exists')
 shutil.copytree(B,C);s=base
 if name=='resize_control':s=s.replace('result.assign(words, Mint::word(0));','result.resize(words);')
 else:
  s=s.replace('inline void forward_radix8(Vec* f, std::size_t q, const std::uint32_t* roots) {','template <bool normalize = false>\ninline void forward_radix8(Vec* f, std::size_t q, const std::uint32_t* roots) {')
  old='\t\tconst Vec f0 = f[j], f1 = f[j + q], f2 = f[j + 2 * q], f3 = f[j + 3 * q];'
  new='''		const auto input = [](Vec value) __attribute__((target("avx2"))) {
			if constexpr (normalize) return reduce(value, kP);
			else return value;
		};
		const Vec f0 = input(f[j]), f1 = input(f[j + q]), f2 = input(f[j + 2 * q]), f3 = input(f[j + 3 * q]);'''
  assert old in s;s=s.replace(old,new)
  at=s.index('class Product {');left=s[:at];right=s[at:]
  old='\tconst std::uint32_t* multiply() {\n\t\tusing namespace ntt::detail;'
  new='\tconst std::uint32_t* multiply() { return multiply<false>(); }\n\n\ttemplate <bool normalize_a>\n\tconst std::uint32_t* multiply() {\n\t\tusing namespace ntt::detail;'
  assert old in right;right=right.replace(old,new,1).replace('forward_radix8(a, q, roots_);','forward_radix8<normalize_a>(a, q, roots_);',1)
  s=left+right
  s=s.replace('\tresult.assign(words, Mint::word(0));\n','')
  old='\tMint* const to = result.data() + offset;'
  new='\tresult.resize(offset);\n\tresult.insert(result.end(), a.begin(), a.end());\n\tresult.resize(words);\n'+old
  assert old in s;s=s.replace(old,new)
  s=s.replace('\tcopy_input_mint(a, to);\n','').replace('\tplan.multiply();\n','\tplan.multiply<true>();\n',1)
 (C/header).write_text(s)
h=(R/'test/benchmark/fps_convolution_borrow_round3.py').read_text().replace('bd951dce6fdcb3ea9db4fcef342147012a8ab0c0',commit).replace('20261025','20261041')
# Exploratory timing validates every timed output; full official sweep is deferred to promising candidates.
a=h.index('        for variant in trees:\n');b=h.index('        fcntl.flock(lock, fcntl.LOCK_EX)',a);h=h[:a]+h[b:]
h=h.replace("input_sha256=sha(inp), orders=orders", "input_sha256=sha(inp), answer_sha256=sha(args.official / 'out' / (filename + '.out')), checker_sha256=sha(args.official / 'checker'), orders=orders")
(W/'whole.py').write_text(h)
print('created',W)
