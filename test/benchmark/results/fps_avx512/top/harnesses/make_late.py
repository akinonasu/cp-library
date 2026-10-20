from pathlib import Path
p=Path('/tmp/fps-avx512-top/reordered_late/components/fps_blocked.hpp');s=p.read_text();s=s.replace('''		const Vec odd_a = times(amb, s), odd_b = times(cmd, zs);
		if (output == Half::kBoth) {
			const Vec even_a = times(ab, s), even_b = times(cd, s);
			f[j] = canonical(add(even_a, even_b)), f[j + h] = canonical(add(odd_a, odd_b));
			f[j + 2 * h] = canonical(diff(even_a, even_b)), f[j + 3 * h] = canonical(diff(odd_a, odd_b));
		} else if (output == Half::kLower) {
			f[j] = reduce(times(add(ab, cd), s), kP), f[j + h] = canonical(add(odd_a, odd_b));
		} else {
			f[j + 2 * h] = reduce(times(diff(ab, cd), s), kP), f[j + 3 * h] = canonical(diff(odd_a, odd_b));
		}''','''		if (output != Half::kUpper) f[j] = reduce(times(add(ab, cd), s), kP);
		if (output != Half::kLower) f[j + 2 * h] = reduce(times(diff(ab, cd), s), kP);
		const Vec odd_a = times(amb, s), odd_b = times(cmd, zs);
		if (output != Half::kUpper) f[j + h] = canonical(add(odd_a, odd_b));
		if (output != Half::kLower) f[j + 3 * h] = canonical(diff(odd_a, odd_b));''');p.write_text(s)
