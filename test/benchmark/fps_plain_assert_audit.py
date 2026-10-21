import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.compact_fps import lex


def normalize_asserts(tokens):
    result, i = [], 0
    while i < len(tokens):
        if tokens[i] != '__assert_fail' or i + 1 == len(tokens) or tokens[i + 1] != '(':
            result.append(tokens[i])
            i += 1
            continue
        j, depth, arguments, begin = i + 2, 0, [], i + 2
        while j < len(tokens):
            token = tokens[j]
            if token in ('(', '[', '{'):
                depth += 1
            elif token in (')', ']', '}'):
                if depth == 0:
                    arguments.append(tokens[begin:j])
                    break
                depth -= 1
            if token == ',' and depth == 0:
                arguments.append(tokens[begin:j])
                begin = j + 1
            j += 1
        assert len(arguments) == 4, arguments
        result += ['__assert_fail', '(', 'assert_text', ',', 'assert_file', ',', 'assert_line', ',']
        result += arguments[3] + [')']
        i = j + 1
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--audit', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    audit = json.loads(args.audit.read_text())
    assert audit['completed']
    inverse = {value: name for name, value in audit['local_names'].items()}
    sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    report = {'started_at': datetime.now(timezone.utc).isoformat(), 'completed': False,
              'harness_sha256': sha(Path(__file__)), 'release_audit_sha256': sha(args.audit), 'pairs': [],
              'scope': 'Assertions enabled. Normalize only diagnostic pragmas and the text/file/line arguments of __assert_fail; retain every assertion predicate and all remaining C++ tokens. Restore internal local names inside the normal implementation.'}
    for pair in audit['pairs']:
        streams, variants = [], {}
        for variant, proof in pair['variants'].items():
            command = [argument for argument in proof['command'] if argument != '-DNDEBUG']
            source = Path(command[-1])
            target = source.parent / (str(len(report['pairs'])) + '-asserts.ii')
            log = target.with_suffix('.log')
            with target.open('wb') as out, log.open('wb') as err:
                subprocess.run([*command, '-E', '-P'], cwd=ROOT, stdout=out, stderr=err, check=True)
            lines = [line for line in target.read_text().splitlines() if not line.lstrip().startswith('#pragma GCC diagnostic')]
            tokens = normalize_asserts(lex('\n'.join(lines)))
            if variant == 'after' and pair['profile'] != 'light_usage':
                begin, end = tokens.index('cp_fps_plain_start'), tokens.index('cp_fps_plain_end')
                tokens[begin:end] = [inverse.get(token, token) for token in tokens[begin:end]]
            streams.append(tokens)
            variants[variant] = {'command': [*command, '-E', '-P'], 'preprocessed_sha256': sha(target),
                                 'log_sha256': sha(log), 'tokens': len(tokens)}
        row = {'profile': pair['profile'], 'compiler': pair['compiler'], 'extra': pair['extra'],
               'tokens_equal': streams[0] == streams[1], 'variants': variants}
        report['pairs'].append(row)
        args.output.write_text(json.dumps(report, indent=2) + '\n')
        assert row['tokens_equal'], row
        print(row['profile'], row['compiler'], row['extra'], 'assertions equal', flush=True)
    report.update(completed=True, completed_at=datetime.now(timezone.utc).isoformat())
    args.output.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
