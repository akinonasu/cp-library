import argparse
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile


parser = argparse.ArgumentParser()
parser.add_argument("--cxx", default="g++")
parser.add_argument("--plugin", type=Path, required=True)
parser.add_argument("--jobs", type=int, default=2)
parser.add_argument("--selection", type=Path)
args = parser.parse_args()
compiler = shutil.which(args.cxx)
if compiler is None:
    parser.error("compiler not found: " + args.cxx)
compiler = str(Path(compiler).resolve())
if not args.plugin.is_file():
    parser.error("plugin not found: " + str(args.plugin))
if args.jobs < 1:
    parser.error("--jobs must be positive")

with tempfile.TemporaryDirectory(prefix="yukicoder-verify-") as directory:
    wrapper = Path(directory) / "cxx"
    wrapper.write_text("#!/bin/sh\nexec " + shlex.quote(compiler)
                       + " -DONLINE_JUDGE -fplugin=" + shlex.quote(str(args.plugin.resolve()))
                       + ' "$@" -Wno-error\n')
    wrapper.chmod(0o755)
    command = [sys.executable, str(Path(__file__).with_name("verify.py")),
               "--unit-only", "--cxx", str(wrapper), "--jobs", str(args.jobs)]
    if args.selection:
        command += ["--selection", str(args.selection.resolve())]
    subprocess.run(command, check=True)
