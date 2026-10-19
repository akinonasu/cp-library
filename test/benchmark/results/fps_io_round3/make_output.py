import argparse
from pathlib import Path


def make(source):
    helper = '''
[[gnu::always_inline]] inline void divide_mint16(const Mint *in, fps_io_fields::detail::Divided &lo, fps_io_fields::detail::Divided &hi, const fps_io_fields::detail::Constants &k) {
	alignas(32) unsigned words[16];
	std::memcpy(words, in, sizeof(words));
	const auto a = io_values(_mm256_load_si256(reinterpret_cast<const __m256i *>(words)));
	const auto b = io_values(_mm256_load_si256(reinterpret_cast<const __m256i *>(words + 8)));
	const auto l = _mm256_permute2x128_si256(a, b, 0x20), h = _mm256_permute2x128_si256(a, b, 0x31);
	lo = fps_io_fields::detail::divide(l, _mm256_srli_epi64(l, 32), k);
	hi = fps_io_fields::detail::divide(h, _mm256_srli_epi64(h, 32), k);
}

inline void format_mint(char *p, const Mint *in, std::size_t count, const fps_io_fields::detail::Constants &k) {
	fps_io_fields::detail::Divided lo, hi;
	divide_mint16(in, lo, hi, k);
	for (std::size_t i = 16; i < count; i += 16, p += 160) {
		fps_io_fields::detail::Divided next_lo, next_hi;
		divide_mint16(in + i, next_lo, next_hi, k);
		fps_io_fields::detail::store(p, lo, hi, k);
		lo = next_lo, hi = next_hi;
	}
	fps_io_fields::detail::store(p, lo, hi, k);
}

'''
    source = source.replace('inline void read_coefficients(', helper + 'inline void read_coefficients(')
    begin = source.index('inline void write_coefficients(')
    end = source.index('\n}\n#pragma GCC pop_options', begin)
    body = '''inline void write_coefficients(FastOutput &output, const Mint *in, std::size_t n) {
	alignas(32) char text[256000];
	const auto &k = fps_io_fields::detail::kConstants;
	for (std::size_t i = 0; i < n; i += 25600) {
		const auto count = std::min<std::size_t>(25600, n - i), full = count / 16 * 16;
		if (full) format_mint(text, in + i, full, k);
		if (full < count) {
			std::array<Mint, 16> rest;
			alignas(16) char last[160];
			std::copy_n(in + i + full, count - full, rest.data());
			format_mint(last, rest.data(), 16, k);
			std::memcpy(text + 10 * full, last, 10 * (count - full));
		}
		if (i + count == n) text[10 * count - 1] = '\\n';
		fast_io_detail::BulkOutput::write(output, std::string_view(text, 10 * count));
	}
	if (!n) output.write('\\n');
}'''
    return source[:begin] + body + source[end:]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    args.output.write_text(make(args.input.read_text()))


if __name__ == '__main__':
    main()
