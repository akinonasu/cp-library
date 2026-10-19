from pathlib import Path
import argparse,collections,hashlib,json,re,subprocess
parser=argparse.ArgumentParser();parser.add_argument('--work',type=Path,default=Path('/tmp/fps-exp-round3-reproduction'));args=parser.parse_args();w=args.work;result={}
for v,needle in [('baseline','native_exp('),('tail_outlined','native_exp_regular(')]:
    binary=w/('exp_'+v);listing=subprocess.check_output(['nm','-S',str(binary)],text=True)
    rows=[]
    for line in listing.splitlines():
        fields=line.split()
        if len(fields)==4 and ('native_exp' in fields[3]):
            name=subprocess.check_output(['c++filt',fields[3]],text=True).strip()
            if needle in name: rows.append((fields,name))
    assert len(rows)==1,rows
    fields,name=rows[0];asm=subprocess.check_output(['objdump','-d','--no-show-raw-insn','--disassemble='+fields[3],str(binary)],text=True)
    (w/('exp_'+v+'_core.asm')).write_text(asm)
    ins=[]
    for line in asm.splitlines():
        m=re.match(r'^\s*[0-9a-f]+:\s+(.*)',line)
        if not m:continue
        text=m[1].split('#')[0].strip()
        text=re.sub(r'[0-9a-f]+ <[^>]*native_exp[^>]*\+(0x[0-9a-f]+)>',r'LOCAL+\1',text)
        text=re.sub(r'[0-9a-f]+ <([^>]+)>',r'<\1>',text)
        text=re.sub(r'-?0x[0-9a-f]+\(%rip\)',r'RIP_DATA',text)
        ins.append(text)
    result[v]=dict(symbol=name,size_bytes=int(fields[1],16),instruction_count=len(ins),opcodes=dict(collections.Counter(x.split()[0] for x in ins)),normalized_instructions_sha256=hashlib.sha256('\n'.join(ins).encode()).hexdigest(),normalized_instructions=ins)
a=result['baseline'].pop('normalized_instructions');b=result['tail_outlined'].pop('normalized_instructions')
result['same_normalized_instructions']=a==b
result['differences']=[dict(index=i,baseline=x,candidate=y) for i,(x,y) in enumerate(zip(a,b)) if x!=y][:40]
(w/'core_assembly.json').write_text(json.dumps(result,indent=2)+'\n');print({k:v for k,v in result.items() if k!='differences'})
