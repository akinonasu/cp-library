import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = (root / 'test/benchmark/generalized_floor_sum_base.hpp').read_text().replace('FLOOR_BASE_HPP', 'GENERALIZED_FLOOR_SUM_HPP').replace('floor_base', 'generalized_floor_sum')
raw = source.replace('divide<2>(count * T(n - 1))', 'T(n >> 1) * T(n - 1 + (n & 1))')
raw = raw.replace('divide<2>(T(n) * T(n - 1)) * T(a / m)', 'T(n >> 1) * T(n - 1 + (n & 1)) * T(a / m)')
one = source.replace('''	const auto &c = coefficients<D, T>();
	std::array<T, D + 1> sums''', '''	if (n == 1) {
		result[0][0] = T(1);
		T value = T(b / m);
		for (int q = 1; q <= D; ++q) result[0][q] = result[0][q - 1] * value;
		return result;
	}
	const auto &c = coefficients<D, T>();
	std::array<T, D + 1> sums''')
one = one.replace('''	T count = T(n), s1 =''', '''	if (n == 1) {
		T value = T(b / m);
		if constexpr (Full) result[0][0] = T(1), result[0][1] = value, result[0][2] = value * value;
		else result = {value, T(0), value * value};
		return result;
	}
	T count = T(n), s1 =''')
one = one.replace('''		if (!n) return result;
		if (a >= m)''', '''		if (!n) return result;
		if (n == 1) return result + T(b / m);
		if (a >= m)''')
rawone = one.replace('divide<2>(count * T(n - 1))', 'T(n >> 1) * T(n - 1 + (n & 1))')
rawone = rawone.replace('divide<2>(T(n) * T(n - 1)) * T(a / m)', 'T(n >> 1) * T(n - 1 + (n & 1)) * T(a / m)')
for name, text in [('raw', raw), ('one', one), ('rawone', rawone)]:
    text = text.replace('GENERALIZED_FLOOR_SUM_HPP', 'FLOOR_' + name.upper() + '_HPP')
    text = text.replace('generalized_floor_sum', 'floor_' + name)
    (out / (name + '.hpp')).write_text(text)
