from pathlib import Path
import shutil
W=Path('/workspace/cp-fps-round4/borrow');C=W/'range_lazy';shutil.copytree(W/'range_import',C);H=C/'components/fps_convolution.hpp';s=H.read_text()
a=s.index('\t\tconst auto input = [](Vec value)');b=s.index('\n\t\tf[j] = add(g0, g1)',a)
s=s[:a]+'''		const Vec f0 = f[j], f1 = f[j + q], f2 = f[j + 2 * q], f3 = f[j + 3 * q];
		const Vec g0 = normalize ? reduce(add(f0, f2), 2 * kP) : add(f0, f2);
		const Vec g1 = normalize ? reduce(add(f1, f3), 2 * kP) : add(f1, f3);
		const Vec h0 = normalize ? reduce(diff(f0, f2), 2 * kP) : diff_canonical(f0, f2);
		const Vec ih1 = times(normalize ? diff(f1, f3) : diff_canonical(f1, f3), i);'''+s[b:]
H.write_text(s)
