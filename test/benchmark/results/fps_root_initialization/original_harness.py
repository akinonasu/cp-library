import io,json,hashlib,subprocess,tarfile,shutil,os,fcntl,random,statistics,time
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
R=Path('/workspace/cp-library');W=Path('/tmp/fps-zero-roots');base='4765ceb';flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];operations=['log','exp','pow','inv','sqrt'];seed=20261020;trials=21

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_SH)
archive=subprocess.check_output(['git','archive',base,'fps','components','math','convolution','test/fps_io.hpp',*[f'test/fps_{op}.test.cpp' for op in operations]],cwd=R)
B=W/'baseline';C=W/'candidate'
for p in [B,C]:
 if p.exists():shutil.rmtree(p)
with tarfile.open(fileobj=io.BytesIO(archive)) as f:f.extractall(B,filter='data')
shutil.copytree(B,C)
p=C/'components/fps_blocked.hpp';s=p.read_text();old='\t\tstd::fill_n(roots_, 2 * table, 0u);';new='\t\tif (!arena.zero_initialized()) std::fill_n(roots_, 2 * table, 0u);';assert s.count(old)==1;s=s.replace(old,new);p.write_text(s)
result={'baseline':subprocess.check_output(['git','rev-parse',base],text=True,cwd=R).strip(),'head':subprocess.check_output(['git','rev-parse','HEAD'],text=True,cwd=R).strip(),'flags':flags,'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],'hardware':subprocess.check_output(['lscpu'],text=True),'seed':seed,'trials':trials,'cpu':2,'change':{'file':'components/fps_blocked.hpp','before':old,'after':new},'sources':{},'builds':{},'kernel':[],'whole':[]}
for name,tree in [('baseline',B),('candidate',C)]:
 result['sources'][name]={str(p.relative_to(tree)):sha(p) for p in sorted(tree.rglob('*.hpp'))}
unit=W/'safety.cpp';unit.write_text(r'''
#include <cstddef>
#include <sys/mman.h>
#ifdef FPS_FORCE_MMAP_FAILURE
static void* fps_failed_mmap(void*, std::size_t, int, int, int, off_t) { return MAP_FAILED; }
#define mmap fps_failed_mmap
#endif
#include "components/fps_blocked.hpp"
#include <cassert>
#include <cstdio>
#undef mmap
int main() {
 using namespace fps_detail;
 using namespace fps_detail::blocked_poly;
 Arena arena(2 * 1024 * 1024);
#ifdef FPS_FORCE_MMAP_FAILURE
 assert(!arena.zero_initialized());
#else
 assert(arena.zero_initialized());
#endif
 for(int lg : {6, 8, 18, 20}) {
  auto prior=arena.take(64);
  std::fill(prior.begin(),prior.end(),0xdeadbeefu);
  Transform transform(arena,lg);
  const std::size_t table=blocked_ntt::detail::table_words(lg),entries=(std::size_t(1)<<lg)/16;
  auto expected=allocate_fps_words(2*table);
  std::fill_n(expected.get(),2*table,0u);
  blocked_ntt::detail::build_table(expected.get(),entries,blocked_ntt::detail::kRoots[0]);
  blocked_ntt::detail::build_table(expected.get()+table,entries,blocked_ntt::detail::kRoots[1]);
  assert(std::equal(expected.get(),expected.get()+table,transform.roots()));
  assert(std::equal(expected.get()+table,expected.get()+2*table,transform.inverse_roots()));
  assert(std::all_of(prior.begin(),prior.end(),[](unsigned x){return x==0xdeadbeefu;}));
 }
 Arena small(4096);
 assert(!small.zero_initialized());
 Transform t(small,8);
 std::puts("safety passed");
}
''')
# Four variants independently exercise normal mmap and forced fallback, with and without sanitizers.
unitjobs=[]
for variant,tree in [('baseline',B),('candidate',C)]:
 for mode,extra in [('mapped',[]),('fallback',['-DFPS_FORCE_MMAP_FAILURE']),('mapped_sanitize',['-fsanitize=address,undefined','-fno-omit-frame-pointer','-g']),('fallback_sanitize',['-DFPS_FORCE_MMAP_FAILURE','-fsanitize=address,undefined','-fno-omit-frame-pointer','-g'])]:
  unitjobs.append((f'safety_{variant}_{mode}',tree,unit,extra))

def compile(job):
 name,tree,source,extra=job;binary=W/name;cmd=['g++',*flags,*extra,'-I',str(tree),str(source),'-o',str(binary)];p=subprocess.run(cmd,capture_output=True,text=True)
 if p.returncode:raise RuntimeError((name,p.stderr))
 return name,{'command':cmd,'source_sha256':sha(source),'binary_sha256':sha(binary),'compiler_output':p.stderr}
for job in unitjobs:
 name,info=compile(job);result['builds'][name]=info
 env=os.environ.copy();env.update(ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1');p=subprocess.run([str(W/name)],env=env,capture_output=True,text=True);assert p.returncode==0,(name,p.stdout,p.stderr);info['output']=p.stdout;info['run_stderr']=p.stderr;print(name,p.stdout.strip(),flush=True)
jobs=[]
for name,tree in [('baseline',B),('candidate',C)]:
 jobs.append(('kernel_'+name,tree,R/'test/benchmark/fps_tuning_basic.cpp',[]))
 for op in operations:jobs.append(('whole_'+op+'_'+name,tree,tree/f'test/fps_{op}.test.cpp',[]))
with ThreadPoolExecutor(max_workers=3) as pool:
 for name,info in pool.map(compile,jobs):result['builds'][name]=info;print('built',name,flush=True)
result['harness_sha256']=sha(Path(__file__));(W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
fcntl.flock(lock,fcntl.LOCK_UN);fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(seed)

def save(): (W/'results.json').write_text(json.dumps(result,indent=2)+'\n')

def summarize(samples):
 before=samples['baseline'];after=samples['candidate'];delta=[b-a for b,a in zip(before,after)]
 return {'samples_ms':samples,'median_ms':{name:statistics.median(v) for name,v in samples.items()},'paired_median_saving_ms':statistics.median(delta),'candidate_wins':sum(d>0 for d in delta),'paired_delta_ms':delta}

for n in [32768,131073,500000]:
 for op in operations:
  samples={name:[] for name in ['baseline','candidate']};checksums=set()
  for rep in range(trials+1):
   order=list(samples);rng.shuffle(order)
   for name in order:
    raw=subprocess.check_output([str(W/('kernel_'+name)),op,str(n)],text=True).split();checksums.add(raw[1]);
    if rep:samples[name].append(float(raw[0])*1000)
  assert len(checksums)==1,(op,n,checksums)
  item={'operation':op,'size':n,'checksum':next(iter(checksums)),**summarize(samples)};result['kernel'].append(item);save();print('kernel',op,n,item['median_ms'],'paired',item['paired_median_saving_ms'],flush=True)
for op in operations:
 problem={'log':'log_of_formal_power_series','exp':'exp_of_formal_power_series','pow':'pow_of_formal_power_series','inv':'inv_of_formal_power_series','sqrt':'sqrt_of_formal_power_series'}[op];folder=Path('/tmp/library-checker-problems/polynomial')/problem;case='max_random_01' if op=='sqrt' else 'max_random_00';inp=folder/'in'/(case+'.in');answer=folder/'out'/(case+'.out');samples={name:[] for name in ['baseline','candidate']}
 for rep in range(trials+1):
  order=list(samples);rng.shuffle(order)
  for name in order:
   out=W/(op+'_'+name+'.out')
   with inp.open('rb') as f,out.open('wb') as g:
    start=time.perf_counter_ns();subprocess.run([str(W/('whole_'+op+'_'+name))],stdin=f,stdout=g,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
   subprocess.run([str(folder/'checker'),str(inp),str(out),str(answer)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   if rep:samples[name].append(elapsed)
 item={'operation':op,'input':str(inp),'input_header':inp.open().readline().strip(),'input_sha256':sha(inp),'answer_sha256':sha(answer),**summarize(samples)};result['whole'].append(item);save();print('whole',op,item['median_ms'],'paired',item['paired_median_saving_ms'],flush=True)
print('complete',flush=True)
