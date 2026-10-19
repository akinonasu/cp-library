from pathlib import Path
import argparse
import difflib
import hashlib
import json
import re

parser = argparse.ArgumentParser()
parser.add_argument('--work', type=Path, default=Path(__file__).resolve().parent)
args = parser.parse_args()
HERE = args.work.resolve()
def functions(name):
    text = (HERE / name / 'actual.s').read_text()
    result = {}
    for match in re.finditer(r'^\s*\.type\s+(\S+), @function\n', text, re.M):
        symbol = match[1]
        start = text.index(symbol + ':', match.end())
        end = text.index('\n\t.size\t' + symbol + ',', start)
        body = text[start:end]
        labels = {}
        def label(match):
            return labels.setdefault(match[0], '.LOCAL' + str(len(labels)))
        body = re.sub(r'\.L[A-Za-z_]*\d+', label, body)
        result[symbol] = body
    return result

old = functions('old')
current = functions('current')
result = {}
for symbol in old.keys() | current.keys():
    if symbol not in old or symbol not in current:
        result[symbol] = 'missing'
    elif old[symbol] == current[symbol]:
        result[symbol] = 'identical after local-label normalization'
    else:
        result[symbol] = 'changed'
        if 'finish' not in symbol:
            filename = re.sub(r'[^A-Za-z0-9_]', '_', symbol) + '.diff'
            (HERE / filename).write_text(''.join(difflib.unified_diff(old[symbol].splitlines(True), current[symbol].splitlines(True), fromfile='old', tofile='current')))
(HERE / 'assembly_comparison.json').write_text(json.dumps(result, indent=2) + '\n')
(HERE / 'assembly_hashes.json').write_text(json.dumps({symbol: dict(old_sha256=hashlib.sha256(old.get(symbol, '').encode()).hexdigest(), current_sha256=hashlib.sha256(current.get(symbol, '').encode()).hexdigest()) for symbol in old.keys() | current.keys()}, indent=2) + '\n')
print(json.dumps(result, indent=2))
