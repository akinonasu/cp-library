---
title: FPS と多項式除算の内部変換
documentation_of: //components/fps_division_blocked.hpp
---

`fps_detail::blocked_poly` の除算用内部部品です。通常は `fps::div_series()`、`fps::divmod()`、`polynomial_inv_mod()` から使います。Linux x86_64 / GCC でのみ定義され、呼び出す前に `fps_detail::NTT::vectorized()` が真であることを確認します。他の環境では公開 FPS 関数が既存の実装を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_div_series(a,n,b,m,out,degree)` | 級数除算の最初の degree 項を追加 | $$O(N\log N)$$ |
| `native_divide(a,n,b,m,q,r)` | 多項式の商と剰余を追加 | $$O(N\log N)$$ |
| `divide_words(n,m)` | 除算用の作業領域のワード数 | $$O(\log N)$$ |
| `quotient_size(n,m)` | 商に必要な係数数 | $$O(1)$$ |
| `remainder_size(n,m)` | 剰余に必要な係数数 | $$O(1)$$ |
| `divide(arena,f,g,q,r)` | 通常表現での多項式除算 | $$O(N\log N)$$ |
| `division_detail::append_words(out,values,n,factor)` | factor 倍した n 係数を追加 | 予約済み: $$O(n)$$、再確保時: 既存要素数＋n に比例 |

表では $$N=\max(n,m,\mathrm{degree})$$ とします。関数は `fps_detail::blocked_poly` 内にあります。入力・出力の表現と必要な長さは以下の通りです。

`native_div_series(a,n,b,m,out,degree)` は Montgomery 表現の `Mint` 配列から `a/b mod x^degree` を求め、空の `out` へ `degree` 係数を追加します。`degree > 0`、`m > 0`、`b[0] != 0` とします。入力が次数より短いときは残りを 0 として扱います。

`native_divide(a,n,b,m,q,r)` は多項式の商と剰余を空の `q`・`r` へ追加します。`n >= m > 0`、`b[m-1] != 0` とします。商は `n-m+1` 係数、剰余は `m-1` 係数です。内部関数は末尾の 0 を削除せず、公開 `divmod()` 側が削除します。どちらも入力は変更せず、入力と出力は重ねません。呼び出し側の入力に追加の padding や整列は要求しません。

内部の `divide_words(n,m)` と `divide(arena,f,g,q,r)` は通常の剰余値 $[0,p)$ を扱います。$p=998244353$ です。`f`・`g`・`q`・`r` は重ねず、`q` の長さを `quotient_size(n,m)`、`r` の長さを `remainder_size(n,m)` にします。`g` は非空で最高次係数が非零です。`n<m` の場合は商を空にして、剰余へ `f` をコピーします。作業用の整列と領域の間隔は `Arena` が管理します。

`division_detail::DivisionPlan` は商のブロック長、変換を保存する窓の数、剰余の復元方法を選びます。商は逆数を共有したブロック除算で求めます。剰余は短い商の直接計算、巡回畳み込み、分割した中央積、商の末尾を使う中央積から選びます。保存した窓の積和は葉の積と逆変換を融合して計算します。`Bump` は一つの作業領域を分割し、容量不足では `std::bad_alloc` を送出します。

`division_detail::append_words(out,values,n,factor)` は内部の通常の剰余値を $[0,p)$ の `factor` 倍し、`Mint::word()` に相当する表現として追加する内部補助です。既に存在する `Mint` オブジェクトへ表現をコピーしてから `vector::insert()` し、vector の要素外へ書き込みません。商は入力同士の Montgomery 因子が打ち消されるため、結果に $R=2^{32}\bmod p$ を掛けます。剰余は入力と同じ因子を保持します。

公開 FPS から呼ぶ経路では係数数・要求次数を $2^{23}$ 以下に制限します。変換長と積の仕様は [専用変換](fps_blocked.md) に従います。

**計算量**: 入力係数数と要求次数の最大を $n$ として、級数除算・多項式除算は $O(n\log n)$ 時間、追加空間は $O(n)$ です。短い入力の直接除算は商と除数の係数数の積に比例します。
