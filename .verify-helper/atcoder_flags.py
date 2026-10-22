from functools import lru_cache
import subprocess


@lru_cache(maxsize=None)
def constexpr_flags(compiler):
    version = subprocess.check_output([compiler, '--version'], text=True).lower()
    flags = ['-fconstexpr-depth=1024']
    if 'clang' in version:
        flags += ['-fconstexpr-steps=524288']
    else:
        flags += ['-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152']
    return flags
