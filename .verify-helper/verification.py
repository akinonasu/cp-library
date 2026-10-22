from pathlib import Path
import re
from urllib.parse import urlparse


def check_judges(root, paths):
    for name in paths:
        source = (root / name).read_text()
        for url in re.findall(r'^\s*#\s*define\s+PROBLEM\s+"([^"\n]+)"', source, re.M):
            host = (urlparse(url).hostname or "").lower().rstrip(".")
            if host == "atcoder.jp" or host.endswith(".atcoder.jp"):
                raise ValueError("AtCoder judge tests are not allowed: " + str(name))


def get_marker():
    from onlinejudge_verify import marker
    from onlinejudge_verify.languages import list as languages

    languages.get(Path("source.cpp"))
    return marker.get_verification_marker(jobs=2)
