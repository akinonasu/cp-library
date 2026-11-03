---
title: Static Threshold Search
documentation_of: //data_structure/static_threshold_search.hpp
---

固定した整数列で、閾値を満たす最初・最後の添字を検索します。標準の `next_ge(l,x)` は `l <= i` かつ `A[i] >= x` を満たす最小の `i` を返します。比較方向・検索方向を型で選び、必要な方向の索引だけを構築します。

```cpp
#include "lib/data_structure/static_threshold_search.hpp"
#include <cassert>
#include <vector>

int main() {
	std::vector<int> a{2, 7, 1, 7, 4};
	StaticThresholdSearch<int> above(a);
	assert(above.next_ge(2, 6) == 3);
	assert(above.next_gt(0, 7) == above.size());
	assert(above.first(0, 3, 6) == 1);
	assert(above.next_record(0) == 1);

	StaticThresholdSearch<int, true> below(a);
	assert(below.next_le(1, 2) == 2);
	assert(below.next_lt(0, 2) == 2);

	StaticThresholdSearch<int, false, true> previous(a);
	assert(previous.prev_ge(4, 7) == 3);
	assert(previous.last(0, 3, 7) == 1);
}
```

型は `StaticThresholdSearch<T = std::int64_t, bool Smaller = false, bool Reverse = false>` です。`T` は `bool` を除く 64 bit 以下の標準整数型で、長さは `INT_MAX` 以下とします。符号付き・符号なしの最小値・最大値も使えます。閾値は同じ `T` を渡し、厳密不等号を実現するために `x+1`・`x-1` は計算しません。

| 型引数 | 条件・方向 |
| --- | --- |
| `Smaller = false` | `A[i] >= x`、strict 版は `A[i] > x` |
| `Smaller = true` | `A[i] <= x`、strict 版は `A[i] < x` |
| `Reverse = false` | 指定位置以降の最小の添字 |
| `Reverse = true` | 指定端点より前の最大の添字 |

| API | 引数と返り値 |
| --- | --- |
| `StaticThresholdSearch()` | 空の索引 |
| `StaticThresholdSearch(std::vector<T> values)` | 列を値で受け取り保持する。lvalue はコピー、rvalue は領域を移す |
| `build(std::vector<T> values)` | 列を置き換えて再構築。`void` |
| `size()` / `empty()` | 列長を `int` / 空かを `bool` で返す |
| `values()` / `operator[](int i)` | 元の順序の列を `std::span<const T>` / 要素を `const T&` で返す |
| `clear()` / `swap(other)` | 空にする / 内容を交換する。`void` |
| `next(l,x)` / `next_strict(l,x)` | `Reverse=false` で使用。`[l,size())` 内の条件を満たす最小の添字を `int` で返す |
| `first(l,r,x)` / `first_strict(l,r,x)` | `Reverse=false` で使用。`[l,r)` 内の最小の添字を `int` で返す |
| `prev(r,x)` / `prev_strict(r,x)` | `Reverse=true` で使用。`[0,r)` 内の条件を満たす最大の添字を `int` で返す |
| `last(l,r,x)` / `last_strict(l,r,x)` | `Reverse=true` で使用。`[l,r)` 内の最大の添字を `int` で返す |
| `next_ge(l,x)` / `next_gt(l,x)` | `Smaller=false, Reverse=false` の非 strict / strict 検索の別名 |
| `next_le(l,x)` / `next_lt(l,x)` | `Smaller=true, Reverse=false` の非 strict / strict 検索の別名 |
| `prev_ge(r,x)` / `prev_gt(r,x)` | `Smaller=false, Reverse=true` の非 strict / strict 検索の別名 |
| `prev_le(r,x)` / `prev_lt(r,x)` | `Smaller=true, Reverse=true` の非 strict / strict 検索の別名 |
| `next_record(int i)` | `i` より検索方向へ進んだ位置で、`A[i]` より厳密に良い値を持つ最も近い添字を `int` で返す。greater なら大きい値、smaller なら小さい値 |

すべての検索で、不在時は **`size()`** を返します。空の索引では結果は 0 です。重複値を含んでいても添字の最小・最大で決まります。`prev` の `r` は含まない端点なので、`i <= p` から探すには `prev_ge(p+1,x)` などを使います。引数の既定値は型引数だけです。

要素取得と `next_record` には `0 <= i < size()`、1端点の検索には `0 <= l,r <= size()`、区間検索には `0 <= l <= r <= size()` を渡します。空区間の検索結果も `size()` です。型引数と異なる方向・条件のメソッドは使用できず、コンパイル時に検査します。

構築後は読み取り専用です。値の変更には `build` を使います。元の列を変更・破棄しても影響しません。`build`・`clear`・代入・破棄後は以前の参照や `span` を使わないでください。コピーは独立し、ムーブ元は空です。ムーブ・`swap` は `noexcept`。`build` の領域確保失敗時は元の索引を維持します。祖先表の添字が `int` に収まらない大きさでは `std::length_error` を投げます。

指定方向に厳密に良い値へ進むリンクを作り、その森を重軽分解します。軽い辺を通る回数は $$O(\log n)$$ なので、重いパスの先頭の祖先列を二分探索して対象のパスを $$O(\log\log n)$$ 時間で特定できます。パス内の単調な整数列には、ワード幅ごとの標本と prefix hash による静的 successor 検索を使います。近距離の検索は固定個数のブロックを先に調べます。

列長を $$n$$、整数型のビット幅を $$w$$ とすると、通常のハッシュ参照を期待定数時間とするモデルで、検索は **期待 $$O(\log\log n+\log w)$$** 時間です。64 bit を固定した場合や、$$w=O(\log n)$$ の word RAM モデルでは期待 $$O(\log\log n)$$ になります。**最悪時間の $$O(\log\log n)$$ 保証ではありません**。ハッシュの衝突は結果の正しさに影響しませんが、時間に影響します。

構築は期待 $$O(n\log n)$$ 時間、保持空間・構築時の空間は $$O(n\log n)$$ です。通常の SegTree より前計算とメモリを多く使い、検索を重視した構造です。構築を含む総時間が常に短くなるわけではありません。`next_record`・サイズ・要素・列の取得・`swap` は $$O(1)$$、コピーと旧領域の解放は保持量に比例し、ムーブ構築は $$O(1)$$ です。空区間・長さ 0・1 の処理は定数時間です。
