---
title: FPS 平方根のブロック変換
documentation_of: //components/fps_sqrt_blocked.hpp
---

`fps::sqrt()` が使う内部部品です。通常は公開 FPS API から利用します。Linux x86_64 / GCC 用に定義され、呼び出し側で AVX2 対応を実行時に確認します。他の環境では既存の FPS 実装を使います。

`fps_detail::blocked_poly::native_sqrt(in,input,out,n,constant)` は、`in` の `input` 個の `Mint` 係数から、定数項が `constant` の平方根 $g^2=f\bmod x^n$ を求めます。結果の `n` 項を、空の `vector<Mint>` である `out` に追加します。`1 <= n <= 2^23`、`input >= 1`、定数項が非零、`constant * constant == in[0]` を前提とします。内部ワードの範囲は $[0,2p)$、$p=998244353$ です。入力の範囲外を 0 とし、次数 `n` 以降は読みません。入力は変更せず、`out` と重ねません。公開配列に padding やアラインメントは要求しません。

公開 FPS 側で先頭の零係数と定数項の平方根を確認してから、この関数を呼びます。先頭の零の個数が奇数、または定数項が平方剰余でない場合は、公開 API が空配列を返します。入力が零多項式の場合も公開 API 側で処理します。

内部では $g$ と $-1/g$ の接頭辞を Newton 更新し、次数 8 の環の専用二乗と、逆変換の最上段と次の順変換の融合を使います。最後の更新は Karp–Markstein の分割により、必要な項数に応じて半分の長さの変換を使います。

入力の Montgomery ワードを通常の整数として扱って $\sqrt{Rf}$ を計算し、出力へ $2^{16}$ を掛けます。$R=2^{32}\bmod p$ なので、出力は元の平方根の Montgomery 表現となります。

通常の剰余値 $[0,p)$ から直接使う内部 API は次のとおりです。

- `sqrt_log(n)` は必要な `Transform` の最大対数を返します。
- `sqrt_scratch(n)` は `sqrt` が必要とする作業領域のワード数を返します。
- `sqrt(t,f,c,g,scratch)` は `g.size()` 項の平方根を求めます。`f`・`g`・作業領域を重ねず、`c*c == f[0] != 0` とします。`g` と作業領域は `Arena` から確保し、`t.lg_max() >= sqrt_log(g.size())` を満たします。
- `sqrt_steps(t,f,c,a,b,ht)` は 64 以上の 2 冪 `m=a.size()` まで平方根を求め、`ht` に $-1/g\bmod x^{m/2}$ の長さ `m` の変換を残します。`a`・`b`・`ht` は重ならない `Arena` の長さ `m` の領域です。
- `sqrt_last_step(t,f,gt,ht,work,hi)` は、長さ `m` の `gt`・`ht` から続く `hi.size()` 項を求めます。`gt` は平方根の長さ `m` の変換、`ht` は `sqrt_steps` の出力とし、`1 <= hi.size() <= m` とします。`work` は長さ `m` の `Arena` 領域です。`hi` は `f` の次数 `m` 以降と同じ領域でも構いません。他の領域は重ねません。

作業領域は RAII の `Arena` が所有し、mmap が使えない場合は通常の整列配列へ戻ります。確保できなければ例外を送出します。

**計算量**: 平方根全体は $O(n\log n)$ 時間、$O(n)$ 追加空間です。`sqrt_log`・`sqrt_scratch` は $O(1)$ 時間です。
