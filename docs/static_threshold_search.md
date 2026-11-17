---
title: Static Threshold Search
documentation_of: //data_structure/static_threshold_search.hpp
library_category: data_structure/range_query
---

固定した列で、しきい値を満たす最初・最後の添字を検索します。標準の `next_ge(l,x)` は `l <= i` かつ `A[i] >= x` を満たす最小の `i` を返します。比較条件・検索方向を型で選びます。`strict` の付いた操作は等号を含めずに比較します。

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

型は `StaticThresholdSearch<T = std::int64_t, bool Smaller = false, bool Reverse = false, Compare = std::less<T>>` です。`T` は `bool` を除くコピー・代入できる型とし、列長は `INT_MAX` 以下にしてください。整数・実数・文字列・構造体などを使えます。しきい値は同じ `T` を渡し、厳密不等号を実現するために `x+1`・`x-1` は計算しません。整数型の最小値・最大値も使えます。

`compare(a,b)` は、`a` が `b` より前に来るとき `true` を返す関数です。既定の `std::less<T>` は通常の `<` に対応します。比較では、自分自身には `false` を返し、`a` が `b` より前、`b` が `c` より前なら `a` が `c` より前になるようにします。どちら向きの比較も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。構築後、再構築するまでは同じ 2 値に対する比較結果が変わらないようにしてください。`Compare` はコピー・交換ができ、`const` オブジェクトから呼べる型を使います。既定のコンストラクタがない `Compare` では、構築時に比較関数を明示してください。`T` に既定のコンストラクタや数値の最大値・最小値がある必要はありません。浮動小数点型を既定の比較で使う場合は NaN を含めないでください。正負の無限大は使えます。

```cpp
#include "lib/data_structure/static_threshold_search.hpp"
#include <cassert>
#include <string>
#include <vector>

struct Item { int priority; std::string name; };
struct ByPriority {
	bool descending;
	bool operator()(const Item &a, const Item &b) const {
		return descending ? a.priority > b.priority : a.priority < b.priority;
	}
};

int main() {
	StaticThresholdSearch<std::string> words(std::vector<std::string>{"aa", "dd", "cc"});
	assert(words.next_ge(0, "bb") == 1);

	std::vector<Item> items{ {2, "a"}, {7, "b"}, {4, "c"} };
	StaticThresholdSearch<Item, false, false, ByPriority> index(items, ByPriority{false});
	assert(index.next_ge(0, Item{5, ""}) == 1);
	index.build(items, ByPriority{true});
	assert(index.next_ge(0, Item{5, ""}) == 0); // 降順では priority <= 5
}
```

独自の比較を指定した場合は、その比較順を基準に大小を読みます。例えば `std::greater<T>` での `next_ge(l,x)` は通常の数値では `A[i] <= x` を検索します。

| 型引数 | 条件・方向 |
| --- | --- |
| `Smaller = false` | `!compare(A[i],x)`、strict 版は `compare(x,A[i])` |
| `Smaller = true` | `!compare(x,A[i])`、strict 版は `compare(A[i],x)` |
| `Reverse = false` | 指定位置以降の最小の添字 |
| `Reverse = true` | 指定端点より前の最大の添字 |

| API | 引数と返り値 |
| --- | --- |
| `StaticThresholdSearch()` / `StaticThresholdSearch(compare)` | 既定／指定した比較で空のデータ構造を作る |
| `StaticThresholdSearch(std::vector<T> values, Compare compare = Compare{})` | 列と比較関数を受け取り構築する。列をそのまま渡すとコピーし、`std::move` で渡すと領域を移す |
| `build(std::vector<T> values)` | 比較関数を維持し、列を置き換えて再構築。`void` |
| `build(std::vector<T> values, Compare compare)` | 列と比較関数を置き換えて再構築。`void` |
| `size()` / `empty()` | 列長を `int` / 空かを `bool` で返す |
| `values()` / `operator[](int i)` | 元の順序の列を `std::span<const T>` / 要素を `const T&` で返す |
| `clear()` | 比較関数を維持して列を空にする。`void` |
| `swap(other)` / `swap(a,b)` | 列と比較関数を交換する。`void` |
| `next(l,x)` / `next_strict(l,x)` | `Reverse=false` で使用。`[l,size())` 内の条件を満たす最小の添字を `int` で返す |
| `first(l,r,x)` / `first_strict(l,r,x)` | `Reverse=false` で使用。`[l,r)` 内の最小の添字を `int` で返す |
| `prev(r,x)` / `prev_strict(r,x)` | `Reverse=true` で使用。`[0,r)` 内の条件を満たす最大の添字を `int` で返す |
| `last(l,r,x)` / `last_strict(l,r,x)` | `Reverse=true` で使用。`[l,r)` 内の最大の添字を `int` で返す |
| `next_ge(l,x)` / `next_gt(l,x)` | `Smaller=false, Reverse=false` の非 strict / strict 検索の別名 |
| `next_le(l,x)` / `next_lt(l,x)` | `Smaller=true, Reverse=false` の非 strict / strict 検索の別名 |
| `prev_ge(r,x)` / `prev_gt(r,x)` | `Smaller=false, Reverse=true` の非 strict / strict 検索の別名 |
| `prev_le(r,x)` / `prev_lt(r,x)` | `Smaller=true, Reverse=true` の非 strict / strict 検索の別名 |
| `next_record(int i)` | `i` より検索方向へ進んだ位置で、`A[i]` より厳密に良い値を持つ最も近い添字を `int` で返す。`Smaller=false` なら比較順で大きい値、`Smaller=true` なら小さい値 |

すべての検索で、不在時は **`size()`** を返します。空のデータ構造では結果は 0 です。重複値を含んでいても添字の最小・最大で決まります。`prev` の `r` は含まない端点なので、`i <= p` から探すには `prev_ge(p+1,x)` などを使います。比較関数の既定値は `Compare{}` です。検索の引数には既定値はありません。

要素取得と `next_record` には `0 <= i < size()`、1端点の検索には `0 <= l,r <= size()`、区間検索には `0 <= l <= r <= size()` を渡します。空区間の検索結果も `size()` です。型引数と異なる方向・条件のメソッドは使用できず、コンパイル時に検査します。

構築後は読み取り専用です。値の変更には `build` を使います。元の列を変更・破棄しても影響しません。`build`・`clear`・代入・破棄後は以前の参照や `span` を使わないでください。列のコピーは独立し、比較関数もコピーされます。ムーブ元は同じ比較関数を持つ空の状態で、再構築して使えます。ムーブ構築は比較関数のコピー、`swap` はその交換、ムーブ代入はその両方が例外を投げない場合に `noexcept` です。構築用配列の領域確保が失敗した場合は、元の列と検索用データを維持します。

列を 64 要素ずつのブロックに分け、ブロックごとの最小値・最大値を持つ木を構築します。検索開始位置を含むブロックは直接調べ、見つからなければ木をたどって条件を満たす最初・最後のブロックを探します。最後にそのブロック内を調べます。ハッシュや乱数は使いません。

列長を $$n$$ とすると、構築は $$O(n)$$ 時間、保持空間・構築時の空間は $$O(n)$$ です。閾値を指定する検索は最悪 $$O(\log n)$$ 時間、追加空間 $$O(1)$$ です。`next_record` は構築時に求めた添字を返すため、最悪 $$O(1)$$ 時間・追加空間です。ブロック内の走査は固定長なので定数時間です。

サイズ・要素・列の取得・`swap` は $$O(1)$$、コピーと旧領域の解放は $$O(n)$$、ムーブ構築は $$O(1)$$ です。`clear` は $$O(n)$$、再構築は旧列の解放を含み、旧列長と新列長の合計に比例する時間・空間です。代入は旧列の解放時間も加わります。これらは要素と比較関数のコピー・比較・交換・破棄を定数時間とした計算量です。文字列などで各操作に長さに応じた時間がかかる場合は、その費用もかかります。空間の式は要素と比較関数 1 個の保持量を定数としたもので、文字列の文字データなどを型の内部で保持する領域も別途必要です。空区間・長さ 0・1 の処理は定数時間です。
