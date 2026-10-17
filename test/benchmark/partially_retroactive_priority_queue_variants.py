import sys
import subprocess
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
revision = '2a4d721b9a8fc2ed25a7cf05ff7312782a162653'
source = subprocess.check_output(['git', 'show', revision + ':data_structure/partially_retroactive_priority_queue.hpp'], cwd=root, text=True).replace('"../components/splitmix64.hpp"', '"components/splitmix64.hpp"')
unit = subprocess.check_output(['git', 'show', revision + ':test/unit/partially_retroactive_priority_queue.cpp'], cwd=root, text=True)
separate_bridges = '''
	int last_bridge(int x, int offset) const {
		if (!x) { assert(offset == 0); return 0; }
		int a = nodes[x].left, b = nodes[x].right;
		int after = offset + nodes[a].sum + nodes[x].state;
		if (after + nodes[b].low == 0) return nodes[a].size + 1 + last_bridge(b, after);
		return last_bridge(a, offset);
	}
	int first_bridge(int x, int offset) const {
		if (offset == 0) return 0;
		assert(x);
		int a = nodes[x].left;
		if (offset + nodes[a].low == 0) return first_bridge(a, offset);
		int after = offset + nodes[a].sum + nodes[x].state;
		return nodes[a].size + 1 + first_bridge(nodes[x].right, after);
	}
	int suffix_dead(int x, int k) const {
		int answer = 0;
		while (x) {
			if (k == 0) return larger(answer, nodes[x].dead);
			int n = nodes[nodes[x].left].size;
			if (k <= n) {
				answer = larger(answer, nodes[nodes[x].right].dead);
				if (nodes[x].state == 1) answer = larger(answer, x);
				x = nodes[x].left;
			} else { k -= n + 1; x = nodes[x].right; }
		}
		return answer;
	}
	int prefix_alive(int x, int k) const {
		int answer = 0;
		while (x && k) {
			if (k == nodes[x].size) return smaller(answer, nodes[x].alive);
			int n = nodes[nodes[x].left].size;
			if (k <= n) x = nodes[x].left;
			else {
				answer = smaller(answer, nodes[nodes[x].left].alive);
				if (nodes[x].value && nodes[x].state == 0) answer = smaller(answer, x);
				k -= n + 1; x = nodes[x].right;
			}
		}
		return answer;
	}
'''
separate_selection = '''
	void change(int a, int b, int key, int state) {
		assert(key);
		if (a && nodes[key].time <= maximum_time(a)) change(a, key, state);
		else change(b, key, state);
	}
	Time maximum_time(int x) const {
		while (nodes[x].right) x = nodes[x].right;
		return nodes[x].time;
	}
	int promote(int a, int b) const {
		return larger(suffix_dead(a, last_bridge(a, 0)), nodes[b].dead);
	}
	int demote(int a, int b, int offset) const {
		return smaller(nodes[a].alive, prefix_alive(b, first_bridge(b, offset)));
	}
'''

for name in ('separate_incremental', 'separate_bulk', 'fused_bulk'):
    candidate = source
    a = candidate.index('\tstruct Selection {')
    b = candidate.index('\tvoid insert_push(', a)
    select = separate_selection
    if name == 'fused_bulk':
        select = select.replace('suffix_dead(a, last_bridge(a, 0))', 'bridge_dead(a, 0)').replace('prefix_alive(b, first_bridge(b, offset))', 'bridge_alive(b, offset)')
    candidate = candidate[:a] + select + candidate[b:]
    candidate = candidate.replace('auto selected = promote(a, b);\n\t\tif (selected.key && less(x, selected.key)) {\n\t\t\tchange(selected, 0);', 'int key = promote(a, b);\n\t\tif (key && less(x, key)) {\n\t\t\tchange(a, b, key, 0);')
    candidate = candidate.replace('change(promote(a, b), 0)', 'change(a, b, promote(a, b), 0)').replace('change(demote(a, b, nodes[a].sum + 1), 1)', 'change(a, b, demote(a, b, nodes[a].sum + 1), 1)').replace('change(demote(a, b, nodes[a].sum), 1);', 'int key = demote(a, b, nodes[a].sum);\n\t\tchange(a, b, key, 1);')
    if name != 'fused_bulk':
        a = candidate.index('\tint bridge_dead(')
        b = candidate.index('\tvoid change(int x,', a)
        candidate = candidate[:a] + separate_bridges + candidate[b:]
    if name == 'separate_incremental':
        a = candidate.index('\t\tstd::vector<int> heap, stack;')
        b = candidate.index('\n\t\tswap(next);', a)
        candidate = candidate[:a] + """\t\tfor (auto &[time, value] : sorted) {
\t\t\tif (value) next.insert(time, std::move(*value));
\t\t\telse next.insert_pop(time);
\t\t}""" + candidate[b:]
    cls = 'Queue_' + name
    candidate = candidate.replace('PARTIALLY_RETROACTIVE_PRIORITY_QUEUE_HPP', name.upper() + '_PRPQ_HPP').replace('PartiallyRetroactivePriorityQueue', cls)
    (out / (name + '.hpp')).write_text(candidate)
    (out / (name + '_test.cpp')).write_text(unit.replace('"data_structure/partially_retroactive_priority_queue.hpp"', '"' + name + '.hpp"').replace('PartiallyRetroactivePriorityQueue', cls))
