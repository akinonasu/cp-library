#include FPS_IO_PROBE_REFERENCE
#include "components/fast_io.hpp"
#include "math/mint.hpp"
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

unsigned long long checksum(const std::vector<unsigned int> &a) {
	unsigned long long result = 1469598103934665603ULL;
	for (auto x : a) result = (result ^ x) * 1099511628211ULL;
	return result;
}

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	std::string operation = argv[1], variant = argv[2];
	std::vector<unsigned int> values;
	std::vector<mint> transformed;
	auto begin = std::chrono::steady_clock::now();
	if (operation == "read") {
		if (variant.starts_with("own")) {
			FastInput input;
			unsigned n = 0, m = 0;
			input.read(n); input.read(m);
			values.resize(n + m);
			if (variant == "own_mint")
				for (auto &x : values) input.read(x), x = mint::raw(x).v;
			else
				for (auto &x : values) input.read(x);
		} else {
			io::Reader input;
			auto n = input.read<unsigned>(), m = input.read<unsigned>();
			values.resize(n + m);
			if (variant == "ref_fixed") read_values(input, values.data(), n), read_values(input, values.data() + n, m);
			else if (variant == "ref_bulk") io::read_bulk(input, values.data(), n), io::read_bulk(input, values.data() + n, m);
			else for (auto &x : values) x = input.read<unsigned>();
		}
	} else if (operation == "write") {
		std::uint64_t n = 0;
		if (std::fread(&n, sizeof(n), 1, stdin) != 1) return 2;
		values.resize(n);
		if (std::fread(values.data(), sizeof(unsigned), n, stdin) != n) return 2;
		if (variant == "own_mint") {
			transformed.resize(n);
			for (std::size_t i = 0; i < n; ++i) transformed[i] = mint::raw(values[i]);
		}
		begin = std::chrono::steady_clock::now();
		if (variant.starts_with("own")) {
			FastOutput output;
			if (variant == "own_mint")
				for (auto x : transformed) output.write(x.val()), output.write(' ');
			else
				for (auto x : values) output.write(x), output.write(' ');
			output.write('\n');
		} else {
			io::Writer output;
			if (variant == "ref_fields") {
				alignas(64) static char text[fields::kTextBytes];
				fields::write(output, values.data(), values.size(), text);
			} else {
				for (auto x : values) output.write(x, ' ');
				output.write('\n');
			}
		}
	} else return 2;
	auto end = std::chrono::steady_clock::now();
	if (operation == "read" && variant == "own_mint") for (auto &x : values) x = mint::word(x).val();
	std::fprintf(stderr, "%.6f %llu %zu\n", std::chrono::duration<double, std::milli>(end - begin).count(), checksum(values), values.size());
}
