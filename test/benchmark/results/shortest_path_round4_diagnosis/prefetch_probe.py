from pathlib import Path
generator=Path(__file__).with_name('finish_probe.py').read_text()
generator=generator[:generator.index('exec(compile(source,')]
exec(compile(generator,str(Path(__file__)), 'exec'))
source=source.replace("/finish_probe'", "/prefetch_probe'").replace('round4-finish-probe','round4-prefetch-probe')
source=source.replace("REVISIONS={'f4':'f4aa743','bd':'bd29f70'}", "REVISIONS={'bd':'bd29f70','prefetch':'bd29f70'}")
source=source.replace('SEED=2026101906','SEED=2026101907')
prefetch='''\n\t\t\tif (i > 32) {\n\t\t\t\tconst auto from = edges[i - 33].from;\n\t\t\t\t__builtin_prefetch(&offset_[from], 1, 3);\n\t\t\t\t__builtin_prefetch(&arcs_[offset_[from] - 1], 1, 3);\n\t\t\t}\n'''
source=source.replace("    env['sources']['own_'+name]={'hashes':copied,'manifest':manifest}", "    env['sources']['own_'+name]={'hashes':copied,'manifest':manifest}\n    if name=='prefetch':\n        csr_actual=snapshot/manifest['components/directed_csr.hpp']\n        csr_actual.write_text(csr_actual.read_text().replace('for (std::size_t i = edges.size(); i; --i) {', 'for (std::size_t i = edges.size(); i; --i) {'+"+repr(prefetch)+"))")
source=source.replace('    csr.write_text(text)', "    if name=='prefetch': text=text.replace('for (std::size_t i = edges.size(); i; --i) {', 'for (std::size_t i = edges.size(); i; --i) {'+"+repr(prefetch)+")\n    csr.write_text(text)")
exec(compile(source,str(Path(__file__)), 'exec'))
