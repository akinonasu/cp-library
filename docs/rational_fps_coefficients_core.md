---
title: 有理 FPS の区間係数取得の共通処理
documentation_of: //components/rational_fps_coefficients.hpp
---

通常版と軽量版で共有する MSB-first 版 Bostan–Mori です。通常は [有理 FPS の区間係数取得](rational_fps_coefficients.md) を使います。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `rational_fps_detail::Coefficients<F,Transform>::get(p,q,l,r)` | $$p/q$$ の半開区間 `[l,r)` の係数を取得 | $$O(N\log N\log r+(N+M)\log(N+M))$$ |
| `Coefficients::middle(f,g)` | 積の次数 `f.size()-1` から `g.size()-1` までを取得。`0<f.size()<=g.size()` | $$O(S\log S)$$ |

`F` は FPS 型、`Transform` は順変換・正規化済み逆変換・成分ごとの積を提供する型です。変換結果の並びは bit reverse 順とします。

$$N=\max(\lvert p\rvert,\lvert q\rvert)$$、$$M=r-l$$、$$S=\lvert g\rvert$$ として、区間取得の追加空間は $$O(N\log r+M)$$、中央部分の積の追加空間は $$O(S)$$ です。
