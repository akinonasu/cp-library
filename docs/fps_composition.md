---
title: FPS の合成・射影の内部処理
documentation_of: //components/fps_composition.hpp
---

Linux x86_64 / GCC 向けの内部部品です。呼び出し側で AVX2 の実行時判定を行い、公開 API は `compose` と `power_projection` を使います。他の環境では既存実装へ戻ります。法は $p=998244353$ です。

`fps_detail::composition_poly` の `Arena`・`Transform`・`Half` はブロック変換の部品を共有します。内部配列は Arena から確保し、利用者の入力に padding や整列を要求しません。確保失敗時には通常の整列配列へ戻り、それも失敗した場合は例外となります。

| 内部 API | 内容 | 計算量 |
| --- | --- | --- |
| `compose_log(n)`・`compose_scratch(n)` | 合成に必要な変換次数と作業領域のワード数 | $$O(1)$$ |
| `compose(t,f,g,h,scratch)` | `h.size()` 項の $f(g)$。普通剰余の入力・出力、$g_0=0$。入力・出力は重ねない | $$O(n\log^2 n)$$ |
| `projection_log(n)`・`projection_scratch(n)` | 係数射影に必要な変換次数と作業領域のワード数 | $$O(1)$$ |
| `power_projection(t,g,a,scratch)` | $a_k=[x^{n-1}]g^k$、$n=\texttt{a.size()}$、$g_0=0$ | $$O(n\log^2 n)$$ |
| `weighted_projection(t,g,w,a,scratch)` | $a_k=\sum_j w_j[x^j]g^k$、`w.size()==a.size()>32`、$g_0=0$ | $$O(n\log^2 n)$$ |
| `native_compose(f,nf,g,ng,out,n)` | 内部 Montgomery 係数の入力から、長さ `n` の合成を空の `vector<Mint>` へ追加 | $$O(n\log^2 n)$$ |
| `native_projection(g,ng,out,n)` | 内部 Montgomery 係数から、長さ `n` の係数射影を空の `vector<Mint>` へ追加 | $$O(n\log^2 n)$$ |
| `native_weighted_projection(g,ng,w,out,n,count)` | `n` 個の重みによる射影の最初の `count` 項を空の `vector<Mint>` へ追加。$0<\texttt{count}\le n$ | $$O(n\log^2 n)$$ |

普通剰余の係数は $[0,p)$、Montgomery 係数の内部ワードは $[0,2p)$ です。入力長を超える係数は 0 とし、出力次数以降は読みません。raw API の作業領域は Arena と同じ整列・余白を持たせます。重み付き raw API は `projection_scratch(n) + Arena::footprint(6*detail::projection_length(n))` ワード、変換次数は `projection_log(n)+1` を使います。

Kinoshita–Li 法の最初の 2 段と最後の 3 段を一変数の計算へ分解し、中間段では零となる列の変換を省きます。重み付き射影は一般の分子を最初の 2 段へ渡します。合成は外側係数に線形なので、外側入力と結果の Montgomery 変換を省略します。

項数を $n$ として時間は $O(n\log^2 n)$、追加空間は合成が $O(n\log n)$、射影が $O(n)$ です。
