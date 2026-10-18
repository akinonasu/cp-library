from pathlib import Path
R=Path('/tmp/fps-native-source/after');p=R/'components/fps_blocked.hpp';s=p.read_text()
a=s.index('[[gnu::noinline]] inline void forward_top4(Source');b=s.index('\ninline bool forward_top8(',a)
top=s[a:b].replace('Source in','MintSource in').replace('forward_top4','mint_forward_top4').replace('forward_top2','mint_forward_top2')
a=s.index('inline void derivative(std::span<const std::uint32_t>');b=s.index('\ninline void divide_by_index(',a)
derivative=s[a:b].replace('derivative(std::span<const std::uint32_t> f','mint_derivative(std::span<const Mint> f').replace('load_unaligned(f.data() + i + 1)','mint_load(f, i + 1)').replace('f[i + 1])','mint_scalar(f[i + 1]))')
a=s.index('inline void subtract_from_derivative(');b=s.index('\nstruct ToeplitzBottom',a)
subtract=s[a:b].replace('subtract_from_derivative(std::span<const std::uint32_t> f','mint_subtract_from_derivative(std::span<const Mint> f').replace('load_unaligned(f.data() + first + 1 + c)','mint_load(f, first + 1 + c)').replace('f[first + 1 + c])','mint_scalar(f[first + 1 + c]))')
a=s.index('template <class Sink, class Transformed = NoLogTransform>');b=s.index('\ninline int log_derivative_log(',a)
ld=s[a:b].replace('void log_derivative(const Transform& t, std::span<const std::uint32_t> f','void mint_log_derivative(const Transform& t, std::span<const Mint> f').replace('std::span<std::uint32_t> scratch, const Sink& sink','std::span<std::uint32_t> scratch, std::span<const std::uint32_t> prefix, const Sink& sink').replace('inverse(t, f, ht.first(k)','inverse(t, prefix, ht.first(k)').replace('t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));','mint_forward(t, f.subspan(from, std::min(f.size() - from, len)), window(s));').replace('derivative(f.first(', 'mint_derivative(f.first(').replace('subtract_from_derivative(f,', 'mint_subtract_from_derivative(f,')
helper='''namespace blocked_poly::detail {
inline unsigned mint_scalar(Mint x) { return std::min(x.v, x.v - Mint::mod()); }
inline Vec mint_load(std::span<const Mint> f, std::size_t at) {
	if (at + 8 <= f.size()) {
		Vec value;
		std::memcpy(&value, f.data() + at, sizeof(value));
		return reduce(value, kP);
	}
	alignas(32) unsigned values[8] = {};
	for (std::size_t i = 0; i < 8 && at + i < f.size(); ++i) values[i] = mint_scalar(f[at + i]);
	return load(values);
}
class MintSource {
public:
	explicit MintSource(std::span<const Mint> f) : f_(f), full_(f.size() / 8), touched_((f.size() + 7) / 8) {}
	Vec operator()(std::size_t v) const {
		if (v < full_) {
			Vec value;
			std::memcpy(&value, f_.data() + 8 * v, sizeof(value));
			return reduce(value, kP);
		}
		if (v >= touched_) return _mm256_setzero_si256();
		return edge(f_, 8 * v);
	}
	bool within(std::size_t lo, std::size_t hi) const { return touched_ == 0 || (lo == 0 && touched_ <= hi); }
private:
	[[gnu::noinline]] static Vec edge(std::span<const Mint> f, std::size_t at) { return mint_load(f, at); }
	std::span<const Mint> f_;
	std::size_t full_, touched_;
};
'''+top+'''
inline void mint_forward(const Transform& t, std::span<const Mint> f, std::span<std::uint32_t> out) {
	const MintSource in(f);
	const detail::Recursion recursion(t.roots(), t.inverse_roots(), ForwardBottom{t.roots()});
	const std::size_t nv = out.size() / 8;
	auto* v = reinterpret_cast<Vec*>(out.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		mint_forward_top4(in, v, h, t.roots());
		for (std::size_t q = 0; q < 4; ++q) recursion.visit(out.data() + 8 * q * h, h, q);
	} else {
		const std::size_t h = nv / 2;
		mint_forward_top2(in, v, h);
		recursion.visit(out.data(), h, 0);
		recursion.visit(out.data() + 8 * h, h, 1);
	}
}
'''+derivative+'\n'+subtract+'\n'+ld+'\n}\n'
marker='namespace blocked_poly {\ninline void native_log('
a=s.index(marker);s=s[:a]+helper+s[a:]
a=s.index(marker);head=s[:a];body=s[a:];body=body.replace('Arena arena(Transform::words(lg) + Arena::footprint(c) + Arena::footprint(log_scratch(n)));','const auto prefix_size = std::min(c, detail::log_block(n));\n\tArena arena(Transform::words(lg) + Arena::footprint(prefix_size) + Arena::footprint(log_scratch(n)));').replace('auto f = arena.take(c), scratch', 'auto f = arena.take(prefix_size), scratch').replace('i < c; ++i','i < prefix_size; ++i').replace('detail::log_derivative(t, f, n, scratch,','detail::mint_log_derivative(t, {in, c}, n, scratch, f,');s=head+body;p.write_text(s)
print('extra lines',helper.count('\n'),'modified',p)
