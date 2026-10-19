import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = (root / 'data_structure/priority_queue/retroactive_priority_queue.hpp').read_text().replace('"../components/splitmix64.hpp"', '"components/splitmix64.hpp"')
a = source.index('\tbool present(')
b = source.index('\n\tint query(', a)
forward = '''	bool present(int key, int full, const Summary *partial) const {
		int count = 0;
		for (int i = 0; i < full; ++i) {
			const auto &v = blocks[i];
			count = rank(v.alive, key) + std::max(0, count + rank(v.dead, key) - v.pops);
		}
		if (partial) count = rank(partial->alive, key) + std::max(0, count + rank(partial->dead, key) - partial->pops);
		return count > 0;
	}
'''
source = (source[:a] + forward + source[b:]).replace('RETROACTIVE_PRIORITY_QUEUE_HPP', 'RETROACTIVE_PRIORITY_QUEUE_FORWARD_HPP').replace('RetroactivePriorityQueue', 'ForwardPriorityQueue')
(out / 'forward.hpp').write_text(source)
unit = (root / 'test/unit/retroactive_priority_queue.cpp').read_text().replace('"data_structure/priority_queue/retroactive_priority_queue.hpp"', '"forward.hpp"').replace('RetroactivePriorityQueue', 'ForwardPriorityQueue')
(out / 'forward_test.cpp').write_text(unit)
