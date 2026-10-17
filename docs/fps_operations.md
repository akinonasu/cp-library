---
title: "FPS の内部演算"
documentation_of: //components/fps_operations.hpp
---

`fps_detail::Operations<T>` は FPS と独立した多項式関数で共有する内部処理です。通常は `fps/fps.hpp` や各関数のヘッダーから利用します。`F` は `FormalPowerSeries<T>`、`W` は内部係数型の FPS、`base` は `vector<T>`、`Word` は内部の 32 bit 整数です。

| 操作 | 内容・返り値 |
| --- | --- |
| `import_work(a)` / `export_work(a)` | 係数型を内部型へ／内部型から変換 |
| `transform(n)` | 根を少なくとも `n` 個保持する共有 NTT プラン |
| `words(a)` | 係数配列の内部ワードへのポインタ |
| `transform_inplace<inverse>(a)` | 内部順序の NTT。逆変換は未正規化 |
| `ntt(a)` / `ntt_inverse(a)` / `intt(a)` | 順変換／未正規化の逆変換／正規化した逆変換 |
| `ntt_half(a)` / `ntt_half(a,n)` | 後半が 0 の係数列を変換 |
| `scale_words(a,c)` | 全係数を定数倍 |
| `geometric_scale(a,first,step)` | 第 `i` 係数を `first * step^i` 倍 |
| `batch_inverse(dst,src)` | 各非零要素の逆元を一括計算 |
| `pointwise(r,a,b,n)` | 先頭 `n` 個の要素ごとの積 |
| `inverse_extend(h,a,cache)` | キャッシュした変換を使って逆数の精度を倍化 |
| `dot(a,b,n)` | 逆向きの添字を使う内積 |
| `mod_sqrt(a)` | 法 998244353 の平方根。非平方剰余は 0 |
| `inverse_ntt_roots(n)` / `ntt_roots(n)` | 内部順序の逆根／根のキャッシュ |
| `inverses(n)` | 整数 0～`n` の逆元のキャッシュ。0 の項は 0 |
| `trim(a)` | 末尾の零係数を除去 |

係数数を $$n$$ として、変換・`inverse_extend` は $$O(n\log n)$$、その他の配列操作は $$O(n)$$ です。根・逆元のキャッシュ拡張は追加要素数に比例し、取得のみなら $$O(1)$$、`words` は $$O(1)$$ です。`mod_sqrt` は固定法に対して $$O(1)$$、追加空間・キャッシュは $$O(n)$$ です。
