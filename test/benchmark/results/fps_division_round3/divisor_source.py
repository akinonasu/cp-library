from pathlib import Path
p=Path('/tmp/fps-division-round3/candidate/components/fps_division_blocked.hpp')
s=p.read_text()
for func in ['long_division','remainder_direct','remainder_wrap','remainder_split','divide_blocks','divide']:
 marker='template<class T>\ninline void '+func+'('
 assert s.count(marker)==1,(func,s.count(marker))
 start=s.index(marker);end=s.find('\n}\n',start)+3
 part=s[start:end].replace('template<class T>','template<class T, class G>')
 part=part.replace('std::span<const std::uint32_t> g','std::span<const G> g')
 if func=='long_division':
  part=part.replace('power(g[d],','power(input_word(g[d]),').replace('multiply_mod(c, g[t])','multiply_mod(c, input_word(g[t]))')
 if func=='remainder_direct':part=part.replace('std::uint64_t(q[i]) * g[j - i]','std::uint64_t(q[i]) * input_word(g[j - i])')
 if func=='divide_blocks':
  part=part.replace('std::reverse_copy(g.end() - std::ptrdiff_t(G.size()), g.end(), G.begin());','if constexpr (std::is_same_v<G, std::uint32_t>) std::reverse_copy(g.end() - std::ptrdiff_t(G.size()), g.end(), G.begin());\n\telse reverse_difference(g, std::ptrdiff_t(g.size() - 1), G.data(), G.size());')
  # local variable G shadows type parameter, use U for type
  part=part.replace('class G>','class U>').replace('span<const G> g','span<const U> g').replace('is_same_v<G,','is_same_v<U,')
 if func=='remainder_wrap':
  a=part.index('\tstd::span<const G> gl = g;');b=part.index('\n\tfold(f, a);',a)
  part=part[:a]+'''\tif (g.size() > len) {
		if constexpr (std::is_same_v<G, std::uint32_t>) std::copy_n(g.begin(), len, b.begin());
		else for (std::size_t i = 0; i < len; ++i) b[i] = input_word(g[i]);
		b[0] = (b[0] + input_word(g[len])) % kP;
		t.cyclic_product(b, a);
	} else {
		cyclic_product(t, g, b, a);
	}'''+part[b:]
 if func=='remainder_split':part=part.replace('t.cyclic_product(g.subspan(len + 1 - k, k + e - 1), 0, c, a);','cyclic_product(t, g.subspan(len + 1 - k, k + e - 1), c, a);')
 s=s[:start]+part+s[end:]
# Add typed fused cyclic transform, preserving raw algorithm.
pos=s.index('template<class T, class G>\ninline void remainder_wrap')
helper='''template<class G>
inline void cyclic_product(const Transform& t, std::span<const G> in, std::span<std::uint32_t> out,
						   std::span<const std::uint32_t> b) {
	if constexpr (std::is_same_v<G, std::uint32_t>) return t.cyclic_product(in, 0, out, b);
	else {
		const std::size_t n = out.size(), nv = n / 8;
		const int lg = std::countr_zero(n);
		if (!std::has_single_bit(n) || lg < Transform::kMinLog || lg > t.lg_max() || in.size() > n) std::abort();
		const MintSource source(in);
		const ProductBottom bottom{t.roots(), t.inverse_roots(), b.data()};
		const Recursion recursion(t.roots(), t.inverse_roots(), bottom);
		const std::uint32_t scale = kInverseScales[true][lg];
		auto* v = reinterpret_cast<Vec*>(out.data());
		if (std::countr_zero(nv) % 2 == 0) {
			const std::size_t h = nv / 4;
			mint_forward_top4(source, v, h, t.roots());
			for (std::size_t j = 0; j < 4; ++j) recursion.visit(out.data() + 8 * j * h, h, j);
			inverse_top4(v, h, Half::kBoth, t.inverse_roots(), Factor(scale));
		} else {
			const std::size_t h = nv / 2;
			mint_forward_top2(source, v, h);
			recursion.visit(out.data(), h, 0);
			recursion.visit(out.data() + 8 * h, h, 1);
			inverse_top2(v, h, Half::kBoth, scale);
		}
	}
}

'''
s=s[:pos]+helper+s[pos:]
s=s.replace('Arena arena(plan.words() + Arena::footprint(m) + Arena::footprint(k) + Arena::footprint(d));\n\tconst std::span<const Mint> f(a, n);\n\tauto g = arena.take(m), q = arena.take(k), r = arena.take(d);\n\tfor (std::size_t i = 0; i < m; ++i) g[i] = std::min(b[i].v, b[i].v - Mint::mod());','Arena arena(plan.words() + Arena::footprint(k) + Arena::footprint(d));\n\tconst std::span<const Mint> f(a, n), g(b, m);\n\tauto q = arena.take(k), r = arena.take(d);')
p.write_text(s)
