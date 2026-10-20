#!/usr/bin/env python3
"""Run one command with inherited I/O and write its peak child RSS in KiB (Linux)."""
from pathlib import Path
import resource
import subprocess
import sys

if __name__ == '__main__':
    result = subprocess.run(sys.argv[2:])
    Path(sys.argv[1]).write_text(str(resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss) + '\n')
    raise SystemExit(result.returncode)
