import io
from pathlib import Path
import subprocess
import tarfile

BASELINE = 'fd41ae5df33960b93288ae98f9edac4e9fdb166f'


def prepare(root: Path):
    target = root / '.verify-helper/cache/fps-tuning-before'
    target.mkdir(parents=True, exist_ok=True)
    data = subprocess.check_output(['git', 'archive', BASELINE, 'fps', 'components', 'math', 'convolution'], cwd=root)
    with tarfile.open(fileobj=io.BytesIO(data)) as archive:
        for member in archive:
            if member.isfile():
                path = target / member.name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(archive.extractfile(member).read())
    return target
