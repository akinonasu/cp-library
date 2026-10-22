import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import sys
import collections

try:
    from .compact_fps import TOKEN, compact
except ImportError:
    from compact_fps import TOKEN, compact

ROOT = Path(__file__).resolve().parents[1]
QUOTE = re.compile(r'^\s*#\s*include\s*"([^"\n]+)"[^\n]*$', re.MULTILINE)
STANDARD = re.compile(r'^#include <([a-z_]+)>$', re.MULTILINE)
IO = ['components/fast_io.hpp', 'components/fast_io_bulk.hpp', 'components/fps_io.hpp']


def shorten_locals(source):
    tokens = [match[0] for match in TOKEN.finditer(source) if match.lastgroup != 'comment']
    counts = collections.Counter(tokens)
    identifiers = {token for token in tokens if token.isidentifier()}
    protected = set('alignas alignof and and_eq asm atomic_cancel atomic_commit atomic_noexcept auto bitand bitor bool break case catch char char8_t char16_t char32_t class compl concept const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new noexcept not not_eq nullptr operator or or_eq private protected public reflexpr register reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch synchronized template this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while xor xor_eq __attribute__ always_inline noinline may_alias maybe_unused'.split())
    protected.update(('bytes', 'cube_roots', 'cube_quotients', 'inverse_cube_roots', 'inverse_cube_quotients'))
    for line in source.splitlines():
        if line.lstrip().startswith('#') or line.endswith('\\') or 'constexpr' in line:
            protected.update(match[0] for match in TOKEN.finditer(line))
    for i, token in enumerate(tokens):
        if i and tokens[i - 1] in ('.', '->', '::', 'class', 'struct', 'using', 'enum', 'namespace', 'typedef'):
            protected.add(token)
        if i + 1 < len(tokens) and tokens[i + 1] in ('(', '::', '<'):
            protected.add(token)
        if token.startswith(('_', 'k', 'ymm', 'xmm')) or token.endswith('_') or not token.isascii():
            protected.add(token)
    protected.update(re.findall(r'\[\s*(\w+)\s*\]\s*"', source))
    candidates = sorted(token for token in identifiers - protected if len(token) >= 5 and counts[token] >= 2)
    short = (a + b for a in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ' for b in '0123456789'
             if a + b not in identifiers)
    rename = dict(zip(candidates, short))
    return TOKEN.sub(lambda match: rename.get(match[0], match[0]), source), rename


def wrap_lines(source):
    lines = []
    for line in source.splitlines():
        if line.lstrip().startswith('#') or len(line) < 1000:
            lines.append(line)
            continue
        start = 0
        for token in TOKEN.finditer(line):
            if token.end() - start >= 1000:
                lines.append(line[start:token.end()])
                start = token.end()
        lines.append(line[start:])
    return '\n'.join(lines) + '\n'


def source_path(name):
    return (ROOT / name).with_suffix('.inc')


def read_source(name):
    path = source_path(name)
    if not path.exists():
        path = ROOT / name
    text = path.read_text(encoding='utf-8')
    rows = text.splitlines()
    assert rows[0].startswith('#ifndef ') and rows[1] == '#define ' + rows[0].split()[1], name
    assert rows[-1] == '#endif', name
    return rows[0].split()[1], '\n'.join(rows[2:-1]) + '\n', path


def render(names, target, *, expand):
    seen, inputs, guards, external = set(), {}, {}, set()

    def load(name):
        if name in seen:
            return ''
        seen.add(name)
        guard, text, source = read_source(name)
        guards[name] = guard
        inputs[source.relative_to(ROOT).as_posix()] = hashlib.sha256(source.read_bytes()).hexdigest()

        def include(match):
            path = (ROOT / Path(name).parent / match[1]).resolve()
            if not path.is_file():
                path = (ROOT / match[1]).resolve()
            dependency = path.relative_to(ROOT).as_posix()
            if expand and (source_path(dependency).exists() or dependency == 'components/fps_asm.hpp'):
                return load(dependency)
            external.add(path)
            return ''

        return '#line 1 "' + name + '"\n' + QUOTE.sub(include, text)

    raw = ''.join(load(name) for name in names)
    standard = set(STANDARD.findall(raw))
    raw = STANDARD.sub('', raw).replace('\\\n', '')
    raw = '#line 1 "fps_backend.hpp"\n' + raw
    if expand and 'light' not in target:
        raw, _ = shorten_locals(raw)
    packed = compact(raw, max_tokens=128, passes=20, preserve_assertions=False)
    packed = '\n'.join(row for row in packed.splitlines() if row.strip() and not row.startswith('#line ')) + '\n'
    packed = wrap_lines(packed)
    guard = guards[names[0]] if not expand else 'CP_' + Path(target).name.upper().replace('.', '_')
    prefix = '#ifndef ' + guard + '\n#define ' + guard + '\n'
    prefix += '#if defined(__GNUC__)\n#pragma GCC diagnostic push\n#pragma GCC diagnostic ignored "-Wmisleading-indentation"\n#endif\n'
    prefix += ''.join('#include <' + name + '>\n' for name in sorted(standard))
    prefix += ''.join('#include "' + os.path.relpath(path, (ROOT / target).parent) + '"\n' for path in sorted(external))
    suffix = '#if defined(__GNUC__)\n#pragma GCC diagnostic pop\n#endif\n#endif\n'
    return prefix + packed + suffix, guards, inputs


def generate():
    outputs, inputs = {}, {}
    target = 'components/fps_backend.hpp'
    names = sorted(path.with_suffix('.hpp').relative_to(ROOT).as_posix()
                   for path in (ROOT / 'fps').glob('*.inc') if 'light' not in path.name)
    text, guards, sources = render(names, target, expand=True)
    outputs[target] = text
    inputs.update(sources)
    for name, guard in guards.items():
        if name == 'components/fps_asm.hpp':
            continue
        include = os.path.relpath(ROOT / target, (ROOT / name).parent)
        outputs[name] = '#ifndef ' + guard + '\n#define ' + guard + '\n#include "' + include + '"\n#endif\n'
    for name in IO:
        text, _, sources = render([name], name, expand=False)
        outputs[name] = text
        inputs.update(sources)
    return outputs, inputs


def main():
    parser = argparse.ArgumentParser(description='Update checked-in FPS distribution headers')
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--manifest', type=Path)
    args = parser.parse_args()
    outputs, inputs = generate()
    changed = []
    for name, text in outputs.items():
        path = ROOT / name
        data = text.encode('utf-8')
        if not path.exists() or path.read_bytes() != data:
            changed.append(name)
            if not args.check:
                path.write_bytes(data)
    if args.manifest:
        report = {'inputs': inputs, 'outputs': {name: {'bytes': len(text.encode('utf-8')),
                  'sha256': hashlib.sha256(text.encode('utf-8')).hexdigest()} for name, text in outputs.items()}}
        args.manifest.parent.mkdir(parents=True, exist_ok=True)
        args.manifest.write_text(json.dumps(report, indent=2) + '\n')
    if args.check and changed:
        print('Stale FPS headers: ' + ', '.join(changed), file=sys.stderr)
        return 1
    print(('Checked' if args.check else 'Updated') + f' {len(outputs)} distribution headers')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
