#!/usr/bin/env python3
import os
import subprocess
import sys

binary = sys.argv[1]
n = sys.argv[2] if len(sys.argv) > 2 else "100000000"
print("n,kind,count,last_prime,max_rss_kib")
for kind in (0, 1):
    process = subprocess.Popen([binary, n, "1", "1", str(kind)], stdout=subprocess.PIPE, text=True)
    output = process.stdout.read().strip()
    _, status, usage = os.wait4(process.pid, 0)
    process.returncode = os.waitstatus_to_exitcode(status)
    if process.returncode:
        raise SystemExit(process.returncode)
    rss = usage.ru_maxrss / 1024 if sys.platform == "darwin" else usage.ru_maxrss
    print(f"{output},{rss:g}", flush=True)
