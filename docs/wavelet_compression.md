---
title: "Wavelet Matrix の座標圧縮"
documentation_of: //components/wavelet_compression.hpp
library_category: components/util
---

通常の Wavelet Matrix と重み付き Wavelet Matrix が共有する内部の座標圧縮です。Wavelet Matrix 本体やビット列には依存しません。

`wavelet_detail::ValueIndex<T>::build(a)` は元の値を昇順・重複なしにした `values` と、各位置の圧縮値 `original` を作ります。`code(x)` は `values` 内で `x` 未満の値の数を返し、`x` が登録されていなくても使えます。

圧縮関数 `compress` は、通常版が利用する値ごとの出現位置と区切りも生成できます。`compress<T,false>` はそれらを生成しません。出現位置を生成する呼び出しでは、前回の出力を初期化してから処理するため、同じ格納先を再利用できます。`ValueIndex` の `T` はコピーでき、`<` で矛盾なく大小を判定できる型です。互いに小さくない値を同じキーとみなします。`compress` を直接使う場合は `==` も必要で、`<` と等値判定が矛盾しないようにしてください。入力の長さは `INT_MAX` 以下です。

要素数を $$n$$、異なる値の数を $$\sigma$$ とします。64 bit 以下の標準整数型（`bool` を除きます）では、構築は $$O(n)$$ 時間です。その他の型では `compress` は $$O(n\log n)$$、`ValueIndex::build` は $$O(n\log\sigma)$$ 時間です。要素の比較・コピーを定数時間とし、異なる値が 0 個または 1 個の場合は対数の項を定数として扱います。

いずれも空間は $$O(n)$$ です。`code` は $$O(\log\sigma)$$ 時間・$$O(1)$$ 追加空間です。要素自身が持つ文字列などの格納領域は別途必要です。

`wavelet_detail::equivalent(a,b)` は、互いに `<` で小さくないキーなら `bool` の `true` を返します。整数型では `==` を使います。比較を定数時間として、時間・追加空間は $$O(1)$$ です。
