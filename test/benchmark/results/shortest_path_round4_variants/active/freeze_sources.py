import hashlib,os,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[5]

def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()

def freeze(destination, driver, source=ROOT, revision=None):
    destination.mkdir(parents=True, exist_ok=True)
    pending = [Path('graph/dijkstra.hpp'), Path('components/fast_io.hpp')]
    copied = {}
    for include in re.findall(r'^\s*#include\s+"([^"\r\n]+)"', driver, re.M):
        pending.append(Path(include))
    while pending:
        relative = Path(os.path.normpath(pending.pop()))
        key = relative.as_posix()
        if key in copied:
            continue
        if revision:
            result = subprocess.run(['git', 'show', revision + ':' + key], cwd=ROOT, capture_output=True)
            if result.returncode:
                continue
            data = result.stdout
        else:
            if not (source / relative).exists():
                continue
            data = (source / relative).read_bytes()
        data = data.replace(b'\r\n', b'\n')
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        copied[key] = sha(target)
        for include in re.findall(rb'^\s*#include\s+"([^"\r\n]+)"', data, re.M):
            pending.append(relative.parent / include.decode())
    return copied

def freeze_reference(destination, source, revision=None):
    destination.mkdir(parents=True, exist_ok=True)
    copied = {}
    files = {}

    def copy(path):
        path = path.resolve()
        if path in files:
            return files[path]
        target = destination / (str(len(files)) + path.suffix)
        files[path] = target
        data = subprocess.check_output(['git', 'show', revision + ':' + path.relative_to(ROOT).as_posix()], cwd=ROOT).decode().replace('\r\n', '\n') if revision else path.read_text()
        key = str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)
        copied[key] = hashlib.sha256(data.encode()).hexdigest()

        def include(match):
            included = path.parent / match[2]
            if not included.exists():
                included = ROOT / match[2]
            if not included.exists():
                raise FileNotFoundError(included)
            return match[1] + copy(included).name + match[3]

        data = re.sub(r'(^\s*#include\s+")([^"\r\n]+)(")', include, data, flags=re.M)
        target.write_text(data)
        return target

    translation = copy(source)
    manifest = {str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path): target.name for path, target in files.items()}
    return translation, copied, manifest
