from pathlib import Path
import shutil
B=Path('/tmp/cp-fps-round2-base');R=Path('/tmp/fps-eval-round2/eval_candidate');
if R.exists():shutil.rmtree(R)
shutil.copytree(B,R)
p=R/'components/fps_evaluation.hpp';s=p.read_text();s=s.replace('PointTree(Arena& arena, std::span<const std::uint32_t> points)','template <class Point>\n\tPointTree(Arena& arena, std::span<const Point> points)',1)
s=s.replace('''		std::copy(points.begin(), points.end(), padded);''','''		if constexpr (std::is_same_v<Point, Mint>) {
			std::size_t i = 0;
			for (; i + 8 <= points.size(); i += 8) {
				Vec x;
				std::memcpy(&x, points.data() + i, sizeof(x));
				store(padded + i, canonical(montgomery(x, broadcast(1))));
			}
			for (; i < points.size(); ++i) padded[i] = points[i].val();
		} else std::copy(points.begin(), points.end(), padded);''',1)
s=s.replace('inline void divide(const Transform& t, const Division& s, std::span<const std::uint32_t> f,','template <class Coefficient>\ninline void divide(const Transform& t, const Division& s, std::span<Coefficient> f,',1)
s=s.replace('''	std::reverse_copy(f.begin(), f.end(), g.begin() + std::ptrdiff_t(big - f.size()));''','''	if constexpr (std::is_same_v<std::remove_const_t<Coefficient>, Mint>) {
		for (std::size_t i = 0; i < f.size(); ++i) {
			const unsigned x = f[f.size() - 1 - i].v;
			g[big - f.size() + i] = std::min(x, x - Mint::mod());
		}
	} else std::reverse_copy(f.begin(), f.end(), g.begin() + std::ptrdiff_t(big - f.size()));''',1)
s=s.replace('InterpolationTree(Arena& arena, std::span<const std::uint32_t> points)','template <class Point>\n\tInterpolationTree(Arena& arena, std::span<const Point> points)',1)
a=s.index('inline std::vector<Mint> native_evaluate(');b=s.index('\ninline std::vector<Mint> native_interpolate(',a)
old=s[a:b]
new='''inline std::vector<Mint> native_evaluate(std::span<const Mint> f, std::span<const Mint> points) {
	if (f.size() * points.size() <= detail::kDirectEvaluation) {
		Arena arena(Arena::footprint(f.size()) + 2 * Arena::footprint(points.size()));
		auto a = arena.take(f.size()), x = arena.take(points.size()), y = arena.take(points.size());
		import_montgomery(f, a);
		import_words(points, x);
		evaluate(arena, a, x, y);
		return export_words(y);
	}
	Arena arena(detail::evaluate_tree_words(f.size(), points.size()));
	PointTree tree(arena, points);
	const std::size_t m8 = tree.size();
	const detail::Division sizes(f.size(), m8);
	const Transform t(arena, sizes.log());
	auto d = arena.take(sizes.big);
	auto scratch = arena.take(std::max(sizes.scratch_words(), PointTree::descend_words(points.size())));
	detail::divide(t, sizes, f, tree.product(), tree.product_transform(), d, scratch);
	auto all = arena.take(m8);
	tree.descend(d.subspan(sizes.big - m8, m8), all, detail::kR, scratch);
	return export_words(all.first(points.size()));
}'''
s=s[:a]+new+s[b:]
a=s.index('inline std::vector<Mint> native_interpolate(');b=s.index('\n}\n}',a)
s=s[:a]+'''inline std::vector<Mint> native_interpolate(std::span<const Mint> points, std::span<const Mint> values) {
	const std::size_t n = points.size();
	Arena arena(interpolate_words(n));
	InterpolationTree tree(arena, points);
	const std::size_t m8 = tree.size(), length = StandardLayout::length(std::uint32_t(m8));
	auto y = arena.take(std::max(n + 1, m8));
	std::reverse_copy(tree.product().begin(), tree.product().begin() + std::ptrdiff_t(n + 1), y.begin());
	derivative(y.first(n + 1), y.first(n));
	const detail::Division sizes(n, m8);
	const Transform t(arena, sizes.log());
	auto window = arena.take(length);
	auto scratch = arena.take(std::max(sizes.scratch_words(), InterpolationTree::scratch_words(n)));
	detail::divide(t, sizes, y.first(n), tree.product(), tree.product_transform(), window.first(m8), scratch);
	import_montgomery(values, y.first(n));
	y[n] = 0;
	tree.interpolate(window.first(m8), y.first(m8), window, scratch);
	std::reverse(window.begin(), window.begin() + std::ptrdiff_t(n));
	return export_words(window.first(n));
}'''+s[b+2:]
p.write_text(s)
