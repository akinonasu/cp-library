import hashlib
import json
import re
import sys
import urllib.request
from pathlib import Path

out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = json.load(urllib.request.urlopen('https://v3.api.judge.yosupo.jp/submissions/410724', timeout=60))['source']
(out / 'reference-source.cpp').write_text(source)
(out / 'reference-source.sha256').write_text(hashlib.sha256(source.encode()).hexdigest() + '\n')
source = source[:source.index('__attribute__((always_inline)) inline int read_int()')]
source = re.sub(r'^#(?:pragma|include).*\n', '', source, flags=re.M)
source = source.replace('const vector<int>& a', 'const int* a').replace('a.begin(), a.begin() + n', 'a, a + n').replace('a.data()', 'a')
source = '#include <bits/stdc++.h>\nnamespace hungarian_reference_code {\n' + source + '\n}\n'
source += '''
struct Hungarian_reference {
	long long cost;
	std::vector<int> row_to_col, col_to_row;
	Hungarian_reference(int n, int m, const int *a) {
		assert(n == m);
		(void)m;
		auto result = hungarian_reference_code::hungarian(n, a);
		cost = result.first;
		row_to_col = std::move(result.second);
		col_to_row.resize(n);
		for (int i = 0; i < n; ++i) col_to_row[row_to_col[i]] = i;
	}
};
'''
(out / 'reference.hpp').write_text(source)
