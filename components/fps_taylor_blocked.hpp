#ifndef CP_COMPONENTS_FPS_TAYLOR_BLOCKED_HPP
#define CP_COMPONENTS_FPS_TAYLOR_BLOCKED_HPP

#include "fps_convolution.hpp"
#include <array>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
namespace taylor_factorials {
inline constexpr std::uint32_t kStep = 1024;
inline constexpr std::uint32_t kTable[513] = {
	1, 941747501, 776540593, 210672823, 690071003, 46617525, 213768760, 693968959,
	80489582, 706932617, 794151417, 465499254, 696674125, 679081391, 691448896, 713797428,
	275558954, 609927147, 557311239, 831274938, 893291440, 167105676, 415665360, 192812445,
	680052439, 557094195, 688128904, 792752264, 192208002, 847923750, 28263099, 728717984,
	491684840, 336918851, 633366503, 758306641, 296930470, 699020919, 910172402, 905192639,
	792232676, 535935445, 410591335, 810793504, 856039476, 765650325, 702144816, 374640872,
	197994359, 320889851, 883115987, 151590670, 245098264, 286989423, 729221238, 511226407,
	874957813, 979489918, 228248737, 453190470, 223927796, 852989243, 50287712, 509033048,
	243625084, 40159580, 655302994, 829942035, 299277458, 86622874, 109276119, 75894103,
	424113548, 567421291, 676588086, 620367716, 815463188, 44143218, 810930880, 499782,
	416513770, 59750787, 280879191, 113328955, 910641127, 542089438, 873391271, 49111081,
	422655771, 749741439, 838916518, 825928836, 422316600, 445140078, 135792920, 796315921,
	334523653, 505750213, 745454889, 284439669, 264422227, 488144299, 264594887, 332021570,
	256723915, 768304408, 506153085, 533770970, 929732704, 471614028, 243021423, 36822049,
	195608626, 792193516, 215245367, 588445499, 10232161, 974583034, 843169718, 33517339,
	744867256, 422042779, 971678608, 929209828, 740055526, 129980885, 404829672, 225807005,
	528205456, 240143253, 706174821, 263068586, 794757329, 221639463, 240967815, 20047837,
	331134934, 127470054, 793695956, 210565051, 959206504, 138448490, 667121261, 136048183,
	226620404, 692730832, 887464126, 715216840, 786895056, 680299255, 592465564, 104271019,
	241786040, 302567286, 781777622, 695362768, 267443112, 60985765, 733684226, 937002588,
	298149860, 170166008, 805749567, 79282620, 437665466, 386653062, 754588014, 265310082,
	81266867, 109466639, 595236940, 591251736, 755288980, 912652888, 460141120, 759221877,
	825572784, 18194464, 368427783, 571463817, 553552793, 218414240, 8897606, 791219964,
	987232136, 626678298, 363848504, 23298153, 623570515, 506916981, 151639897, 269752482,
	341219264, 108067268, 360379612, 245950594, 64405509, 799733966, 607922664, 255515653,
	169008059, 832297534, 475858276, 990269468, 56687032, 487125465, 715077943, 946099639,
	419973380, 30892688, 823299765, 890460664, 875848715, 155161884, 374508954, 548318802,
	921787093, 698253774, 302647199, 259167661, 143814525, 129489686, 795264610, 975222339,
	968512986, 345514813, 646295751, 970375323, 591627665, 300860854, 909331562, 720624591,
	385294700, 7631741, 443276555, 350397413, 827870055, 225714870, 401347161, 77749271,
	597036959, 606229862, 154326026, 361671183, 413774614, 227075193, 608071418, 914590447,
	407447508, 552058576, 47474609, 386778642, 758202478, 385950691, 703740000, 966684774,
	546362027, 298705630, 676952658, 326449356, 843917452, 242409430, 587833706, 40469219,
	210585234, 354879640, 771230526, 34097422, 102287772, 902362543, 722263693, 814944322,
	451297925, 902863475, 817029670, 905023960, 555424886, 272662632, 394766353, 666370092,
	206219158, 15110235, 976379739, 965535222, 935252937, 650991646, 958967237, 363182237,
	233817196, 344617468, 791123841, 630913138, 726529184, 562573003, 563061146, 792960341,
	994145576, 230805329, 299465420, 259430953, 179251015, 566371622, 567895990, 558709601,
	401533746, 472680587, 807075382, 388625548, 342362854, 714080723, 354518461, 501335155,
	921205714, 245572383, 995121871, 597206529, 697042204, 260992104, 462829184, 62381294,
	98950871, 913320293, 752117967, 982560320, 257419215, 463752698, 697205827, 656377881,
	419290152, 856531770, 447197724, 657965581, 404661208, 722130014, 899687713, 718707067,
	430330393, 639106400, 834964009, 902887941, 810884366, 650679669, 328559029, 108952421,
	175250094, 141508696, 394269300, 94451378, 262192347, 556407145, 661623397, 656765174,
	893733886, 359124883, 407147338, 808451311, 157717476, 949408897, 371377907, 798014486,
	870707644, 11124205, 741182897, 157137689, 593507555, 598520569, 306478687, 42984407,
	411434960, 116335798, 367793469, 345378248, 332981554, 863453067, 588666836, 607696540,
	175011207, 886178571, 387142464, 568865780, 148884679, 477839356, 859137664, 898399404,
	46230887, 716245342, 919506383, 429269215, 489098767, 920899704, 397809292, 323931347,
	417053198, 292836984, 916151861, 903880874, 530159990, 969165939, 930377972, 143032010,
	308745688, 229058228, 904049457, 60317698, 729051789, 192106544, 936997143, 184865547,
	169635803, 457399664, 408032575, 961424900, 378907963, 965079378, 946148009, 745479287,
	341457669, 573316525, 140488077, 559501368, 217284569, 576419480, 775027508, 235369686,
	852776706, 238333284, 188364228, 154707197, 555869026, 259713631, 976997327, 135089920,
	637108313, 605899300, 425299135, 630257864, 414103956, 979668540, 465614154, 736566528,
	578651185, 500126788, 80473976, 482476368, 671936429, 35129612, 389885031, 703915264,
	423047129, 748946352, 989296698, 3993118, 974224703, 841241239, 932795359, 258719575,
	251469715, 16846321, 829202764, 9295132, 213896299, 331463491, 472169457, 817096142,
	274013552, 803636696, 940028364, 742052578, 854664711, 615518321, 593217956, 761584448,
	355182187, 843893299, 269045370, 953802150, 37568955, 542668251, 606642174, 349512365,
	736272199, 770343238, 300023457, 658958330, 122738470, 738075352, 594519714, 497862531,
	938212102, 435561555, 918773761, 884365386, 579355058, 354020897, 592394585, 827412134,
	973741053, 589739172, 312826770, 337102791, 527832013, 824911332, 521867730, 61667222,
	55417785, 731992906, 378154470, 145250909, 177299477, 98768957, 991600964, 978388836,
	495787634};
}
namespace taylor_detail {
namespace ntt = multiply_detail::ntt;
using ntt::detail::add;
using ntt::detail::broadcast;
using ntt::detail::diff;
using ntt::detail::kP;
using ntt::detail::kR;
using ntt::detail::multiply_mod;
using ntt::detail::power;
using ntt::detail::reduce;
using ntt::detail::slot;
using multiply_detail::Subtrees;
using ntt::detail::Vec;
using blocked_poly::detail::load;
using blocked_poly::detail::low;
using blocked_poly::detail::store;
inline std::uint32_t inverse(std::uint32_t x) { return power(x, kP - 2); }
inline std::uint32_t mont(std::uint32_t x) { return multiply_mod(x, kR); }
inline Vec odd_lanes(Vec x) { return _mm256_srli_epi64(x, 32); }
[[gnu::always_inline]] inline Vec montgomery(Vec a, Vec a_odd, Vec b, Vec b_odd) {
	const Vec ni = broadcast(ntt::kernels::kNI), p = broadcast(kP);
	Vec even = _mm256_mul_epu32(a, b), odd = _mm256_mul_epu32(a_odd, b_odd);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}
[[gnu::always_inline]] inline void transpose(Vec (&r)[8]) {
	Vec t[8], u[8];
#pragma GCC unroll 4
	for (int i = 0; i < 8; i += 2) {
		t[i] = _mm256_unpacklo_epi32(r[i], r[i + 1]);
		t[i + 1] = _mm256_unpackhi_epi32(r[i], r[i + 1]);
	}
#pragma GCC unroll 2
	for (int i = 0; i < 8; i += 4) {
		u[i] = _mm256_unpacklo_epi64(t[i], t[i + 2]);
		u[i + 1] = _mm256_unpackhi_epi64(t[i], t[i + 2]);
		u[i + 2] = _mm256_unpacklo_epi64(t[i + 1], t[i + 3]);
		u[i + 3] = _mm256_unpackhi_epi64(t[i + 1], t[i + 3]);
	}
#pragma GCC unroll 4
	for (int i = 0; i < 4; ++i) {
		r[i] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x20);
		r[i + 4] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x31);
	}
}
constexpr int kVectors = 4, kLanes = 8 * kVectors;
template <class F>
void fill(Vec (&x)[kVectors], F f) {
	alignas(32) std::uint32_t values[kLanes];
	for (int s = 0; s < kLanes; ++s) values[s] = f(s);
	for (int v = 0; v < kVectors; ++v) x[v] = load(values + 8 * v);
}
using Block = Vec[kVectors][8];
struct Chain {
	Vec x[kVectors], base[kVectors], base_odd[kVectors], u, step;
	template <class X, class Base>
	Chain(X initial, Base base_of, std::uint32_t step_value) : u(_mm256_setzero_si256()), step(broadcast(step_value)) {
		fill(x, initial);
		fill(base, base_of);
		for (int v = 0; v < kVectors; ++v) base_odd[v] = odd_lanes(base[v]);
	}
	[[gnu::always_inline]] void advance(Block& block, int t) {
#pragma GCC unroll 4
		for (int v = 0; v < kVectors; ++v) {
			block[v][t] = x[v];
			x[v] = montgomery(x[v], odd_lanes(x[v]), add(base[v], u), add(base_odd[v], u));
		}
		u = reduce(add(u, step), kP);
	}
};
class TaylorShift {
public:
	explicit TaylorShift(std::size_t n) : n_(n) {
		lg_ = std::max(10, int(std::bit_width(2 * n - 1)));
		lg_ += lg_ % 2;
		chunk_ = ((n + kLanes - 1) / kLanes + 15) / 16 * 16;
		if (chunk_ / 16 % 2 == 0) chunk_ += 16;
		const std::size_t len = length(), tables = ntt::detail::table_words(lg_);
		const std::size_t words = 2 * len + 3 * kPadding + 2 * tables;
		storage_ = allocate_fps_words(words);
		if (!storage_.get_deleter().region) std::fill_n(storage_.get(), words, 0u);
		b_ = storage_.get() + kPadding;
		a_ = b_ + len + kPadding;
		roots_ = a_ + len + kPadding;
		inverse_roots_ = roots_ + tables;
	}
	std::uint32_t* coefficients() { return a_; }
	const std::uint32_t* shift(std::uint32_t c) {
		using namespace ntt::detail;
		const std::size_t len = length(), q = len / 64;
		build_table(roots_, len / 16, kRoots[0]);
		build_table(inverse_roots_, len / 16, kRoots[1]);
		lane_factorials();
		weights(c);
		auto* av = reinterpret_cast<Vec*>(a_);
		auto* bv = reinterpret_cast<Vec*>(b_);
		multiply_detail::forward_radix8(av, q, roots_);
		multiply_detail::forward_radix8(bv, q, roots_);
		for (std::size_t t = 0; t < 8; ++t) {
			std::uint32_t& x = b_[8 * q * t];
			x = (x % kP + (t < 4 ? 1 : kP - 1)) % kP;
		}
		const Subtrees subtrees(roots_, inverse_roots_);
		for (std::size_t t = 0; t < 4; ++t) subtrees.visit(av + t * q, bv + t * q, q, t);
		ntt::kernels::inverse_identity(av, q, inverse_roots_);
		for (std::size_t t = 4; t < 8; ++t) subtrees.visit(av + t * q, bv + t * q, q, t);
		ntt::kernels::inverse(av + 4 * q, q, inverse_roots_ + slot(1), inverse_roots_ + slot(2));
		output();
		return b_;
	}
private:
	static constexpr std::size_t kPadding = 1040;
	std::size_t length() const { return std::size_t(1) << lg_; }
	std::size_t start(int s) const { return s * chunk_; }
	void lane_factorials() {
		using taylor_factorials::kStep;
		const auto below = [this](int s) { return std::uint32_t(start(s + 1) / kStep * kStep); };
		Vec x[kVectors], m[kVectors], count[kVectors];
		fill(x, [&](int s) { return mont(taylor_factorials::kTable[below(s) / kStep]); });
		fill(m, [&](int s) { return mont(below(s) + 1); });
		fill(count, [&](int s) { return std::uint32_t(start(s + 1) - below(s)); });
		const Vec unit = broadcast(kR);
		for (std::uint32_t j = 0; j < kStep - 1; ++j) {
			const Vec step = broadcast(j);
#pragma GCC unroll 4
			for (int v = 0; v < kVectors; ++v) {
				const Vec factor = _mm256_blendv_epi8(unit, m[v], _mm256_cmpgt_epi32(count[v], step));
				x[v] = montgomery(x[v], odd_lanes(x[v]), factor, odd_lanes(factor));
				m[v] = reduce(add(m[v], unit), kP);
			}
		}
		alignas(32) std::uint32_t lane_x[kLanes];
		for (int v = 0; v < kVectors; ++v) store(lane_x + 8 * v, x[v]);
		const std::uint32_t unit_inverse = inverse(kR);
		factorial_[0] = 1;
		for (int s = 0; s < kLanes; ++s) factorial_[s + 1] = multiply_mod(lane_x[s] % kP, unit_inverse);
		std::uint32_t f = 1;
		for (int s = 0; s <= kLanes; ++s) inverse_start_[s] = f, f = multiply_mod(f, factorial_[s]);
		f = inverse(f);
		for (int s = kLanes; s >= 0; --s)
			inverse_start_[s] = multiply_mod(inverse_start_[s], f), f = multiply_mod(f, factorial_[s]);
	}
	void weights(std::uint32_t c) {
		const std::uint32_t c_chunk = power(c, std::uint32_t(chunk_)), c_step = mont(inverse(c));
		std::array<std::uint32_t, kLanes> c_power;
		for (int s = 0; s < kLanes; ++s) c_power[s] = s ? multiply_mod(c_power[s - 1], c_chunk) : c_chunk;
		Chain input([this](int s) { return mont(factorial_[s]); },
					[this](int s) { return mont(std::uint32_t(start(s) + 1)); }, kR);
		Chain kernel([&](int s) { return multiply_mod(c_power[s], inverse_start_[s + 1]); },
					[&](int s) { return multiply_mod(std::uint32_t(start(s + 1)), c_step); }, kP - c_step);
		const std::size_t stride = chunk_;
		Block forward[2], backward[2];
#pragma GCC unroll 8
		for (int t = 0; t < 8; ++t) input.advance(forward[0], t), kernel.advance(backward[0], t);
		for (std::size_t j = 0, cur = 0; j < stride; j += 8, cur ^= 1) {
			std::uint32_t* const a = a_ + j;
			std::uint32_t* const e = b_ + length() / 2 - stride + j;
#pragma GCC unroll 8
			for (int t = 0; t < 8; ++t) {
				input.advance(forward[cur ^ 1], t);
				kernel.advance(backward[cur ^ 1], t);
				const int v = kVectors * t / 8, first = kVectors * t % 8;
				if (first == 0) transpose(forward[cur][v]), transpose(backward[cur][v]);
#pragma GCC unroll 4
				for (int l = first; l < first + kVectors; ++l) {
					const std::size_t at = (8 * v + l) * stride;
					const Vec f = load(a + at), x = forward[cur][v][l];
					store(a + at, reduce(montgomery(f, odd_lanes(f), x, odd_lanes(x)), kP));
					store(e - at, reduce(backward[cur][v][l], kP));
				}
			}
		}
	}
	void output() {
		const std::uint32_t scale = multiply_mod(mont(inverse(std::uint32_t(length() / 8))), kR);
		Chain chain([&](int s) { return multiply_mod(multiply_mod(inverse_start_[s + 1], std::uint32_t(start(s + 1))), scale); },
					[this](int s) { return mont(std::uint32_t(start(s + 1) - 1)); }, kP - kR);
		const std::size_t stride = chunk_, half = length() / 2;
		Block block[2];
#pragma GCC unroll 8
		for (int t = 0; t < 8; ++t) chain.advance(block[0], 7 - t);
		for (std::size_t j = 0, cur = 0; j < stride; j += 8, cur ^= 1) {
			const std::uint32_t* const u = a_ + stride - 8 - j;
			std::uint32_t* const out = b_ + stride - 8 - j;
#pragma GCC unroll 8
			for (int t = 0; t < 8; ++t) {
				chain.advance(block[cur ^ 1], 7 - t);
				const int v = kVectors * t / 8, first = kVectors * t % 8;
				if (first == 0) transpose(block[cur][v]);
#pragma GCC unroll 4
				for (int l = first; l < first + kVectors; ++l) {
					const std::size_t at = (8 * v + l) * stride;
					const Vec g = low(diff(load(u + at), load(u + at + half))), z = block[cur][v][l];
					store(out + at, reduce(montgomery(g, odd_lanes(g), z, odd_lanes(z)), kP));
				}
			}
		}
	}
	std::size_t n_, chunk_;
	int lg_;
	std::uint32_t *a_, *b_, *roots_, *inverse_roots_;
	FpsWords storage_;
	std::array<std::uint32_t, kLanes + 1> factorial_, inverse_start_;
};
inline void shift(std::vector<Mint> &f, Mint c) {
	TaylorShift t(f.size());
	std::size_t i = 0;
	for (; i + 8 <= f.size(); i += 8) {
		const auto x = blocked_poly::detail::load_unaligned(reinterpret_cast<const unsigned *>(f.data() + i));
		store(t.coefficients() + i, reduce(x, kP));
	}
	for (; i < f.size(); ++i) t.coefficients()[i] = std::min(f[i].v, f[i].v - kP);
	const auto *out = t.shift(c.val());
	i = 0;
	for (; i + 8 <= f.size(); i += 8)
		blocked_poly::detail::store_unaligned(reinterpret_cast<unsigned *>(f.data() + i), load(out + i));
	for (; i < f.size(); ++i) f[i] = Mint::word(out[i]);
}
}
}

#pragma GCC pop_options
#endif
#endif
