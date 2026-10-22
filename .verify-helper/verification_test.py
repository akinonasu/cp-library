import contextlib
import datetime
from importlib.util import find_spec
import json
import os
from pathlib import Path
import tempfile
import time
import unittest
from unittest.mock import patch

from verification import check_judges, get_marker


class JudgePolicy(unittest.TestCase):
    def test_repository_has_no_atcoder_drivers(self):
        root = Path(__file__).resolve().parents[1]
        check_judges(root, (root / "test").rglob("*.test.cpp"))

    def test_atcoder_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "sample.test.cpp"
            for host in ("atcoder.jp", "www.atcoder.jp", "ATCODER.JP."):
                path.write_text(f'#define PROBLEM "https://{host}/contests/sample/tasks/sample_a"\n')
                with self.assertRaisesRegex(ValueError, "AtCoder judge tests are not allowed"):
                    check_judges(root, [path.name])

    def test_public_judges_are_allowed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "sample.test.cpp"
            for url in ("https://judge.yosupo.jp/problem/aplusb", "https://yukicoder.me/problems/no/2362"):
                path.write_text(f'#define PROBLEM "{url}"\n')
                check_judges(root, [path.name])
            path.write_text('int main() {}\n')
            check_judges(root, [path.name])


@unittest.skipUnless(find_spec("onlinejudge_verify"), "Requires online-judge-verify-helper")
class VerificationStatus(unittest.TestCase):
    def test_parallel_reload_preserves_successes_during_lazy_language_initialization(self):
        from onlinejudge_verify import config, marker
        from onlinejudge_verify.languages import list as languages

        with tempfile.TemporaryDirectory() as directory, contextlib.ExitStack() as stack:
            root = Path(directory)
            names = ["a.test.cpp", "b.test.cpp", "c.test.cpp", "d.test.cpp"]
            for name in names:
                (root / name).write_text("int main() {}\n")
            timestamp = (datetime.datetime.now(datetime.timezone.utc) + datetime.timedelta(days=1)).strftime("%Y-%m-%d %H:%M:%S %z")
            path = root / "timestamps.json"
            path.write_text(json.dumps(dict.fromkeys(names, timestamp)))
            previous = Path.cwd()
            os.chdir(root)
            stack.callback(os.chdir, previous)

            initialize = languages.CPlusPlusLanguage

            def slow_initialization():
                time.sleep(0.05)
                return initialize()

            def load(**kwargs):
                return marker.VerificationMarker(json_path=path, use_git_timestamp=False, **kwargs)

            stack.enter_context(patch.object(config, "_loaded_config", None))
            stack.enter_context(patch.object(languages, "_dict", None))
            stack.enter_context(patch.object(languages, "CPlusPlusLanguage", slow_initialization))
            stack.enter_context(patch.object(marker, "get_verification_marker", side_effect=load))
            verification = get_marker()
            for name in names:
                self.assertTrue(verification.is_verified(Path(name)), name)


if __name__ == "__main__":
    unittest.main()
