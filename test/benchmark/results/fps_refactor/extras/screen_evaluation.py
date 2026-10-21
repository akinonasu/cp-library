import subprocess,statistics,json,random,hashlib,argparse
from pathlib import Path
parser=argparse.ArgumentParser(); parser.add_argument('directory',type=Path); root=parser.parse_args().directory; rng=random.Random(739); rows=[]
for n in [32768,131072,524288]:
 for op in ['multipoint_eval','interpolate']:
  out={x:[] for x in ['before','candidate']};checks=set();runs=list(out)*5;rng.shuffle(runs)
  for v in runs:
   fields=subprocess.check_output([str(root/(v+'_split')),op,str(n)],text=True).split()
   out[v].append(float(fields[0])*1000);checks.add(fields[1])
  assert len(checks)==1,(op,n,out,checks)
  row={'operation':op,'n':n,'milliseconds':out,'ratio':statistics.median(out['candidate'])/statistics.median(out['before']),'checksum':next(iter(checks))}
  rows.append(row); print(op,n,round(row['ratio'],4),flush=True)
(root/'evaluation_screen.json').write_text(json.dumps({'rows':rows,'hashes':{v:hashlib.sha256((root/(v+'_split')).read_bytes()).hexdigest() for v in ['before','candidate']}},indent=2)+'\n')
