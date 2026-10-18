#include "profile.hpp"
#include "convolution/convolution.hpp"
#include "test/fps_io.hpp"

#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC target("avx2")
namespace profile_public {
using namespace fps_detail;

fps read(FastInput& input, int n) {
	fps out;
	if (!n) return out;
	if (std::size_t(n) > out.max_size() - out.size()) throw std::length_error("fps input");
	{
		prepare_fps_output(out, out.size() + n);
		auto values = std::make_unique_for_overwrite<unsigned[]>(std::min<std::size_t>(32768, n));
		profile::step(profile::InputPublicAlloc);
		for (std::size_t i = 0; i < std::size_t(n); i += 32768) {
			const auto count = std::min<std::size_t>(32768, n - i);
			fast_io_detail::BulkInput::read(input, values.get(), count);
			profile::step(profile::InputParse);
			append_fps_words<false>(out, values.get(), count);
			profile::step(profile::InputMintBuild);
		}
	}
	profile::step(profile::InputScratchRelease);
	return out;
}

std::vector<Mint> multiply(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	using namespace multiply_detail;
	assert(!a.empty() && !b.empty() && a.size() + b.size() - 1 <= (1 << 23));
	assert(Product::fits(a.size(), b.size()) && NTT::vectorized());
	profile::FinalStep release{profile::PlanRelease};
	Product plan(a.size(), b.size());
	const auto len = std::size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	profile::step(profile::PlanAllocInit);
	copy_input(a, plan.a(), len);
	copy_input(b, plan.b(), len);
	profile::step(profile::CopyInput);
	const auto* p = plan.multiply();
	profile::step(profile::MultiplyRoots);
	std::vector<Mint> result;
	const std::size_t size = a.size() + b.size() - 1;
	prepare_fps_output(result, size);
	append_fps_words<true>(result, p, size);
	profile::step(profile::ResultVector);
	return result;
}

void write(FastOutput& output, const Mint* in, std::size_t n) {
	alignas(32) unsigned values[25600];
	alignas(32) char text[256000];
	const auto &k = fps_io_fields::detail::kConstants;
	profile::step(profile::OutputSetup);
	for (std::size_t i = 0; i < n; i += 25600) {
		const auto count = std::min<std::size_t>(25600, n - i), full = count / 16 * 16;
		std::size_t j = 0;
		for (; j + 8 <= count; j += 8) {
			alignas(32) unsigned words[8];
			std::memcpy(words, in + i + j, sizeof(words));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(values + j), io_values(_mm256_load_si256(reinterpret_cast<const __m256i *>(words))));
		}
		for (; j < count; ++j) values[j] = in[i + j].val();
		profile::step(profile::OutputConvert);
		if (full) fps_io_fields::detail::format(text, values, full, k);
		if (full < count) {
			alignas(32) unsigned rest[16] = {};
			alignas(16) char last[160];
			std::copy_n(values + full, count - full, rest);
			fps_io_fields::detail::format(last, rest, 16, k);
			std::memcpy(text + 10 * full, last, 10 * (count - full));
		}
		if (i + count == n) text[10 * count - 1] = '\n';
		profile::step(profile::OutputFormat);
		fast_io_detail::BulkOutput::write(output, std::string_view(text, 10 * count));
		profile::step(profile::OutputWrite);
	}
	if (!n) output.write('\n');
}
}

#pragma GCC pop_options

static void write_profiled(FastOutput& output, const fps& f) {
	profile_public::write(output, f.data(), f.size());
}

int main() {
	profile::start();
	{
		FastInput input;
		{
			FastOutput output;
			int n = 0, m = 0; input.read(n); input.read(m);
			profile::step(profile::IoSetupHeader);
			{
				auto a = profile_public::read(input, n), b = profile_public::read(input, m);
				write_profiled(output, profile_public::multiply(a, b));
				profile::step(profile::ResultRelease);
			}
			profile::step(profile::PublicInputsRelease);
		}
		profile::step(profile::WriterFlushRelease);
	}
	profile::step(profile::InputRelease);
	profile::emit();
}
