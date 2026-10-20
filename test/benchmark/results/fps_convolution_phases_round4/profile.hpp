#pragma once
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <unistd.h>
namespace profile {
enum Stage {
	IoSetupHeader, InputPublicAlloc, InputParse, InputMintBuild, InputScratchRelease,
	PlanAllocInit, CopyInput, MultiplyRoots, ResultVector, ResultMoveResize, PlanRelease,
	OutputSetup, OutputConvert, OutputFormat, OutputWrite, ResultRelease,
	PublicInputsRelease, WriterFlushRelease, InputRelease, Count
};
inline std::uint64_t total[Count], first, previous;
inline unsigned calls;
inline std::uint64_t now() {
	++calls;
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return std::uint64_t(t.tv_sec) * 1000000000u + t.tv_nsec;
}
[[gnu::noinline]] inline void start() { first = previous = now(); }
[[gnu::noinline]] inline void step(Stage stage) {
	const auto t = now();
	total[stage] += t - previous;
	previous = t;
}
struct FinalStep {
	Stage stage;
	~FinalStep() { step(stage); }
};
[[gnu::noinline]] inline void emit() {
	constexpr const char* names[Count] = {
		"io_setup_header", "input_public_alloc", "input_parse", "input_mint_build", "input_scratch_release",
		"plan_alloc_init", "copy_input", "multiply_roots", "result_vector", "result_move_resize", "plan_release",
		"output_setup", "output_convert", "output_format", "output_write", "result_release",
		"public_inputs_release", "writer_flush_release", "input_release"
	};
	char buffer[4096];
	int size = std::snprintf(buffer, sizeof(buffer), "{\"internal_ns\":%llu,\"clock_calls\":%u,\"phases_ns\":{", (unsigned long long)(previous - first), calls);
	for (int i = 0; i < Count; ++i)
		size += std::snprintf(buffer + size, sizeof(buffer) - size, "%s\"%s\":%llu", i ? "," : "", names[i], (unsigned long long)total[i]);
	size += std::snprintf(buffer + size, sizeof(buffer) - size, "}}\n");
	::write(2, buffer, size);
}
}
