import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = (root / 'test/benchmark/hungarian_baseline.hpp').read_text()
unit = (root / 'test/unit/hungarian.cpp').read_text()
monge_start = source.index('\t\tif (n == m) {')
monge_end = source.index('\t\tstd::vector<Cost> price', monge_start)
reduced = (source[:monge_start] + source[monge_end:]).replace('if (count[i] == 1)', 'if (count[i] == 1 && n > 1)')
init_start = reduced.index('\t\tif (n == m) {')
init_end = reduced.index('\t\tstd::iota(columns', init_start)
rect_start = reduced.index('\n\t\t} else {', init_start)
greedy = reduced[:init_start] + '\t\t{\n' + reduced[rect_start + len('\n\t\t} else {\n'):init_end] + reduced[init_end:]
dense = reduced[:init_start] + reduced[init_end:]
pass_start = reduced.index('\t\t\tfor (int pass = 0; pass < 2; ++pass)')
column = reduced[:pass_start] + reduced[rect_start:]
transfer_start = reduced.index('\t\t\tfor (int i = 0; i < n; ++i) if (count[i] == 1')
no_transfer = reduced[:transfer_start] + reduced[pass_start:]
ascending = source.replace('for (int j = m - 1; j >= 0; --j)', 'for (int j = 0; j < m; ++j)')
queue = source
start = queue.index('\t\t\tfor (int pass = 0; pass < 2; ++pass)')
end = queue.index('\n\t\t} else {', start)
body = queue[start:end]
a = body.index('\t\t\t\t\tint best_col = 0;')
b = body.index('\n\t\t\t\t}', a)
body = body[a:b]
body = body.replace('if (old != -1) row_to_col[old] = -1;', '''if (old != -1) {
						row_to_col[old] = -1;
						if (best < second) todo[--head] = old;
						else todo.push_back(old);
					}''')
queue = queue[:start] + '''			std::vector<int> todo;
			todo.reserve(3 * n);
			for (int i = 0; i < n; ++i) if (row_to_col[i] == -1) todo.push_back(i);
			for (int head = 0, work = 0; head < int(todo.size()) && work < 2 * n; ++work) {
				int i = todo[head++];
''' + body + '\n\t\t\t}' + queue[end:]
classic_start = source.index('\t\tif (n == m) {')
classic_end = source.rindex('\t\tfinish();')
classic = source[:classic_start] + '''		std::vector<Cost> u(n + 1), v(m + 1), slack(m + 1);
		std::vector<int> p(m + 1), prev(m + 1);
		std::vector<unsigned char> used(m + 1);
		for (int root = 1; root <= n; ++root) {
			p[0] = root;
			std::fill(slack.begin(), slack.end(), std::numeric_limits<Cost>::max());
			std::fill(used.begin(), used.end(), 0);
			int col = 0;
			do {
				used[col] = 1;
				int row = p[col], next = 0;
				Cost delta = std::numeric_limits<Cost>::max();
				for (int j = 1; j <= m; ++j) if (!used[j]) {
					Cost x = value(row - 1, j - 1) - u[row] - v[j];
					if (x < slack[j]) slack[j] = x, prev[j] = col;
					if (slack[j] < delta) delta = slack[j], next = j;
				}
				for (int j = 0; j <= m; ++j) {
					if (used[j]) u[p[j]] += delta, v[j] -= delta;
					else if (j) slack[j] -= delta;
				}
				col = next;
			} while (p[col]);
			do {
				int next = prev[col];
				p[col] = p[next];
				col = next;
			} while (col);
		}
		for (int j = 1; j <= m; ++j) if (p[j]) row_to_col[p[j] - 1] = j - 1, col_to_row[j - 1] = p[j] - 1;
''' + source[classic_end:]
narrow_start = source.index('\t\t\tfor (int j = 0; j < m; ++j) price[j] = value(0, j);')
narrow_end = source.index('\t\t\tfor (int j = m - 1;', narrow_start)
narrow = source[:narrow_start] + '''			std::vector<T> minimum(a[0], a[0] + m);
			std::vector<int> owner(m), count(n);
			for (int i = 1; i < n; ++i)
				for (int j = 0; j < m; ++j) {
					T x = a[i][j];
					bool better = maximize ? x > minimum[j] : x < minimum[j];
					minimum[j] = better ? x : minimum[j];
					owner[j] = better ? i : owner[j];
				}
			for (int j = 0; j < m; ++j) price[j] = maximize ? -Cost(minimum[j]) : Cost(minimum[j]);
''' + source[narrow_end:]
wide_type = 'std::conditional_t<(sizeof(Cost) < 16), __int128_t, Cost>'
input_type = 'std::conditional_t<(sizeof(T) < 8), long long, __int128_t>'
narrow_order = narrow.replace(wide_type, input_type)
free_only = narrow_order.replace('x == best && col_to_row[best_col] != -1', 'x == best && col_to_row[best_col] != -1 && col_to_row[j] == -1')
rect_start = narrow_order.index('\n\t\t} else {', narrow_order.index('std::vector<Cost> price'))
rect_end = narrow_order.index('\t\tstd::iota(columns', rect_start)
rect_scan = narrow_order[:rect_start] + '''
		} else {
			for (int i = 0; i < n; ++i) {
				T best = a[i][0];
				for (int j = 1; j < m; ++j) best = maximize ? std::max(best, a[i][j]) : std::min(best, a[i][j]);
				for (int j = 0; j < m; ++j) if (a[i][j] == best && col_to_row[j] == -1) {
					row_to_col[i] = j;
					col_to_row[j] = i;
					break;
				}
			}
		}
''' + narrow_order[rect_end:]
variants = dict(classic=classic, dense=dense, greedy=greedy, column=column, no_transfer=no_transfer, reduced=reduced, ascending=ascending, queue=queue, current=(root / 'graph/hungarian.hpp').read_text(), baseline=source, narrow=narrow, narrow_order=narrow_order, free_only=free_only, rect_scan=rect_scan, queue_half=queue.replace('work < 2 * n', 'work < n'))
for name, code in variants.items():
    code = code.replace('std::min(delta, value(i, j) - price[j])', 'std::min(delta, Cost(value(i, j) - price[j]))')
    code = code.replace('HUNGARIAN_HPP', 'HUNGARIAN_' + name.upper() + '_HPP').replace('Hungarian', 'Hungarian_' + name)
    (out / (name + '.hpp')).write_text(code)
    (out / (name + '_test.cpp')).write_text(unit.replace('"graph/hungarian.hpp"', '"' + name + '.hpp"').replace('Hungarian', 'Hungarian_' + name))
