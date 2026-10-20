import contextlib
import io
import json
from pathlib import Path
import runpy
import subprocess
import sys
import tempfile
import threading
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parent.parent


class ParallelVerification(unittest.TestCase):
    def run_checks(self, selection, run, *flags):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "selection.json"
            path.write_text(json.dumps(selection))
            arguments = ["verify.py", "--unit-only", "--jobs", "2", "--selection", str(path), *flags]
            with patch.object(sys, "argv", arguments), patch("subprocess.run", side_effect=run), \
                    contextlib.redirect_stdout(io.StringIO()):
                runpy.run_path(str(ROOT / "test/verify.py"), run_name="__main__")

    def test_headers_compile_concurrently_without_sharing_files(self):
        barrier = threading.Barrier(2, timeout=5)
        lock = threading.Lock()
        sources = {}
        def run(command, **kwargs):
            source = Path(command[-1])
            barrier.wait()
            with lock:
                sources[str(source)] = source.read_text()
        headers = ["tree/dsu_on_tree.hpp", "components/barrett32.hpp"]
        self.run_checks({"headers": headers, "unit": []}, run, "--cxx", "g++", "clang++")
        self.assertEqual(len(sources), 4)
        for header in headers:
            self.assertEqual(sum(f'#include "{header}"' in source for source in sources.values()), 2)

    def test_executables_are_private_to_each_compilation(self):
        barrier = threading.Barrier(2, timeout=5)
        lock = threading.Lock()
        compiled, executed = [], []
        def run(command, **kwargs):
            if command[0] in ("g++", "clang++"):
                barrier.wait()
                with lock:
                    compiled.append(command[-1])
            else:
                with lock:
                    executed.append(command[0])
        self.run_checks({"headers": [], "unit": ["test/unit/dsu_on_tree.cpp"]}, run, "--cxx", "g++", "clang++")
        self.assertEqual(len(set(compiled)), 2)
        self.assertEqual(sorted(compiled), sorted(executed))

    def test_compile_failure_fails_verification(self):
        def failed(command, **kwargs):
            raise subprocess.CalledProcessError(1, command)
        with self.assertRaises(subprocess.CalledProcessError):
            self.run_checks({"headers": [], "unit": ["test/unit/dsu_on_tree.cpp"]}, failed, "--cxx", "g++")

    def test_runtime_failure_fails_verification(self):
        def failed(command, **kwargs):
            if command[0] != "g++":
                raise subprocess.CalledProcessError(1, command)
        with self.assertRaises(subprocess.CalledProcessError):
            self.run_checks({"headers": [], "unit": ["test/unit/dsu_on_tree.cpp"]}, failed, "--cxx", "g++")

    def test_sanitizer_keeps_its_flags_and_selection(self):
        commands = []
        def run(command, **kwargs):
            commands.append(command)
        self.run_checks({"headers": ["tree/dsu_on_tree.hpp"], "unit": [], "sanitize": ["test/unit/dsu_on_tree.cpp"]},
                        run, "--cxx", "clang++", "--sanitize")
        self.assertEqual(len(commands), 2)
        self.assertIn("-fsanitize=address,undefined", commands[0])
        self.assertNotIn("-fsyntax-only", commands[0])

    def test_workflow_prefers_the_matching_image_cache(self):
        import yaml

        workflow = yaml.load((ROOT / ".github/workflows/verify.yml").read_text(), Loader=yaml.BaseLoader)
        steps = workflow["jobs"]["verify"]["steps"]
        image = next(step for step in steps if step.get("id") == "cache-environment")
        self.assertEqual(image["if"], "steps.route.outputs.mode == 'normal'")
        self.assertIn("ImageVersion", image["run"])
        cache = next(step for step in steps if step.get("id") == "verification-cache")["with"]
        prefix = "verified-v3-${{ steps.cache-environment.outputs.image }}-"
        self.assertIn(prefix, cache["key"])
        restores = cache["restore-keys"].splitlines()
        self.assertIn(prefix, restores[0])
        self.assertIn(prefix, restores[1])
        self.assertIn("verified-v2-", restores[2])


if __name__ == "__main__":
    unittest.main()
