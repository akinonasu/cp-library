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
    'asm', '__asm__', 'assert', 'static_assert', '__LINE__', '__FILE__',
    '__COUNTER__', '__BASE_FILE__', '__INCLUDE_LEVEL__', '__TIMESTAMP__',
}


def lex(text):
    return [match[0] for match in TOKEN.finditer(text)]


@functools.lru_cache(maxsize=None)
def needs_space(left, right):
    joined = left + right
    return lex(joined) != [left, right] or joined.startswith(('//', '/*'))


def compact(source):
    if not source.startswith('#line '):
        raise ValueError('oj-bundle output beginning with #line is required')
    if '\r' in source:
        raise ValueError('LF line endings are required')
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
    macros = set(re.findall(r'^\s*#\s*define\s+([^\W\d]\w*)\(', source, re.MULTILINE))
    protected = PROTECTED | macros
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
        unsafe_lines.add(begin)
        j = i + 1
        while j < len(matches) and (matches[j].lastgroup == 'comment'
                                   or (match[0] in ('asm', '__asm__') and matches[j][0] in ('volatile', '__volatile__'))):
            j += 1
        if j < len(matches) and matches[j][0] == '(':
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
                for j in range(i, min(i + 24, len(code))):
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
    definitions = ''.join('#define ' + name + ' ' + render(code) + '\n' for name, code in dictionary)
    body = ''.join(original if code is None else render(code) + '\n'
                   for code, original in zip(codes, originals))
    cleanup = ''.join('#undef ' + name + '\n' for name in [*(name for name, _ in dictionary), *rename.values()])
    return definitions + body + cleanup


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
