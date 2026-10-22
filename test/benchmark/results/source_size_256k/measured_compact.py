import argparse
import bisect
import collections
import functools
import re
import sys
import unicodedata
from pathlib import Path


TOKEN = re.compile(
    r'(?P<raw>(?:u8|u|U|L)?R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimiter)"(?:[^\W\d]\w*)?)'
    r'|(?P<comment>//[^\n]*|/\*.*?\*/)'
    r'|(?:u8|u|U|L)?"(?:[^"\\\n]|\\[\s\S])*"(?:[^\W\d]\w*)?'
    r"|(?:u8|u|U|L)?'(?:[^'\\\n]|\\[\s\S])*'(?:[^\W\d]\w*)?"
    r'|[^\W\d]\w*'
    r"|(?:\d|\.\d)(?:[eEpP][+-]|[\w.'])*"
    r'|::|->\*|->|\.\*|\.\.\.|<<=|>>=|<=>|\+\+|--|&&|\|\||<<|>>|<=|>=|==|!='
    r'|\+=|-=|\*=|/=|%=|&=|\|=|\^=|##|<:|:>|<%|%>|%:|[^\s]',
    re.DOTALL,
)
PROTECTED = {
    'assert', 'static_assert', '__LINE__', '__FILE__',
    '__COUNTER__', '__BASE_FILE__', '__INCLUDE_LEVEL__', '__TIMESTAMP__',
    '__builtin_LINE', '__builtin_FILE', '__builtin_FUNCTION', '__builtin_source_location',
    '__builtin_COLUMN', 'source_location', 'current',
}


def lex(text):
    return [match[0] for match in TOKEN.finditer(text)]


@functools.lru_cache(maxsize=None)
def needs_space(left, right):
    joined = left + right
    return lex(joined) != [left, right] or joined.startswith(('//', '/*'))


def factor_asm_clobbers(source):
    clobbers = ', '.join('"xmm' + str(i) + '"' for i in range(16)) + ', "memory", "cc"'
    pattern = re.compile(r':\s*' + r'\s*,\s*'.join(re.escape(x) for x in clobbers.split(', ')))
    matches = list(TOKEN.finditer(source))
    replacements = []
    starts = [match.start() for match in matches]
    for index, match in enumerate(matches):
        if match[0] not in ('asm', '__asm__'):
            continue
        j = index + 1
        while j < len(matches) and matches[j][0] in ('volatile', '__volatile__'):
            j += 1
        if j == len(matches) or matches[j][0] != '(':
            continue
        depth, k = 1, j + 1
        while k < len(matches) and depth:
            depth += (matches[k][0] == '(') - (matches[k][0] == ')')
            k += 1
        if depth:
            continue
        begin, end = matches[j].end(), matches[k - 1].start()
        for found in pattern.finditer(source, begin, end):
            at = bisect.bisect_right(starts, found.start()) - 1
            if matches[at].start() == found.start() and matches[at][0] == ':':
                replacements.append((found.start(), found.end()))
    if len(replacements) < 2:
        return source
    name = 'CP_FPS_ASM_CLOBBERS_SOURCE'
    while name in source:
        name += '_'
    for begin, end in reversed(replacements):
        original = source[begin:end]
        source = source[:begin] + ': ' + name + '\n' * original.count('\n') + source[end:]
    first, rest = source.split('\n', 1)
    return first + '\n#define ' + name + ' ' + clobbers + '\n' + first + '\n' + rest


def compact(source):
    if not source.startswith('#line '):
        raise ValueError('oj-bundle output beginning with #line is required')
    if '\r' in source:
        raise ValueError('LF line endings are required')
    rewrite_lines = can_rewrite_lines(source)
    source = factor_asm_clobbers(source)
    letters = [
        chr(i) for i in range(0xC0, 0x800)
        if chr(i).isidentifier() and unicodedata.normalize('NFC', chr(i)) == chr(i)
        and chr(i) not in source
    ]
    names = sorted(set(re.findall(r'\bCP_FPS_ASM_\w+', source)))
    if len(names) >= len(letters):
        raise ValueError('not enough unused macro names')
    rename = dict(zip(names, letters))
    letters = letters[len(names):]
    source = TOKEN.sub(lambda match: rename.get(match[0], match[0]), source)
    macros = set(re.findall(r'^\s*#\s*define\s+([^\W\d]\w*)', source, re.MULTILINE))
    function_macros = set(re.findall(r'^\s*#\s*define\s+([^\W\d]\w*)\(', source, re.MULTILINE))
    protected = PROTECTED | (macros - {rename.get('CP_FPS_ASM_BLOCK')})
    unsafe_lines = set()
    newlines = [i for i, char in enumerate(source) if char == '\n']
    matches = list(TOKEN.finditer(source))
    for i, match in enumerate(matches):
        begin = bisect.bisect_left(newlines, match.start())
        if match.lastgroup in ('raw', 'comment') or '\n' in match[0]:
            end = begin + match[0].count('\n')
            unsafe_lines.update(range(begin, end + 1))
        if match[0] not in protected:
            continue
        j = i + 1
        while j < len(matches) and (matches[j].lastgroup == 'comment'
                                   or (match[0] in ('asm', '__asm__') and matches[j][0] in ('volatile', '__volatile__'))):
            j += 1
        called = j < len(matches) and matches[j][0] == '('
        if match[0] not in function_macros or called:
            unsafe_lines.add(begin)
        if called:
            depth = 1
            j += 1
            while j < len(matches) and depth:
                depth += (matches[j][0] == '(') - (matches[j][0] == ')')
                j += 1
            if depth:
                raise ValueError('unbalanced protected macro call')
            end = bisect.bisect_left(newlines, matches[j - 1].end() - 1)
            unsafe_lines.update(range(begin, end + 1))
    del matches

    tokens = []
    indices = {}

    def intern(token):
        if token not in indices:
            indices[token] = chr(len(tokens) + 256)
            tokens.append(token)
        return indices[token]

    def render(code):
        parts = [tokens[ord(symbol) - 256] for symbol in code]
        return ''.join(
            (' ' if i and needs_space(parts[i - 1], part) else '') + part
            for i, part in enumerate(parts)
        )

    codes, originals = [], []
    continuation = False
    for line_number, line in enumerate(source.splitlines(keepends=True)):
        directive = continuation or line.lstrip().startswith('#')
        parts = lex(line)
        keep = directive or line_number in unsafe_lines
        continuation = line.rstrip('\n').endswith('\\')
        if continuation:
            keep = True
        codes.append(None if keep else ''.join(intern(part) for part in parts))
        originals.append(line if keep else None)

    merged_codes, merged_originals, line_counts = [], [], []
    i = 0
    while i < len(codes):
        if codes[i] is None:
            merged_codes.append(None)
            merged_originals.append(originals[i])
            line_counts.append(1)
            i += 1
        else:
            j = i
            while j < len(codes) and codes[j] is not None:
                j += 1
            merged_codes.append(''.join(codes[i:j]))
            merged_originals.append(None)
            line_counts.append(j - i)
            i = j
    codes, originals = merged_codes, merged_originals

    dictionary = []
    for _ in range(12):
        counts = collections.Counter()
        for code in codes:
            if code is None:
                continue
            parts = [tokens[ord(symbol) - 256] for symbol in code]
            for i, first in enumerate(parts):
                if first in ('(', ')', ',', ']', '}') or first.startswith(('"', "'")):
                    continue
                stack = []
                for j in range(i, min(i + 48, len(code))):
                    part = parts[j]
                    if part in ('(', '[', '{'):
                        stack.append(part)
                    elif part in (')', ']', '}'):
                        if not stack or stack.pop() != {')': '(', ']': '[', '}': '{'}[part]:
                            break
                    elif part == ',' and not stack:
                        break
                    if not stack:
                        counts[code[i:j + 1]] += 1
        candidates = []
        for code, count in counts.items():
            if count < 3:
                continue
            size = len(render(code).encode('utf-8'))
            gain = (size - 2) * count - size - 22
            if gain > 12:
                candidates.append((gain, code))
        candidates.sort(reverse=True)
        added = 0
        for _, code in candidates[:400]:
            count = sum(line.count(code) for line in codes if line is not None)
            size = len(render(code).encode('utf-8'))
            if (size - 2) * count - size - 22 <= 8:
                continue
            if len(dictionary) == len(letters):
                break
            name = letters[len(dictionary)]
            symbol = intern(name)
            dictionary.append((name, code))
            codes = [line.replace(code, symbol) if line is not None else None for line in codes]
            added += 1
        if not added:
            break
    while True:
        best = None
        for index, (name, code) in enumerate(dictionary):
            symbol = intern(name)
            uses = sum(line.count(symbol) for line in codes if line is not None)
            uses += sum(body.count(symbol) for _, body in dictionary)
            size = len(render(code).encode('utf-8'))
            gain = size + 18 + 2 * len(name.encode('utf-8')) - (size - len(name.encode('utf-8'))) * uses
            if gain > 2 and (best is None or gain > best[0]):
                best = gain, index, symbol, code
        if best is None:
            break
        _, index, symbol, code = best
        dictionary.pop(index)
        codes = [line.replace(symbol, code) if line is not None else None for line in codes]
        dictionary = [(name, body.replace(symbol, code)) for name, body in dictionary]

    definitions = ''.join('#define ' + name + ' ' + render(code) + '\n' for name, code in dictionary)
    body = ''.join(original if code is None else render(code) + '\n' * lines
                   for code, original, lines in zip(codes, originals, line_counts))
    cleanup = ''.join('#undef ' + name + '\n' for name in [*(name for name, _ in dictionary), *rename.values()])
    result = definitions + body + cleanup
    return compact_lines(result) if rewrite_lines else result


def literal_lines(source):
    newlines = [i for i, char in enumerate(source) if char == '\n']
    protected = set()
    for match in TOKEN.finditer(source):
        if match.lastgroup in ('raw', 'comment'):
            begin = bisect.bisect_left(newlines, match.start())
            protected.update(range(begin, begin + match[0].count('\n') + 1))
    return protected


def can_rewrite_lines(source):
    protected = literal_lines(source)
    sensitive = set(PROTECTED)
    definitions = re.findall(r'^\s*#\s*define\s+([^\W\d]\w*)([^\n]*)', source, re.MULTILINE)
    while True:
        old = len(sensitive)
        for name, value in definitions:
            if sensitive.intersection(lex(value)):
                sensitive.add(name)
        if len(sensitive) == old:
            break
    lines = source.splitlines()
    conditions = []
    for index, line in enumerate(lines):
        if index in protected:
            continue
        match = re.match(r'\s*#\s*(\w+)\b(.*)', line)
        if not match:
            continue
        directive, value = match.groups()
        if directive in ('if', 'elif') and sensitive.intersection(lex(value)):
            return False
        if directive == 'line' and conditions:
            if len(conditions) != 1 or conditions[0]['alternate']:
                return False
            conditions[0]['marker'] = True
        if directive in ('if', 'ifdef', 'ifndef'):
            conditions.append({'marker': False, 'alternate': False})
        elif directive in ('else', 'elif') and conditions:
            if conditions[-1]['marker']:
                return False
            conditions[-1]['alternate'] = True
        elif directive == 'endif' and conditions:
            condition = conditions.pop()
            if condition['marker']:
                following = next((row for row in lines[index + 1:] if row.strip()), '')
                if not re.match(r'\s*#\s*line\b', following):
                    return False
    return True


def compact_lines(source):
    protected = literal_lines(source)
    lines = source.splitlines(keepends=True)
    out, logical, i, conditionals = [], 1, 0, []

    def restore():
        out.append('#line ' + str(logical) + '\n')
        for depth in range(len(conditionals)):
            conditionals[depth] = True

    while i < len(lines):
        index, line = i, lines[i]
        i += 1
        if index not in protected and not line.strip():
            count = 1
            while i < len(lines) and i not in protected and not lines[i].strip():
                count += 1
                i += 1
            logical += count
            if len('#line ' + str(logical) + '\n') < count:
                restore()
            else:
                out.append('\n' * count)
            continue
        marker = re.match(r'\s*#\s*line\s+(\d+)', line) if index not in protected else None
        if marker:
            out.append(line)
            logical = int(marker[1])
            continue
        directive = re.match(r'\s*#\s*(\w+)\b', line) if index not in protected else None
        count, merged = 1, line
        if directive:
            while merged.endswith('\\\n') and i < len(lines):
                merged = merged[:-2] + lines[i]
                i += 1
                count += 1
        if count > 1 and not protected.intersection(range(index, i)):
            out.append(merged)
            logical += count
            restore()
        else:
            out.extend(lines[index:i])
            logical += count
        if directive:
            kind = directive[1]
            if kind in ('if', 'ifdef', 'ifndef'):
                conditionals.append(False)
            elif kind in ('else', 'elif') and conditionals and conditionals[-1]:
                restore()
            elif kind == 'endif' and conditionals:
                if conditionals.pop():
                    restore()
    return ''.join(out)


def main():
    parser = argparse.ArgumentParser(description='Compact bundled FPS source using preprocessing macros')
    parser.add_argument('input', nargs='?', type=Path)
    parser.add_argument('-o', '--output', type=Path)
    parser.add_argument('--limit', type=int, help='require output to be strictly smaller than this byte count')
    args = parser.parse_args()
    source = args.input.read_text(encoding='utf-8') if args.input else sys.stdin.read()
    try:
        result = compact(source)
        size = len(result.encode('utf-8'))
        if args.limit is not None and size >= args.limit:
            raise ValueError(f'{size} bytes is not below {args.limit} bytes')
    except ValueError as error:
        parser.error(str(error))
    if args.output:
        args.output.write_text(result, encoding='utf-8')
    else:
        sys.stdout.write(result)
    print(f'{len(source.encode("utf-8"))} -> {size} bytes', file=sys.stderr)


if __name__ == '__main__':
    main()
