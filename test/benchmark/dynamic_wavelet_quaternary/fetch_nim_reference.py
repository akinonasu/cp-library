#!/usr/bin/env python3
"""Rebuild the pinned CC0-1.0 kemuniku/cplib Nim reference, without vendoring it.

Library: https://github.com/kemuniku/cplib
Compiler flags: https://img.atcoder.jp/file/language-update/2025-10/language-list.html
The main follows the supplied ABC343 F submission; an unused originalSource
metadata template is omitted. checks:off stays after dependency definitions.
A local Nim2.2.4 toolchain is supplied by --nim; this helper downloads no toolchain.
"""
import argparse
import datetime
import fcntl
import hashlib
import json
from pathlib import Path
import subprocess
import urllib.request

PIN = "c95c3c35f66357921a0d3bcd8cb79cbc7b13d6c3"
BASE = "https://raw.githubusercontent.com/kemuniku/cplib/" + PIN
FILES = {'/src/cplib/collections/dynamic_quaternary_waveletmatrix.nim': '90aedcfd100cc02b61a9735488f185336ed6a88c2453f967fed4da282326ee5b', '/src/cplib/collections/dynamic_quaternary_vector.nim': '4ce47b576695ae636c80754206fac652b7addfbb32c72633a7749d7b08d067ce', '/src/cplib/collections/dynamic_quaternary_kernels.nim': '294c16755e6de8cc397aa7bce4daf1c2a1f0e1bf673d7e0369da39bba2ac9d8a', '/src/cplib/collections/dynamic_bplus_kernels.nim': '0ddbaa6418a1c3e482c9fbc1aba6b3c3b1c1b0d04abc2fbf343bb020c1f1c34e', '/src/cplib/tmpl/sheep.nim': 'b6b1af04e745a12e5041739e8648352a00ba5c166622dde0ca2106a85a2ba20a', '/src/cplib/tmpl/fastio.nim': '075bfbe1736976148ac7adc6157a6e1cff0aa15d1a9ee19db5910552aa5aa063', '/src/cplib/utils/constants.nim': 'b9451e5d994497483e22ff9936876b31ffea94c13cdd922521805871875fdf4d', '/LICENSE': 'a2010f343487d3f7618affe54f789f5487602331c0a8d03f49e9a7c547cf0499'}
PRELUDE = r'''{.define: quadOptimized.}
{.define: quadLeafPrefix.}
include cplib/tmpl/sheep
import cplib/collections/dynamic_quaternary_waveletmatrix
import cplib/collections/dynamic_quaternary_vector

'''
STREAM_MAIN = r'''{.checks: off.}
let N = ii()
let Q = ii()
let A = lii(N)
let wm = initDynamicQuadWaveletMatrix(A, QuadVector[32, 256], H = 30)
for _ in 0..<Q:
    let t = ii()
    if t == 1:
        let p = ii()
        let x = ii()
        wm[p - 1] = x
    else:
        let l = ii()
        let r = ii()
        let mx = wm.kth_largest(l - 1, r, 0)
        let x = wm.prev_value(l - 1, r, mx)
        if x.isNone:
            print(0)
        else:
            print(wm.count(l - 1, r, x.get()))
'''
API_MAIN = r'''import std/times
{.checks: off.}
let N = ii()
let Q = ii()
let A = lii(N)
var operations = newSeq[tuple[kind, a, b: int]](Q)
var answers = newSeq[int](Q)
for i in 0..<Q:
    operations[i] = (ii(), ii(), ii())
let startWall = epochTime()
let startCpu = cpuTime()
let wm = initDynamicQuadWaveletMatrix(A, QuadVector[32, 256], H = 30)
let builtCpu = cpuTime()
let builtWall = epochTime()
var answerCount = 0
for op in operations:
    if op.kind == 1:
        wm[op.a - 1] = op.b
    else:
        let l = op.a - 1
        let r = op.b
        let mx = wm.kth_largest(l, r, 0)
        let x = wm.prev_value(l, r, mx)
        answers[answerCount] = if x.isNone: 0 else: wm.count(l, r, x.get())
        inc answerCount
let doneCpu = cpuTime()
let doneWall = epochTime()
stderr.writeLine("{\"build_cpu\":", builtCpu - startCpu,
    ",\"operation_cpu\":", doneCpu - builtCpu,
    ",\"build_wall\":", builtWall - startWall,
    ",\"operation_wall\":", doneWall - builtWall,
    ",\"answers\":", answerCount, "}")
for i in 0..<answerCount:
    print(answers[i])
'''
EXPECTED = {'BundledMain.nim': '1ce8527d48a30fa97168582da0b963e5e97f6ad2c157f52f50b71f3457ebe47f', 'abc343f_api.nim': '7f369dabcb8386c27b2dc51c19118515f1730d94a8ffb36bd04cf48807e54a43'}

FLAGS = ["cpp", "-d:release", "--opt:speed", "--multimethods:on",
         "--warning[SmallLshouldNotBeUsed]:off", "--hints:off",
         "--maxLoopIterationsVM:10000000000000", "--maxCallDepthVM:10000000000000"]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def expand(text, directory):
    result = []
    for line in text.splitlines(keepends=True):
        stripped = line.strip()
        if stripped.startswith(("import cplib/", "include cplib/")):
            name = stripped.split(None, 1)[1]
            if "," in name or "#" in name:
                raise ValueError("Unsupported library directive: " + stripped)
            indentation = line[:len(line) - len(line.lstrip())]
            child = expand((directory / (name + ".nim")).read_text(), directory)
            result.extend(indentation + part if part.strip() else part
                          for part in child.splitlines(keepends=True))
        else:
            result.append(line)
    return "".join(result)


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--directory", type=Path, required=True)
    parser.add_argument("--nim", type=Path, required=True)
    parser.add_argument("--compile", action="store_true")
    parser.add_argument("--lock", type=Path, default=Path("/tmp/cp-fps-tuning.lock"))
    args = parser.parse_args()
    directory, nim = args.directory.resolve(), args.nim.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    version = subprocess.check_output([str(nim), "--version"], text=True)
    if "Nim Compiler Version 2.2.4 " not in version:
        raise RuntimeError("The pinned reference requires Nim2.2.4")
    records = []
    for remote, expected in FILES.items():
        relative = remote.removeprefix("/src/") if remote.startswith("/src/") else "LICENSE"
        target = directory / relative
        fetched = not target.exists()
        if fetched:
            with urllib.request.urlopen(BASE + remote, timeout=60) as response:
                data = response.read()
        else:
            data = target.read_bytes()
        if sha(data) != expected:
            raise RuntimeError("Pinned source SHA256 mismatch: " + relative)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        records.append({"url": BASE + remote, "path": relative,
                        "sha256": expected, "bytes": len(data), "fetched": fetched})
    expanded = expand(PRELUDE, directory)
    sources = {"BundledMain.nim": expanded + STREAM_MAIN,
               "abc343f_api.nim": expanded + API_MAIN}
    (directory / "Main.nim").write_text(PRELUDE + STREAM_MAIN)
    for name, text in sources.items():
        if sha(text.encode()) != EXPECTED[name]:
            raise RuntimeError("Reconstructed bundle SHA256 mismatch: " + name)
        (directory / name).write_text(text)
    commands = {}
    for name, binary, cache in [("BundledMain.nim", "abc343f-bundled", "bundled-nimcache"),
                                ("abc343f_api.nim", "nim-api", "nim-api-cache")]:
        commands[binary] = [str(nim), *FLAGS, "--nimcache:" + str(directory / cache),
                            "--out:" + str(directory / binary), str(directory / name)]
    if args.compile:
        with args.lock.open("w") as lock:
            fcntl.flock(lock, fcntl.LOCK_SH)
            for command in commands.values():
                subprocess.run(command, check=True, cwd=directory)
    result = {"utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
              "library_commit": PIN, "license": "CC0-1.0", "sources": records,
              "bundle_sha256": EXPECTED, "nim_version": version,
              "nim_binary_sha256": sha(nim.read_bytes()), "commands": commands,
              "compiled": args.compile, "helper_sha256": sha(Path(__file__).read_bytes())}
    if args.compile:
        result["binary_sha256"] = {name: sha((directory / name).read_bytes()) for name in commands}
    (directory / "reconstruction.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"directory": str(directory), "bundle_sha256": EXPECTED,
                      "commands": commands, "compiled": args.compile}, indent=2))


if __name__ == "__main__":
    main()
