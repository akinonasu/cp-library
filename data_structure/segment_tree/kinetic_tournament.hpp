#ifndef KINETIC_TOURNAMENT_HPP
#define KINETIC_TOURNAMENT_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <class T = std::int64_t, bool maximize = false, class R = std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>>
class KineticTournament {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	static_assert((std::is_integral_v<R> && std::is_signed_v<R>) || std::is_same_v<R, __int128_t>);
	using U = std::uint64_t;
	using W = std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>;

  public:
	struct Line { T a, b; };
	struct Result { int index; R value; };
	using value_type = R;
	using time_type = T;

  private:
	int n_ = 0, base_ = 1, active_ = 0;
	T time_ = 0;
	std::vector<Line> lines_;
	std::vector<int> winner_;
	std::vector<U> event_;
	static U encode(T t) { return U(std::int64_t(t)) ^ (U(1) << 63); }
	static T decode(U t) { return T(std::bit_cast<std::int64_t>(t ^ (U(1) << 63))); }
	static U first_event(U a, U b) { return !a ? b : !b ? a : std::min(a, b); }
	static bool better(R a, int i, R b, int j) {
		if (a == b) return i < j;
		if constexpr (maximize) return a > b;
		else return a < b;
	}
	R evaluate(int i) const { return R(W(lines_[i].a) * W(time_) + W(lines_[i].b)); }
	U crossing(int w, int l) const {
		const auto &a = lines_[w], &b = lines_[l];
		U slope;
		T x, y;
		if constexpr (maximize) {
			if (a.a >= b.a) return 0;
			slope = U(b.a) - U(a.a);
			x = a.b; y = b.b;
		} else {
			if (a.a <= b.a) return 0;
			slope = U(a.a) - U(b.a);
			x = b.b; y = a.b;
		}
		bool negative = x < y;
		U numerator = negative ? U(y) - U(x) : U(x) - U(y);
		U q = numerator / slope, r = numerator % slope;
		W t = negative ? -W(q) : W(q);
		if (negative) t += l > w && !r;
		else t += l > w || r != 0;
		if (t > W(std::numeric_limits<T>::max())) return 0;
		assert(t > W(time_));
		return encode(T(t));
	}
	void pull(int k) {
		int x = winner_[2 * k], y = winner_[2 * k + 1];
		U event = 2 * k < base_ ? first_event(event_[2 * k], event_[2 * k + 1]) : 0;
		if (x == -1) winner_[k] = y;
		else if (y == -1) winner_[k] = x;
		else {
			if (better(evaluate(y), y, evaluate(x), x)) std::swap(x, y);
			winner_[k] = x;
			event = first_event(event, crossing(x, y));
		}
		event_[k] = event;
	}
	void refresh(int k, U t) {
		if (k >= base_ || !event_[k] || event_[k] > t) return;
		refresh(2 * k, t);
		refresh(2 * k + 1, t);
		pull(k);
	}
	void initialize(int n, T t) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 4);
		n_ = n; time_ = t; active_ = 0;
		base_ = int(std::bit_ceil(unsigned(std::max(1, n))));
		winner_.assign(2 * base_, -1);
		event_.assign(base_, 0);
	}
	void check([[maybe_unused]] int i) const { assert(0 <= i && i < n_); }
	std::optional<Result> result(int i) const {
		return i == -1 ? std::nullopt : std::optional<Result>(Result{i, evaluate(i)});
	}

  public:
	explicit KineticTournament(int n = 0, T t = 0) { reset(n, t); }
	explicit KineticTournament(const std::vector<Line> &lines, T t = 0) { build(lines, t); }
	KineticTournament(const KineticTournament &) = default;
	KineticTournament &operator=(const KineticTournament &) = default;
	KineticTournament(KineticTournament &&other) : KineticTournament() { swap(other); }
	KineticTournament &operator=(KineticTournament &&other) {
		if (this != &other) { KineticTournament moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(KineticTournament &other) noexcept {
		std::swap(n_, other.n_); std::swap(base_, other.base_); std::swap(active_, other.active_);
		std::swap(time_, other.time_);
		lines_.swap(other.lines_); winner_.swap(other.winner_); event_.swap(other.event_);
	}
	void reset(int n, T t = 0) {
		initialize(n, t);
		lines_.resize(n);
	}
	void build(const std::vector<Line> &lines, T t = 0) {
		assert(lines.size() <= std::size_t(std::numeric_limits<int>::max() / 4));
		initialize(int(lines.size()), t);
		lines_.assign(lines.begin(), lines.end());
		active_ = n_;
		for (int i = 0; i < n_; ++i) winner_[base_ + i] = i;
		for (int k = base_ - 1; k; --k) pull(k);
	}
	int size() const { return n_; }
	int active_size() const { return active_; }
	bool empty() const { return active_ == 0; }
	T time() const { return time_; }
	void clear() {
		std::fill(winner_.begin(), winner_.end(), -1);
		std::fill(event_.begin(), event_.end(), 0);
		active_ = 0;
	}
	void set(int i, T a, T b) {
		check(i);
		lines_[i] = {a, b};
		int k = base_ + i;
		active_ += winner_[k] == -1;
		winner_[k] = i;
		while (k >>= 1) pull(k);
	}
	bool erase(int i) {
		check(i);
		int k = base_ + i;
		if (winner_[k] == -1) return false;
		winner_[k] = -1;
		--active_;
		while (k >>= 1) pull(k);
		return true;
	}
	std::optional<Line> get(int i) const {
		check(i);
		return winner_[base_ + i] == -1 ? std::nullopt : std::optional<Line>(lines_[i]);
	}
	void advance(T t) {
		assert(time_ <= t);
		time_ = t;
		refresh(1, encode(t));
	}
	std::optional<T> next_event() const {
		return base_ == 1 || !event_[1] ? std::nullopt : std::optional<T>(decode(event_[1]));
	}
	std::optional<Result> all_prod() const { return result(winner_[1]); }
	std::optional<Result> prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n_);
		int index = -1;
		R value = 0;
		auto take = [&](int k) {
			int i = winner_[k];
			if (i == -1) return;
			R candidate = evaluate(i);
			if (index == -1 || better(candidate, i, value, index)) index = i, value = candidate;
		};
		for (l += base_, r += base_; l < r; l >>= 1, r >>= 1) {
			if (l & 1) take(l++);
			if (r & 1) take(--r);
		}
		return index == -1 ? std::nullopt : std::optional<Result>(Result{index, value});
	}
};

#endif
