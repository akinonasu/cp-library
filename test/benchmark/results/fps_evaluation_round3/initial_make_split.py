from pathlib import Path
import shutil
W=Path('/tmp/fps-evaluation-round3')
for name in ['split','split_noodd']:
 C=W/name;shutil.copytree(W/'base',C,dirs_exist_ok=True);p=C/'components/fps_taylor_blocked.hpp';s=p.read_text()
 lo=s.index('\tvoid weights(std::uint32_t c) {');hi=s.index('\n\tvoid output()',lo)
 s=s[:lo]+'''	void weights(std::uint32_t c) {
		const std::size_t stride = chunk_;
		{
			Chain input([this](int s) { return mont(factorial_[s]); },
						[this](int s) { return mont(std::uint32_t(start(s) + 1)); }, kR);
			Block forward[2];
#pragma GCC unroll 8
			for (int t = 0; t < 8; ++t) input.advance(forward[0], t);
			for (std::size_t j = 0, cur = 0; j < stride; j += 8, cur ^= 1) {
				std::uint32_t* const a = a_ + j;
#pragma GCC unroll 8
				for (int t = 0; t < 8; ++t) {
					input.advance(forward[cur ^ 1], t);
					const int v = kVectors * t / 8, first = kVectors * t % 8;
					if (first == 0) transpose(forward[cur][v]);
#pragma GCC unroll 4
					for (int l = first; l < first + kVectors; ++l) {
						const std::size_t at = (8 * v + l) * stride;
						const Vec f = load(a + at), x = forward[cur][v][l];
						store(a + at, reduce(montgomery(f, odd_lanes(f), x, odd_lanes(x)), kP));
					}
				}
			}
		}
		{
			const std::uint32_t c_chunk = power(c, std::uint32_t(chunk_)), c_step = mont(inverse(c));
			std::array<std::uint32_t, kLanes> c_power;
			for (int s = 0; s < kLanes; ++s) c_power[s] = s ? multiply_mod(c_power[s - 1], c_chunk) : c_chunk;
			Chain kernel([&](int s) { return multiply_mod(c_power[s], inverse_start_[s + 1]); },
						[&](int s) { return multiply_mod(std::uint32_t(start(s + 1)), c_step); }, kP - c_step);
			Block backward[2];
#pragma GCC unroll 8
			for (int t = 0; t < 8; ++t) kernel.advance(backward[0], t);
			for (std::size_t j = 0, cur = 0; j < stride; j += 8, cur ^= 1) {
				std::uint32_t* const e = b_ + length() / 2 - stride + j;
#pragma GCC unroll 8
				for (int t = 0; t < 8; ++t) {
					kernel.advance(backward[cur ^ 1], t);
					const int v = kVectors * t / 8, first = kVectors * t % 8;
					if (first == 0) transpose(backward[cur][v]);
#pragma GCC unroll 4
					for (int l = first; l < first + kVectors; ++l) {
						const std::size_t at = (8 * v + l) * stride;
						store(e - at, reduce(backward[cur][v][l], kP));
					}
				}
			}
		}
	}''' + s[hi:]
 if name=='split_noodd':
  s=s.replace('x[kVectors], base[kVectors], base_odd[kVectors], u, step;', 'x[kVectors], base[kVectors], u, step;')
  s=s.replace('\n\t\tfor (int v = 0; v < kVectors; ++v) base_odd[v] = odd_lanes(base[v]);','')
  s=s.replace('x[v] = montgomery(x[v], odd_lanes(x[v]), add(base[v], u), add(base_odd[v], u));','const Vec factor = add(base[v], u);\n\t\t\tx[v] = montgomery(x[v], odd_lanes(x[v]), factor, odd_lanes(factor));')
 p.write_text(s);print(name)
