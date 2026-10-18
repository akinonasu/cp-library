from pathlib import Path
p=Path('/tmp/fps-native-source/after_shared/components/fps_blocked.hpp');s=p.read_text()
# Keep the non-template normal entry points; inline a shared body for each input view.
a=s.index('[[gnu::noinline]] inline void forward_top4(Source');b=s.index('\ninline bool forward_top8(',a)
original=s[a:b];body=original.replace('[[gnu::noinline]] inline void forward_top4(Source','template <class Input>\n[[gnu::always_inline]] inline void forward_top4_body(Input').replace('[[gnu::noinline]] inline void forward_top2(Source','template <class Input>\n[[gnu::always_inline]] inline void forward_top2_body(Input')
body+='''
[[gnu::noinline]] inline void forward_top4(Source in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void forward_top2(Source in, Vec* out, std::size_t h) {
	forward_top2_body(in, out, h);
}
'''
s=s[:a]+body+s[b:]
# Extract the specialized view/load/derivative operations and put them before the common loop.
a=s.index('namespace blocked_poly::detail {\ninline unsigned mint_scalar');b=s.index('template <class Sink, class Transformed = NoLogTransform>',a);c=s.index('namespace blocked_poly {\ninline void native_log(',b)
helpers=s[a:b].removeprefix('namespace blocked_poly::detail {\n')
x=helpers.index('[[gnu::noinline]] inline void mint_forward_top4(');y=helpers.index('inline void mint_forward(',x)
helpers=helpers[:x]+'''[[gnu::noinline]] inline void mint_forward_top4(MintSource in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void mint_forward_top2(MintSource in, Vec* out, std::size_t h) {
	forward_top2_body(in, out, h);
}

'''+helpers[y:]
s=s[:a]+s[c:]
a=s.index('struct NoLogTransform {');s=s[:a]+helpers+s[a:]
a=s.index('template <class Sink, class Transformed = NoLogTransform>');b=s.index('\ninline int log_derivative_log(',a)
original=s[a:b]
impl=original.replace('template <class Sink, class Transformed = NoLogTransform>', 'template <class Input, class Sink, class Transformed>')
impl=impl.replace('void log_derivative(const Transform& t, std::span<const std::uint32_t> f','void log_derivative_impl(const Transform& t, Input f').replace('std::span<std::uint32_t> scratch, const Sink& sink','std::span<std::uint32_t> scratch, std::span<const std::uint32_t> prefix, const Sink& sink').replace('inverse(t, f, ht.first(k)','inverse(t, prefix, ht.first(k)')
impl=impl.replace('''		t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));''','''		if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_forward(t, f.subspan(from, std::min(f.size() - from, len)), window(s));
		else t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));''')
impl=impl.replace('''			derivative(f.first(std::min(f.size(), count + 1)), work.first(count));''','''			if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_derivative(f.first(std::min(f.size(), count + 1)), work.first(count));
			else derivative(f.first(std::min(f.size(), count + 1)), work.first(count));''')
impl=impl.replace('''			subtract_from_derivative(f, first, count, work.data(), k);''','''			if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_subtract_from_derivative(f, first, count, work.data(), k);
			else subtract_from_derivative(f, first, count, work.data(), k);''')
wrappers='''
template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, std::span<const std::uint32_t> f, std::size_t n,
		std::span<std::uint32_t> scratch, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative_impl(t, f, n, scratch, f, sink, transformed);
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void mint_log_derivative(const Transform& t, std::span<const Mint> f, std::size_t n,
		std::span<std::uint32_t> scratch, std::span<const std::uint32_t> prefix, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative_impl(t, f, n, scratch, prefix, sink, transformed);
}
'''
s=s[:a]+impl+wrappers+s[b:]
# Keep the old path below 2^18 to avoid switching away from huge-page allocation at 131073.
a=s.index('namespace blocked_poly {\ninline void native_log(');before=s[:a];native=s[a:]
native=native.replace('const auto prefix_size = std::min(c, detail::log_block(n));','const bool direct = n >= 262144;\n\tconst auto prefix_size = direct ? std::min(c, detail::log_block(n)) : c;')
native=native.replace('detail::mint_log_derivative(t, {in, c}, n, scratch, f, [&out, integrate, n]', 'const auto sink = [&out, integrate, n]')
native=native.replace('''	});
}
}''','''	};
	if (direct) detail::mint_log_derivative(t, {in, c}, n, scratch, f, sink);
	else detail::log_derivative(t, f, n, scratch, sink);
}
}''')
s=before+native;s=s.replace('#include <cstdlib>','#include <cstdlib>\n#include <type_traits>');p.write_text(s)
base=Path('/tmp/fps-native-source/before/components/fps_blocked.hpp').read_text();print('net extra lines',s.count('\n')-base.count('\n'))
