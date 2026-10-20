import json
from pathlib import Path

import shortest_path_round4_check as check


if __name__ == '__main__':
    scripts = [Path(__file__).resolve(), Path(check.__file__).resolve()]
    hashes = {path.relative_to(check.ROOT).as_posix(): check.digest(path.read_bytes()) for path in scripts}
    check.WORK = check.ROOT / '.verify-helper/cache/shortest-path/round5/check'
    check.TREE = check.WORK / 'source'
    check.RESULT = check.ROOT / 'test/benchmark/results/shortest_path_round5_validation.json'
    check.UNITS = ['overwrite_array', *check.UNITS]
    check.main()
    assert hashes == {path.relative_to(check.ROOT).as_posix(): check.digest(path.read_bytes()) for path in scripts}
    result = json.loads(check.RESULT.read_text())
    result['checker_source_sha256'] = hashes
    check.RESULT.write_text(json.dumps(result, indent=2) + '\n')
