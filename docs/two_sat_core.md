---
title: "2-SAT の共通部品"
documentation_of: //components/two_sat_core.hpp
---

[TwoSAT](https://qandori.github.io/cp-library/graph/two_sat.hpp.html) の含意グラフを処理する内部部品です。通常は節を追加できる `TwoSAT` を使用し、符号付きリテラルの含意辺を直接用意する場合にこの部品を使います。

```cpp
#include "lib/components/two_sat_core.hpp"
#include <cassert>
#include <utility>
#include <vector>

std::vector<std::pair<int, int>> edges{ {0, 2}, {3, 1}, {2, 3} };
std::vector<int> forced{3};
std::vector<bool> answer(2);
assert(two_sat_detail::solve(2, edges, forced, answer));
assert(answer[0] && answer[1]);
```

`two_sat_detail::solve(variables, edges, forced, answer)` は充足可能性を `bool` で返します。引数に既定値はありません。

| 引数 | 型・条件 |
| --- | --- |
| `variables` | 非負の `int`。変数の個数で、`INT_MAX / 2` 以下 |
| `edges` | `std::span<const std::pair<int,int>>`。含意の始点・終点を並べる。辺数は `INT_MAX` 以下 |
| `forced` | `std::span<const int>`。既存の単位節で真になるリテラルを並べる |
| `answer` | 長さが `variables` の `std::vector<bool>&`。充足代入の出力先 |

変数 `i` の `false` 側のリテラルは `2*i`、`true` 側は `2*i+1` とし、反対のリテラルは `literal ^ 1` です。辺・固定リテラルの番号は全て `[0,2*variables)` としてください。

辺 `u -> v` は、`u` が真なら `v` が真であることを表します。対偶の辺 `(v ^ 1) -> (u ^ 1)` も用意する必要があります。単位節の辺 `(literal ^ 1) -> literal` は自身が対偶なので 1 本で構いません。重複する辺と自己辺も扱えます。対偶が揃っていることの検査は行いません。

`forced` は先に伝播できる単位節のリテラルを渡す補助入力です。各要素について、対応する辺 `(literal ^ 1) -> literal` を `edges` にも含めてください。別の制約を追加する引数ではありません。重複は許容し、空配列も指定できます。

返り値が `true` の場合は、`answer` を充足代入で更新します。代入例は一意ではありません。返り値が `false` の場合の `answer` の内容は未規定です。入力の `edges`・`forced` は更新しません。呼び出しごとに判定し、結果を保存しません。変数が 0 個の場合は、辺と固定リテラルも空とし、`true` と空の代入例を返します。

固定リテラル同士の矛盾を先に調べます。全変数が固定されている場合は、含意辺を直接検査して判定し、隣接表を構築しません。

初めは配列による連結隣接表を構築し、予算付きの伝播を行います。分岐の多いグラフで探索量が増えた場合は、辺の探索順を保ちながら同じ配列を CSR に変換して伝播を続けます。伝播で矛盾が生じた試行は取り消します。

処理予算を使い切った場合は、CSR を用いた 2-SAT 専用の反復 Tarjan 法へ移ります。出辺のない頂点は単独の強連結成分として扱い、探索スタックへ積みません。同じ成分に反対のリテラルが入れば充足不可能です。

変数数を $$n$$、含意辺数と `forced` の長さの和を $$m$$ として、時間・追加空間は $$O(n+m)$$ です。空の問題は $$O(1)$$ 時間・空間です。
