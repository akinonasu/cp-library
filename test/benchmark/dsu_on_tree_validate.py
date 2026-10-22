from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / '.verify-helper/cache/dsu-on-tree/validation'
OUTPUT = ROOT / 'test/benchmark/results/dsu_on_tree_validation.json'
SOURCES = ['tree/dsu_on_tree.hpp', 'components/undirected_csr.hpp', 'test/unit/dsu_on_tree.cpp',
           'test/dsu_on_tree_subtree_sum.test.cpp', 'docs/dsu_on_tree.md']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    os.chdir(ROOT)
    WORK.mkdir(parents=True, exist_ok=True)
    record = dict(utc=datetime.now(timezone.utc).isoformat(), source_sha256={p: sha(ROOT / p) for p in SOURCES}, commands=[])
    previous_path = WORK.parent / 'gcc-unit.json'
    previous = json.loads(previous_path.read_text()) if previous_path.exists() else None
    reused = (previous is not None and previous['compile_exit'] == 0 and previous['run_exit'] == 0 and
              all((ROOT / p).is_file() and sha(ROOT / p) == value for p, value in previous['source_sha256'].items()) and
              (WORK.parent / 'unit').is_file() and sha(WORK.parent / 'unit') == previous['binary_sha256'])
    record['successful_gcc_unit'] = previous if reused else {'reused': False}
    log = OUTPUT.with_suffix('.log')
    log.write_text('')

    def save():
        OUTPUT.write_text(json.dumps(record, indent=2) + '\n')

    def run(command, input_data=None, expected=None):
        command = list(map(str, command))
        process = subprocess.run(command, input=input_data, capture_output=True, timeout=120)
        row = dict(command=command, exit_code=process.returncode, stdout_sha256=hashlib.sha256(process.stdout).hexdigest())
        if expected is not None:
            row['answer_matches'] = process.stdout.split() == expected.split()
        record['commands'].append(row)
        with log.open('a') as stream:
            stream.write('$ ' + shlex.join(command) + '\n' + process.stderr.decode(errors='replace'))
            if expected is None:
                stream.write(process.stdout.decode(errors='replace'))
            stream.write('exit_code=' + str(process.returncode) + '\n')
        save()
        assert process.returncode == 0, (command, process.stderr.decode(errors='replace'))
        if expected is not None:
            assert row['answer_matches'], (command, process.stdout[:1000], expected[:1000])
        return process.stdout

    common = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT)]
    standalone = WORK / 'header.cpp'
    standalone.write_text('#include "tree/dsu_on_tree.hpp"\n#include "tree/dsu_on_tree.hpp"\n'
                          '#if defined(TEMPLATE_HPP) || defined(rep) || defined(all)\n#error "template macros"\n#endif\nint main() {}\n')
    snippets = re.findall(r'```cpp\n(.*?)```', (ROOT / 'docs/dsu_on_tree.md').read_text(), re.S)
    source = '#include "tree/dsu_on_tree.hpp"\nint main() {\n'
    source += re.sub(r'^#include.*\n', '', snippets[0], flags=re.M)
    source += '\nassert((answer == std::vector<int>{3, 2, 1, 1, 1}));\n'
    source += snippets[1] + '\nassert((mode == std::vector<int>{2, 2, 1, 1, 1}));\n}\n'
    (WORK / 'example.cpp').write_text(source)
    for compiler in ['g++', 'clang++']:
        run([compiler, '--version'])
        run([compiler, *common, '-fsyntax-only', standalone])
        run([compiler, *common, '-DNDEBUG', '-fsyntax-only', standalone])
        if compiler == 'clang++' or not reused:
            binary = WORK / (compiler + '-unit')
            run([compiler, *common, 'test/unit/dsu_on_tree.cpp', '-o', binary])
            run([binary])
        run([compiler, *common, WORK / 'example.cpp', '-o', WORK / (compiler + '-example')])
        run([WORK / (compiler + '-example')])
        run([compiler, *common, '-DNDEBUG', 'test/dsu_on_tree_subtree_sum.test.cpp', '-o', WORK / (compiler + '-subtree_sum')])
    run(['clang++', *common, '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g',
         'test/unit/dsu_on_tree.cpp', '-o', WORK / 'sanitize-unit'])
    run([WORK / 'sanitize-unit'])

    upstream = Path('/home/qand/.cache/online-judge-tools/library-checker-problems')
    problem = upstream / 'tree/vertex_add_subtree_sum'
    record['library_checker'] = dict(commit=subprocess.check_output(['git', '-C', upstream, 'rev-parse', 'HEAD'], text=True).strip(),
                                     generator_sha256=sha(upstream / 'generate.py'), info_sha256=sha(problem / 'info.toml'), cases=[])
    for path in sorted((problem / 'in').glob('*.in')):
        expected = (problem / 'out' / path.with_suffix('.out').name).read_bytes()
        record['library_checker']['cases'].append(dict(name=path.name, input_sha256=sha(path), expected_sha256=hashlib.sha256(expected).hexdigest()))
        for compiler in ['g++', 'clang++']:
            run([WORK / (compiler + '-subtree_sum')], path.read_bytes(), expected)

    selection = dict(headers=['tree/dsu_on_tree.hpp'], bundles=['tree/dsu_on_tree.hpp'])
    (WORK / 'selection.json').write_text(json.dumps(selection))
    generator = WORK / 'docs.py'
    generator.write_text('from pathlib import Path\nimport sys\nsys.path.insert(0, ".verify-helper")\n'
                         'from onlinejudge_verify.documentation import configure\n'
                         'configure.find_markdown_paths = lambda **kwargs: [Path("docs/dsu_on_tree.md")]\n'
                         'from documentation import generate\n'
                         'generate({"state":{"targets":{"headers":{"tree/dsu_on_tree.hpp":""},"verification":{}}}})\n')
    run([sys.executable, generator])
    run([sys.executable, '.verify-helper/check-docs.py', '--selection', WORK / 'selection.json'])
    page = ROOT / '.verify-helper/markdown/tree/dsu_on_tree.hpp.md'
    record['documentation_sha256'] = sha(page)
    assert record['source_sha256'] == {p: sha(ROOT / p) for p in SOURCES}
    record['status'] = 'passed'
    save()
    print('DSU on Tree validation passed', flush=True)


if __name__ == '__main__':
    main()
