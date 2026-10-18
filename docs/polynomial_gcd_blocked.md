---
title: 多項式 Half-GCD の専用変換
documentation_of: //components/polynomial_gcd_blocked.hpp
---

`polynomial_gcd.hpp` が使う Linux x86_64 / GCC 用の内部部品です。通常は公開関数 `polynomial_gcd()`、`polynomial_extended_gcd()`、`polynomial_inv_mod()` を使います。専用経路を呼ぶ前に `fps_detail::NTT::vectorized()` を確認します。他の環境では既存の Half-GCD へ戻ります。

`fps_detail::blocked_poly::gcd_detail::HalfGcd` は次数ごとの jump を求めます。`HalfGcd::words(n)` ワードの `Arena` を用意し、`HalfGcd(arena,n)` で変換表と LIFO の作業領域を確保します。作業領域が不足したときは `std::bad_alloc` を送出します。

`matrix(k,room)` は次数の低下量が `k` 以下の変換行列用領域を返します。`room` は保存する変換の最大長で、保存しないときは 0 にします。返した行列の係数配列は `HalfGcd` の作業領域を参照するため、その寿命内だけ使います。

`jump(a,n,b,m,k,out,need)` は `deg(a)=n > deg(b)=m`、`0 <= k <= n` の入力で、互除法を `deg(b) < n-k` になるまでまとめます。零多項式の `b` は `m=-1` で表します。入力係数は $[0,998244353)$、最高次係数は非零とします。`out` は `matrix(k,room)` で確保します。`need` の bit `2*i+j` は行列の $(i,j)$ 成分を求めるかを指定し、既定値は 15 です。指定しない成分の係数と長さは使用しません。

`Matrix::progress` は最初の多項式の次数低下量です。`entry[i]`・`size[i]` は係数配列とその長さ、`at0[i]` は定数項です。小さい jump は除算を繰り返さない互除法で求めます。このため行列全体の非零なスカラー倍は通常の互除法の行列と異なる場合があります。公開関数が最終的な GCD と Bézout 係数を正規化します。行列式が常に $\pm1$ であるとは仮定しません。

変換済みの行列成分は保持し、長さを倍にするときは既存の下半分を再利用します。行列積は同じ行の窓を共有し、必要な成分だけ計算します。行列の作用には巡回畳み込みを使い、巻き戻った係数は短い低次積で補正します。短い互除法の 3 項更新は、64 bit の積和をまとめて Montgomery 還元します。SIMD 更新で触れる前後の領域は所有する作業配列内に確保・初期化します。

`native_inv_mod(a,n,b,m,out)` は `Mint` 配列を受け取り、逆元が存在すれば空の `out` に追加して真を返します。`b` は非空で最高次係数が非零です。定数の法では空の結果を返します。`native_gcd_matrix(a,n,b,m,need,sink)` は `n>m>0` の入力の jump を求め、行列が有効な間に `sink(matrix)` を呼びます。呼び出し側の入力は変更せず、追加の整列や padding を要求しません。

入力の Montgomery ワードを一様な $R=2^{32}$ 倍の通常の剰余値として扱います。法多項式の逆元を公開 `Mint` に戻すときは $R^2\bmod p$ を掛け、二重の表現変換を避けます。

**計算量**: 最大係数数を $n$ として、Half-GCD・GCD・拡張 GCD・法多項式に対する逆元は $O(n\log^2 n)$ 時間、追加空間は $O(n)$ です。
