from pathlib import Path
import shutil
W=Path('/tmp/fps-borrow-round3');W.mkdir(exist_ok=True)
B=Path('/tmp/fps-evaluation-round3/base')
for name,alignment in [('borrow32',32),('borrow2m',1<<21)]:
 C=W/name;shutil.copytree(B,C,dirs_exist_ok=True);p=C/'components/fps_convolution.hpp';s=p.read_text()
 a=s.index('\tProduct(std::size_t n, std::size_t m) :');b=s.index('\n\tProduct(const Product&)',a)
 s=s[:a]+'''	Product(std::size_t n, std::size_t m) : Product(n, m, nullptr) {}
	Product(std::size_t n, std::size_t m, std::uint32_t* borrowed_a) : lg_(log_length(n, m)) {
		const std::size_t len = length(), tables = ntt::detail::table_words(lg_);
		const std::size_t words = (borrowed_a ? 1 : 2) * (len + kPadding) + 2 * tables;
		storage_ = allocate_fps_words(words);
		a_ = borrowed_a ? borrowed_a : storage_.get();
		b_ = storage_.get() + (borrowed_a ? 0 : len + kPadding);
		roots_ = b_ + len + kPadding;
		inverse_roots_ = roots_ + tables;
		std::fill(roots_, inverse_roots_ + tables, 0);
	}
''' +s[b:]
 a=s.index('template <class Plan>\ninline std::vector<Mint> run(')
 s=s[:a]+'''
inline void copy_input_mint(const std::vector<Mint>& in, Mint* out) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	std::size_t i = 0;
	const auto p = _mm256_set1_epi32(Mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		__m256i x;
		std::memcpy(&x, in.data() + i, sizeof(x));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		std::memcpy(static_cast<void*>(out + i), &x, sizeof(x));
	}
	for (; i < in.size(); ++i) out[i] = Mint::word(std::min(in[i].v, in[i].v - Mint::mod()));
}

inline std::vector<Mint> run_borrowed(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	const std::size_t len = std::size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	constexpr std::size_t alignment = '''+str(alignment)+''';
	const std::size_t words = len + 16 + alignment / sizeof(Mint) - 1, size = a.size() + b.size() - 1;
	std::vector<Mint> result;
	prepare_fps_output(result, words);
	result.resize(words);
	const auto address = reinterpret_cast<std::uintptr_t>(result.data());
	const std::size_t offset = ((alignment - (address & (alignment - 1))) & (alignment - 1)) / sizeof(Mint);
	Mint* const to = result.data() + offset;
	Product plan(a.size(), b.size(), reinterpret_cast<std::uint32_t*>(to));
	copy_input_mint(a, to);
	copy_input(b, plan.b(), len);
	plan.multiply();
	if (offset) std::memmove(static_cast<void*>(result.data()), to, size * sizeof(Mint));
	result.resize(size);
	return result;
}

''' +s[a:]
 s=s.replace('if (Product::fits(a.size(), b.size())) return run<Product>(a, b);','if (Product::fits(a.size(), b.size())) return run_borrowed(a, b);')
 p.write_text(s);print(name)
