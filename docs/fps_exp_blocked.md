---
title: FPS の指数・冪の内部変換
documentation_of: //components/fps_exp_blocked.hpp
---

`fps::exp()`・`fps::pow()` と形式的合成逆関数が使う内部部品です。Linux x86_64 / GCC で定義されます。呼び出し側は AVX2 対応を実行時に確認します。他の環境では公開 FPS の既存実装を使います。以下では $$p=998244353$$、係数数を $$n\ge1$$ とします。

`fps_detail::blocked_poly` に次の関数を定義します。

| 関数 | 内容 | 計算量 |
| --- | --- | --- |
| `exp_log(n)` / `exp_scratch(n)` | 指数が必要とする変換プランの最大 log 長／作業領域の word 数 | $$O(1)$$ |
| `exp(t,f,g,scratch)` | `g.size()` 項の $$\exp(f)$$。`f` の定数項は 0 | $$O(n\log n)$$ |
| `power_log(n)` / `power_scratch(n)` | 冪が必要とする最大 log 長／作業領域の word 数 | $$O(1)$$ |
| `power(t,f,e,c,g,scratch)` | `g.size()` 項の $$c\exp(e\log(f/f_0))$$。$$f_0\ne0$$、`e,c` は $$[0,p)$$ | $$O(n\log n)$$ |
| `native_exp(in,input,out,n,differentiated=false)` | `input` 個の `Mint` 係数から指数を計算し、空の `vector<Mint>` である `out` へ $$n$$ 項を追加 | $$O(n\log n)$$ |
| `native_power(in,input,out,n,e,c)` | `Mint` 入力から $$c\exp(e\log(f/f_0))$$ の $$n$$ 項を `out` へ追加。`c` は `Mint` | $$O(n\log n)$$ |

raw の `f,g` はそれぞれ `span<const uint32_t>`・`span<uint32_t>` です。係数は $$[0,p)$$ の通常の剰余値です。`f` の範囲外は 0 として扱います。`f` と `g` は同じ先頭を持つインプレース処理が可能で、それ以外では重ねません。`g`・`scratch` は `Arena` から確保して 32 byte に整列し、作業領域を入力・出力と重ねません。`scratch` の長さは対応する `*_scratch(n)` 以上、`t.lg_max()` は `*_log(n)` 以上とします。`Arena::take()` の呼び出しごとに必要な `Arena::footprint()` の余白も、Arena 全体の確保に含めます。

native 関数は入力を変更せず、入力・出力を重ねません。入力ワードは `Mint` の内部 Montgomery 表現 $$[0,2p)$$ です。入力と出力に余分な padding やアラインメントは要求しません。`native_exp` の `differentiated=true` は、入力を通常の指数の引数ではなく $$q=xf'$$ と解釈します。定数項はどちらも 0 とします。`native_power` の入力の定数項は非零です。公開 FPS は $$256\le n\le2^{24}$$ の条件でこれらへ分岐します。

`native_exp` の通常入力は、内部語 `aR` と通常表現の次数 `i` の Montgomery 積で `i*a` に変換します。公開の次数上限では積の縮約結果は `2p` 未満となり、`p` による縮約を 1 回行います。微分済み入力では内部語の表現変換だけを行います。入力は完全な 8 語のブロックを `memcpy` で読み、端数は個別に扱います。この前処理は $$O(n)$$ 時間、$$O(1)$$ 追加空間です。

`native_exp` は $$n=2^k+\delta$$、$$2^k\ge4096$$、$$1\le\delta\le16$$ では、既存の計算本体で $$2^k$$ 項を求めた後、$$\displaystyle i g_i=\sum_{j=1}^{i}q_j g_{i-j}$$ から残りの項を補います。通常入力では $$q_j=j f_j$$、微分済み入力では入力自体が $$q$$ です。追加部分は AVX2 の内積を使い、$$O(n\delta)$$ 時間、$$O(1)$$ 追加空間です。入力と構築済み出力を完全な 8 項単位で `memcpy` し、端数は個別に扱います。

指数は既知の $$g\bmod x^m$$ と逆数 $$1/g\bmod x^{m/2}$$ を更新し、最後の更新を $$m/2$$ 項のブロックに分けます。冪は対数微分から $$q=e\,xf'/f$$ を作り、その部分変換を指数の最後の更新へ渡します。native の結果は Montgomery 倍のまま生成し、結果全体の再変換を省きます。

時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。`*_log()`・`*_scratch()` は $$O(1)$$ 時間・空間です。入力の最大長を $$a$$ とすると native 関数が読む長さは $$\min(a,n)$$ です。
