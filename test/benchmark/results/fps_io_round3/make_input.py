import argparse
import importlib.util
from pathlib import Path


def make(original, tail_batch=False, forward_emit=False):
    script = Path(__file__).with_name('harness_v1.py')
    spec = importlib.util.spec_from_file_location('fps_io_round3_v1', script)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    source = module.variants(original)['fused']
    begin = source.index('\tstatic std::size_t chunk(')
    end = source.index('\ttemplate <class Emit>\n\tstatic std::size_t chunk_blocks', begin)
    source = source[:begin] + source[end:]
    source = source.replace('std::size_t count, unsigned *scratch, Emit emit)', 'std::size_t count, unsigned *const (&out)[4], Emit emit)')
    source = source.replace('\t\tunsigned *const out[4] = {scratch, scratch + kCapacity, scratch + 2 * kCapacity, scratch + 3 * kCapacity};\n', '')
    wrapper = '''	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

'''
    source = source.replace('\ttemplate <class Emit>\n\tstatic void read_blocks', wrapper + '\ttemplate <class Emit>\n\tstatic void read_blocks')
    source = source.replace('const auto parsed = chunk_blocks(p, in.end, count - done, scratch.get(), emit);',
                            'unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};\n\t\t\t\tconst auto parsed = chunk_blocks(p, in.end, count - done, out, emit);')
    if tail_batch:
        source = source.replace('\t\t\twhile (done + 2 <= count', '\t\t\tunsigned tail[64];\n\t\t\tstd::size_t used = 0;\n\t\t\twhile (done + 2 <= count')
        old = '\t\t\t\tconst unsigned values[2] = {h0 * 100000000 + l0, h1 * 100000000 + l1};\n\t\t\t\temit(values, 2);'
        new = '''				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}'''
        assert source.count(old) == 1
        source = source.replace(old, new)
        source = source.replace('\t\t\tin.current = p;', '\t\t\tif (used) emit(tail, used);\n\t\t\tin.current = p;')
    if forward_emit:
        source = source.replace('std::size_t count, unsigned *const (&out)[4], Emit emit)',
                                'std::size_t count, unsigned *const (&out)[4], Emit &&emit)')
    return source


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--tail-batch', action='store_true')
    parser.add_argument('--forward-emit', action='store_true')
    args = parser.parse_args()
    args.output.write_text(make(args.input.read_text(), args.tail_batch, args.forward_emit))


if __name__ == '__main__':
    main()
