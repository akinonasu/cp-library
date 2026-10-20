---
title: 多点評価・補間の SIMD 木
documentation_of: //components/fps_evaluation.hpp
---

多点評価・補間が共有する内部部品です。通常は `fps/multipoint_eval.hpp` または `fps/interpolate.hpp` を使います。Linux x86_64 / GCC で定義され、呼び出し側が AVX2 対応を確認します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_evaluate(f,points)` | 各点での評価値を返す | $$O((n+m)\log^2(n+m))$$ |
| `native_interpolate(points,values)` | 点と値から係数列を返す | $$O(m\log^2 m)$$ |
| `evaluate(arena,f,points,values)` | 通常表現での多点評価 | $$O((n+m)\log^2(n+m))$$ |
| `interpolate(arena,points,values,c)` | 通常表現での補間 | $$O(m\log^2 m)$$ |
| `evaluate_words(n,m)` | 多点評価の作業領域のワード数 | $$O(1)$$ |
| `interpolate_words(m)` | 補間の作業領域のワード数 | $$O(1)$$ |

関数は `fps_detail::evaluation_poly` 内にあります。`n` は多項式の係数数、`m` は評価点数です。

`fps_detail::evaluation_poly::native_evaluate(f,points)` は内部 `Mint` の入力から評価値の `vector<Mint>` を返します。入力は変更せず、評価点の重複を許します。`native_interpolate(points,values)` は相異なる点での値から係数列を返します。こちらの入力長は等しく、非零です。呼び出し側は入力長・点数がそれぞれ $$2^{23}$$ 以下であることを確認します。

8 本の多項式を SIMD の各 lane で構築し、その積を上位の木で結合します。`PointTree` は各節点の変換を保持し、降下時の中間積を変換されたまま半分へ縮めます。`InterpolationTree` はこの降下と補間の上昇を共有します。`TreeTransform` は次数 8 の環を葉とする標準配置と、8 本の係数を並べる lane 配置の両方を扱います。

評価は入力多項式、補間は値の列について線形です。native の入口ではその係数列の Montgomery 共通因子 $$R=2^{32}$$ を保持し、出力をそのまま内部表現として格納します。評価点だけ通常表現へ変換します。

生ワードを扱う `evaluate(arena,f,points,values)` と `interpolate(arena,points,values,c)` の入出力は通常表現 $$[0,p)$$、木の係数は内部で Montgomery 表現を使います。ここで $$p=998244353$$ です。`evaluate_words(n,m)`・`interpolate_words(m)` は作業領域の必要量を返します。`Arena` は通常の整列確保へフォールバックした場合も、元の実装が要求する初期値 0 を保証します。公開配列に padding や整列は要求しません。

係数数を $$n$$、点数を $$m$$ として、時間は $$O((n+m)\log^2(n+m))$$、追加空間は $$O((n+m)\log(n+m))$$ です。
