from pathlib import Path
import sys,subprocess,difflib,json,hashlib
ROOT=Path(subprocess.check_output(['git','rev-parse','--show-toplevel'],text=True).strip())
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'test/benchmark'))
from shortest_path_round3 import freeze
REVISION='c075fb7'
phase=subprocess.check_output(['git','show',REVISION+':test/benchmark/shortest_path_round4.cpp'],cwd=ROOT,text=True)
actual=subprocess.check_output(['git','show',REVISION+':test/dijkstra_path.test.cpp'],cwd=ROOT,text=True).replace('"../graph/','"graph/').replace('"../components/','"components/')
conditions={
    'all':'true',
    'sorted':'std::is_sorted(edges.begin(), edges.end(), [](const Edge &a, const Edge &b) { return a.from < b.from; })',
    'sample64':None,
    'sample128':None,
}
manifests={}
for name in ['current',*conditions,'emplace']:
    tree=HERE/name
    manifests[name]=freeze(tree,phase,revision=REVISION)
    (tree/'phase.cpp').write_text(phase)
    (tree/'actual.cpp').write_text(actual)
    header=tree/'components/directed_csr.hpp'
    original=header.read_text()
    text=original
    if name=='emplace':
        position=text.index('\n\t\tstd::size_t active = offset_[0] != 0;')
        text=text[:position]+'''
\t\tif (n < 8192 && std::is_sorted(edges.begin(), edges.end(), [](const Edge &a, const Edge &b) { return a.from < b.from; })) {
\t\t\tstd::size_t sum = 0;
\t\t\tfor (auto &value : offset_) {
\t\t\t\tauto count = value;
\t\t\t\tvalue = sum;
\t\t\t\tsum += count;
\t\t\t}
\t\t\tarcs_.clear();
\t\t\tarcs_.reserve(edges.size());
\t\t\tfor (const auto &edge : edges) arcs_.emplace_back(edge.to, edge.weight);
\t\t\treturn;
\t\t}
'''+text[position:]
    if name in conditions:
        position=text.index('\n\t\tfor (; i; --i) {')
        selection=''
        condition=conditions[name]
        if name.startswith('sample'):
            count=int(name.removeprefix('sample'))
            selection=f'''\t\t\tconst auto sample = std::min<std::size_t>(edges.size(), {count});
\t\t\tstd::size_t same = 0;
\t\t\tfor (std::size_t j = 1; j < sample; ++j) same += edges[j - 1].from == edges[j].from;
'''
            condition='sample && same * 4 >= (sample - 1) * 3'
        block='''
#if defined(__GNUC__) || defined(__clang__)
\t\telse
#endif
\t\t{
'''+selection+f'''\t\t\tif ({condition}) {{
\t\t\t\twhile (i) {{
\t\t\t\t\tconst auto from = edges[i - 1].from;
\t\t\t\t\tauto pos = offset_[from];
\t\t\t\t\tdo {{
\t\t\t\t\t\tconst auto &edge = edges[--i];
\t\t\t\t\t\tarcs_[--pos] = {{edge.to, edge.weight}};
\t\t\t\t\t}} while (i && edges[i - 1].from == from);
\t\t\t\t\toffset_[from] = pos;
\t\t\t\t}}
\t\t\t\treturn;
\t\t\t}}
\t\t}}
'''
        text=text[:position]+block+text[position:]
        header.write_text(text)
    (HERE/(name+'.patch')).write_text(''.join(difflib.unified_diff(original.splitlines(True),text.splitlines(True),fromfile='a/components/directed_csr.hpp',tofile='b/components/directed_csr.hpp')))
(HERE/'manifest.json').write_text(json.dumps(dict(baseline=REVISION,source_hashes=manifests),indent=2)+'\n')
print('prepared',list(manifests),flush=True)
