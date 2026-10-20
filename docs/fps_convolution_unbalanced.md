---
title: FPS の非対称畳み込み
documentation_of: //components/fps_convolution_unbalanced.hpp
---

Linux x86_64 / GCC の AVX2 用内部部品です。公開の `fps::multiply` と `convolution` から利用します。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `fps_detail::multiply_detail::unbalanced::fits(n,m)` | 短辺が 32 より大きく、長辺が 32768 以上かつ短辺の 32 倍以上か判定 | $$O(1)$$ |
| `fps_detail::multiply_detail::unbalanced::multiply(a,b)` | 読み取り専用の二つの係数列の積を返す | $$O((n+m)\log m)$$ |

ここで $$n$$ は長辺、$$m$$ は短辺です。`multiply` の呼び出し側は `fits(a.size(),b.size())`、AVX2 対応、両 span が生存中の `Mint` 要素を指すこと、非空入力、入力の内部 Montgomery 値が $$[0,2p)$$、出力長が $$2^{23}$$ 以下であることを保証します。公開 dispatch は `fits` を満たす場合だけこの経路を使います。

短辺の通常剰余の変換を一度作り、長辺を幅 $$L-m+1$$ のブロックに分けます。$$L=8\operatorname{bitceil}(m)$$ とし、各ブロックの循環積を線形積として復元し、隣接ブロックの重なり $$m-1$$ 項を加算します。長辺は Montgomery 表現のまま計算し、返す係数も正規化した Montgomery 表現です。

入力を変更せず、公開入力に alignment や padding を要求しません。作業配列は `Arena` が所有し、出力は独立した `vector<Mint>` の生存中の要素として構築します。同じ入力を二回渡しても安全です。追加の作業空間は $$O(m)$$、返す配列は $$O(n+m)$$ です。

測定と採用条件の根拠は [非対称畳み込みのブロック化](https://github.com/qandori/cp-library/blob/main/test/benchmark/fps_convolution_unbalanced_round4.md) にあります。
