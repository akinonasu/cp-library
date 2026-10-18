from pathlib import Path
import re,subprocess,json,difflib,hashlib
W=Path('/tmp/fps-division-round2');results={};parsed={}
for variant,binary in [('baseline',W/'inverse_mod_baseline'),('candidate',W/'inverse_mod_final')]:
 text=subprocess.check_output(['objdump','-d','-C','--no-show-raw-insn',str(binary)],text=True);(W/('invmod_'+variant+'.asm')).write_text(text);functions={};current=None
 for line in text.splitlines():
  match=re.match(r'^([0-9a-f]+) <(.+)>:$',line)
  if match:
   current=match.group(2);functions[current]={'start':int(match.group(1),16),'instructions':[]};continue
  match=re.match(r'^\s*([0-9a-f]+):\s+(.+)$',line)
  if current and match:
   asm=match.group(2).split('#')[0].strip()
   if not asm or 'nop' in asm or asm.startswith('xchg   %ax,%ax'):continue
   functions[current]['instructions'].append((int(match.group(1),16),asm))
 parsed[variant]=functions
for name,b in parsed['baseline'].items():
 cname=name
 if 'division_detail::fold(' in name:cname='void '+name.replace('division_detail::fold(','division_detail::fold<unsigned int const>(')
 if 'division_detail::subtract(' in name:cname='void '+name.replace('division_detail::subtract(','division_detail::subtract<unsigned int>(')
 if ('gcd_detail::HalfGcd::' not in name and 'division_detail::fold(' not in name and 'division_detail::subtract(' not in name) or cname not in parsed['candidate']:continue
 c=parsed['candidate'][cname]
 def normalized(function):
  ins=function['instructions'];idx={address:i for i,(address,_) in enumerate(ins)};lo=function['start'];hi=ins[-1][0] if ins else lo
  def target(match):
   address=int(match.group(1),16)
   if lo<=address<=hi:
    at=min((a for a in idx if a>=address),default=address)
    return '@L'+str(idx.get(at,'end'))
   return '<'+match.group(2)+'>'
  out=[]
  for _,asm in ins:
   asm=re.sub(r'\b([0-9a-f]+) <(.+)>',target,asm);asm=re.sub(r'[-+]?0x[0-9a-f]+\(%rip\)','[RIP]',asm);out.append(asm)
  return out
 bn,cn=normalized(b),normalized(c);diff=list(difflib.unified_diff(bn,cn,fromfile='baseline',tofile='candidate',lineterm=''))
 results[name]={'baseline_instructions':len(bn),'candidate_instructions':len(cn),'same_normalized_instruction_sequence':bn==cn,'normalized_diff':diff}
report={'harness_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'binaries':{name:hashlib.sha256((W/binary).read_bytes()).hexdigest() for name,binary in [('baseline','inverse_mod_baseline'),('candidate','inverse_mod_final')]},'size_output':subprocess.check_output(['size',str(W/'inverse_mod_baseline'),str(W/'inverse_mod_final')],text=True),'section_size_output':subprocess.check_output(['size','-A',str(W/'inverse_mod_baseline'),str(W/'inverse_mod_final')],text=True),'note':'Read-only objdump comparison. Normalize RIP displacements and symbolic branch/call addresses; remove alignment nops and instruction comments. Local targets use instruction indexes. This compares instruction structure, not a performance proof.','functions':results};(W/'assembly_review.json').write_text(json.dumps(report,indent=2)+'\n')
for name,v in results.items():print(name,v['baseline_instructions'],v['candidate_instructions'],'identical',v['same_normalized_instruction_sequence'])
